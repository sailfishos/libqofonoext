/****************************************************************************
**
** Copyright (C) 2016-2020 Jolla Ltd.
** Copyright (C) 2021 Open Mobile Platform LLC.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
****************************************************************************/

#include "qofonoextcellinfo.h"
#include "qofonoext_p.h"

#include <qofonomodem.h>

static const QString kMethodGetCells("GetCells");

typedef QMap<QString,QWeakPointer<QOfonoExtCellInfo> > QOfonoExtCellInfoMap;
Q_GLOBAL_STATIC(QOfonoExtCellInfoMap, sharedInstances)

// ==========================================================================
// QOfonoExtCellInfoProxy
//
// qdbusxml2cpp doesn't really do much, and has a number of limitations,
// such as the limits on number of arguments for QDBusPendingReply template.
// It's easier to write these proxies by hand.
// ==========================================================================

class QOfonoExtCellInfoProxy: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static const QString INTERFACE;
    QOfonoExtCellInfoProxy(QString aPath, QObject* aParent) :
        QDBusAbstractInterface(OFONO_SERVICE, aPath, qPrintable(INTERFACE),
            OFONO_BUS, aParent) {}

public Q_SLOTS: // METHODS
    QDBusPendingCall GetCellsAsync()
        { return asyncCall(kMethodGetCells); }
    QDBusMessage GetCellsSync()
        { return call(kMethodGetCells); }
    QDBusPendingCall Unsubscribe()
        { return asyncCall("Unsubscribe"); }

Q_SIGNALS: // SIGNALS
    void CellsAdded(QList<QDBusObjectPath> aPaths);
    void CellsRemoved(QList<QDBusObjectPath> aPaths);
    void Unsubscribed();
};

const QString QOfonoExtCellInfoProxy::INTERFACE("org.nemomobile.ofono.CellInfo");

// ==========================================================================
// QOfonoExtCellInfo::Private
// ==========================================================================

class QOfonoExtCellInfo::Private : public QObject
{
    Q_OBJECT

public:
    Private(QOfonoExtCellInfo* aParent);
    QString modemPath() const;
    void setModemPath(QString aPath);
    void setModemPathSyncInit(QString aPath);
    void setActive(bool val);

private:
    void getCellsSyncInit();
    void getCellsAsync();
    void invalidate();
    void setModemPath(QString aPath, QSharedPointer<QOfonoModem> aModem, void (Private::*aGetCells)());
    void checkInterfacePresence(void (Private::*getCellsFn)());
    static QStringList getPaths(const QList<QDBusObjectPath> aPaths);
    void unsubscribe();

private Q_SLOTS:
    void onModemChanged();
    void onGetCellsFinished(QDBusPendingCallWatcher* aWatcher);
    void onUnsubscribeFinished(QDBusPendingCallWatcher* aWatcher);
    void onCellsAdded(QList<QDBusObjectPath> aCells);
    void onCellsRemoved(QList<QDBusObjectPath> aCells);
    void onUnsubscribed();

public:
    bool iValid;
    bool iFixedPath;
    QStringList iCells;
    bool iActive;

private:
    QOfonoExtCellInfo* iParent;
    QOfonoExtCellInfoProxy* iProxy;
    QSharedPointer<QOfonoModem> iModem;
};

QOfonoExtCellInfo::Private::Private(QOfonoExtCellInfo* aParent) :
    QObject(aParent),
    iValid(false),
    iFixedPath(false),
    iActive(false),
    iParent(aParent),
    iProxy(NULL)
{
}

inline QString QOfonoExtCellInfo::Private::modemPath() const
{
    return iModem.isNull() ? QString() : iModem->objectPath();
}

inline void QOfonoExtCellInfo::Private::setModemPath(QString aPath)
{
    setModemPath(aPath, QOfonoModem::instance(aPath), &Private::getCellsAsync);
}

inline void QOfonoExtCellInfo::Private::setModemPathSyncInit(QString aPath)
{
    setModemPath(aPath, QOfonoModem::instance(aPath, true), &Private::getCellsSyncInit);
}

void QOfonoExtCellInfo::Private::setModemPath(QString aPath,
    QSharedPointer<QOfonoModem> aModem, void (Private::*aGetCells)())
{
    // Caller has checked the the path has actually changed
    invalidate();
    if (aPath.isEmpty()) {
        if (iModem) {
            iModem->disconnect(this);
            iModem.clear();
        }
    } else {
        if (iModem) iModem->disconnect(this);
        iModem = aModem;
        connect(iModem.data(),
            SIGNAL(validChanged(bool)),
            SLOT(onModemChanged()));
        connect(iModem.data(),
            SIGNAL(interfacesChanged(QStringList)),
            SLOT(onModemChanged()));
        checkInterfacePresence(aGetCells);
    }
}

void QOfonoExtCellInfo::Private::getCellsAsync()
{
    connect(new QDBusPendingCallWatcher(iProxy->GetCellsAsync(), iProxy),
        SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onGetCellsFinished(QDBusPendingCallWatcher*)));
    if (!iActive) {
        iActive = true;
        Q_EMIT iParent->activeChanged();
    }
}

void QOfonoExtCellInfo::Private::getCellsSyncInit()
{
    QDBusPendingReply<QList<QDBusObjectPath> > reply(iProxy->GetCellsSync());
    if (!reply.isError()) {
        iCells = getPaths(reply.value());
        iValid = true;
    } else {
        // Repeat call asynchronously on timeout
        QDBusError error(reply.error());
        qWarning() << error;
        if (QOfonoExt::isTimeout(error)) {
            getCellsAsync();
        }
    }
    if (!iActive) {
        iActive = true;
        Q_EMIT iParent->activeChanged();
    }
}

void QOfonoExtCellInfo::Private::unsubscribe()
{
    if (iProxy) {
        connect(new QDBusPendingCallWatcher(iProxy->Unsubscribe(), iProxy),
            SIGNAL(finished(QDBusPendingCallWatcher*)),
            SLOT(onUnsubscribeFinished(QDBusPendingCallWatcher*)));
    }
}

void QOfonoExtCellInfo::Private::checkInterfacePresence(void (Private::*aGetCells)())
{
    if (iModem && iModem->isValid() &&
        iModem->interfaces().contains(QOfonoExtCellInfoProxy::INTERFACE)) {
        if (!iProxy) {
            iProxy = new QOfonoExtCellInfoProxy(iModem->objectPath(), this);
            if (iProxy->isValid()) {
                connect(iProxy,
                    SIGNAL(CellsAdded(QList<QDBusObjectPath>)),
                    SLOT(onCellsAdded(QList<QDBusObjectPath>)));
                connect(iProxy,
                    SIGNAL(CellsRemoved(QList<QDBusObjectPath>)),
                    SLOT(onCellsRemoved(QList<QDBusObjectPath>)));
                connect(iProxy, SIGNAL(Unsubscribed()),
                        SLOT(onUnsubscribed()));
                (this->*aGetCells)();
            } else {
                invalidate();
            }
        }
    } else {
        invalidate();
    }
}

void QOfonoExtCellInfo::Private::invalidate()
{
    if (iProxy) {
        delete iProxy;
        iProxy = NULL;
    }
    if (iValid) {
        iValid = false;
        Q_EMIT iParent->validChanged();
    }
}

QStringList QOfonoExtCellInfo::Private::getPaths(const QList<QDBusObjectPath> aPaths)
{
    QStringList list;
    const int n = aPaths.count();
    for (int i = 0; i < n; i++) {
        list.append(aPaths.at(i).path());
    }
    list.sort();
    return list;
}

void QOfonoExtCellInfo::Private::onGetCellsFinished(QDBusPendingCallWatcher* aWatcher)
{
    QDBusPendingReply<QList<QDBusObjectPath> > reply(*aWatcher);
    if (reply.isError()) {
        // Repeat the call on timeout
        QDBusError error(reply.error());
        qWarning() << error;
        if (QOfonoExt::isTimeout(error)) {
            getCellsAsync();
        }
    } else {
        const QStringList list(getPaths(reply.value()));
        if (iCells != list) {
            iCells = list;
            Q_EMIT iParent->cellsChanged();
        }
        if (!iValid) {
            iValid = true;
            Q_EMIT iParent->validChanged();
        }
    }
    aWatcher->deleteLater();
}

void QOfonoExtCellInfo::Private::onModemChanged()
{
    checkInterfacePresence(&Private::getCellsAsync);
}

void QOfonoExtCellInfo::Private::onUnsubscribeFinished(
                                        QDBusPendingCallWatcher* aWatcher)
{
    aWatcher->deleteLater();
}

void QOfonoExtCellInfo::Private::onCellsAdded(QList<QDBusObjectPath> aCells)
{
    QStringList cells;
    for (int i=0; i<aCells.count(); i++) {
        QString path = aCells.at(i).path();
        if (!iCells.contains(path)) {
            iCells.append(path);
            cells.append(path);
        }
    }
    if (!cells.isEmpty()) {
        iCells.sort();
        Q_EMIT iParent->cellsAdded(cells);
        Q_EMIT iParent->cellsChanged();
    }
}

void QOfonoExtCellInfo::Private::onCellsRemoved(QList<QDBusObjectPath> aCells)
{
    QStringList cells;
    for (int i=0; i<aCells.count(); i++) {
        QString path = aCells.at(i).path();
        if (iCells.removeOne(path)) {
            cells.append(path);
        }
    }
    if (!cells.isEmpty()) {
        Q_EMIT iParent->cellsRemoved(cells);
        Q_EMIT iParent->cellsChanged();
    }
}

void QOfonoExtCellInfo::Private::onUnsubscribed()
{
    if (iActive) {
        iActive = false;
        Q_EMIT iParent->activeChanged();
    }
}

void QOfonoExtCellInfo::Private::setActive(bool val)
{
    if (val != iActive) {
        if (val)
            getCellsAsync();
        else
            unsubscribe();
    }
}

// ==========================================================================
// QOfonoExtCellInfo
// ==========================================================================

QOfonoExtCellInfo::QOfonoExtCellInfo(QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

QOfonoExtCellInfo::QOfonoExtCellInfo(QString aModemPath, QObject* aParent) : // Since 1.0.27
    QObject(aParent),
    iPrivate(new Private(this))
{
    iPrivate->setModemPathSyncInit(aModemPath); // Blocks
}

QOfonoExtCellInfo::~QOfonoExtCellInfo()
{
    iPrivate->setActive(false);
}

QSharedPointer<QOfonoExtCellInfo> QOfonoExtCellInfo::instance(QString aPath)
{
    return instance(aPath, false); // Don't block
}

QSharedPointer<QOfonoExtCellInfo> QOfonoExtCellInfo::instance(QString aPath, bool aMayBlock) // Since 1.0.27
{
    QSharedPointer<QOfonoExtCellInfo> ptr = sharedInstances()->value(aPath);
    if (ptr.isNull()) {
        QOfonoExtCellInfo* cellInfo;
        if (aMayBlock) {
            cellInfo = new QOfonoExtCellInfo(aPath); // Blocks
        } else {
            cellInfo = new QOfonoExtCellInfo();
            cellInfo->setModemPath(aPath);
        }
        cellInfo->iPrivate->iFixedPath = true;
        ptr = QSharedPointer<QOfonoExtCellInfo>(cellInfo, &QObject::deleteLater);
        sharedInstances()->insert(aPath, QWeakPointer<QOfonoExtCellInfo>(ptr));
    }
    return ptr;
}

bool QOfonoExtCellInfo::valid() const
{
    return iPrivate->iValid;
}

QString QOfonoExtCellInfo::modemPath() const
{
    return iPrivate->modemPath();
}

QStringList QOfonoExtCellInfo::cells() const
{
    return iPrivate->iCells;
}

void QOfonoExtCellInfo::setModemPath(QString aModemPath)
{
    if (iPrivate->modemPath() != aModemPath) {
        if (iPrivate->iFixedPath) {
            qWarning() << "Attempting to change fixed path" << iPrivate->modemPath();
        } else {
            iPrivate->setModemPath(aModemPath);
        }
    }
}

bool QOfonoExtCellInfo::active() const
{
    return iPrivate->iActive;
}

void QOfonoExtCellInfo::setActive(bool val)
{
    iPrivate->setActive(val);
}

#include "qofonoextcellinfo.moc"
