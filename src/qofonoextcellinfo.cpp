/****************************************************************************
**
** Copyright (C) 2016-2017 Jolla Ltd.
** Contact: Slava Monich <slava.monich@jolla.com>
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
    QDBusPendingCall GetInterfaceVersion()
        { return asyncCall("GetInterfaceVersion"); }
    QDBusPendingCall GetCells()
        { return asyncCall("GetCells"); }

Q_SIGNALS: // SIGNALS
    void CellsAdded(QList<QDBusObjectPath> aPaths);
    void CellsRemoved(QList<QDBusObjectPath> aPaths);
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

private:
    void getCells();
    void invalidate();

private Q_SLOTS:
    void checkInterfacePresence();
    void onGetCellsFinished(QDBusPendingCallWatcher* aWatcher);
    void onCellsAdded(QList<QDBusObjectPath> aCells);
    void onCellsRemoved(QList<QDBusObjectPath> aCells);

public:
    bool iValid;
    bool iFixedPath;
    QStringList iCells;

private:
    QOfonoExtCellInfo* iParent;
    QOfonoExtCellInfoProxy* iProxy;
    QSharedPointer<QOfonoModem> iModem;
};

QOfonoExtCellInfo::Private::Private(QOfonoExtCellInfo* aParent) :
    QObject(aParent),
    iValid(false),
    iFixedPath(false),
    iParent(aParent),
    iProxy(NULL)
{
}

QString QOfonoExtCellInfo::Private::modemPath() const
{
    return iModem.isNull() ? QString() : iModem->objectPath();
}

void QOfonoExtCellInfo::Private::setModemPath(QString aPath)
{
    // Caller has checked the the path has actually changed
    invalidate();
    if (aPath.isEmpty()) {
        iModem.clear();
    } else {
        if (iModem) iModem->disconnect(this);
        iModem = QOfonoModem::instance(aPath);
        connect(iModem.data(),
            SIGNAL(validChanged(bool)),
            SLOT(checkInterfacePresence()));
        connect(iModem.data(),
            SIGNAL(interfacesChanged(QStringList)),
            SLOT(checkInterfacePresence()));
        checkInterfacePresence();
    }
}

void QOfonoExtCellInfo::Private::getCells()
{
    connect(new QDBusPendingCallWatcher(iProxy->GetCells(), iProxy),
        SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onGetCellsFinished(QDBusPendingCallWatcher*)));
}

void QOfonoExtCellInfo::Private::checkInterfacePresence()
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
                getCells();
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

void QOfonoExtCellInfo::Private::onGetCellsFinished(QDBusPendingCallWatcher* aWatcher)
{
    QDBusPendingReply<QList<QDBusObjectPath> > reply(*aWatcher);
    if (reply.isError()) {
        // Repeat the call on timeout
        qWarning() << reply.error();
        if (QOfonoExt::isTimeout(reply.error())) {
            getCells();
        }
    } else {
        QList<QDBusObjectPath> pathList = reply.value();
        QStringList list;
        for (int i=0; i<pathList.count(); i++) {
            list.append(pathList.at(i).path());
        }
        list.sort();
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

// ==========================================================================
// QOfonoExtCellInfo
// ==========================================================================

QOfonoExtCellInfo::QOfonoExtCellInfo(QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

QOfonoExtCellInfo::~QOfonoExtCellInfo()
{
}

QSharedPointer<QOfonoExtCellInfo> QOfonoExtCellInfo::instance(QString aPath)
{
    QSharedPointer<QOfonoExtCellInfo> ptr = sharedInstances()->value(aPath);
    if (ptr.isNull()) {
        ptr = QSharedPointer<QOfonoExtCellInfo>(new QOfonoExtCellInfo, &QObject::deleteLater);
        ptr->setModemPath(aPath);
        ptr->iPrivate->iFixedPath = true;
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

#include "qofonoextcellinfo.moc"
