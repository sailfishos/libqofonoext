/****************************************************************************
**
** Copyright (C) 2016 Jolla Ltd.
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

#include "cellinfo_interface.h"

#include <qofonomodem.h>

typedef QMap<QString,QWeakPointer<QOfonoExtCellInfo> > QOfonoExtCellInfoMap;
Q_GLOBAL_STATIC(QOfonoExtCellInfoMap, sharedInstances)

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
    void onCellAdded(QDBusObjectPath aPath);
    void onCellRemoved(QDBusObjectPath aPath);

public:
    bool iValid;
    bool iFixedPath;
    QStringList iCells;

private:
    QOfonoExtCellInfo* iParent;
    QOfonoExtCellInfoProxy* iProxy;
    QSharedPointer<QOfonoModem> iModem;
    QString iInterfaceName;
};

QOfonoExtCellInfo::Private::Private(QOfonoExtCellInfo* aParent) :
    QObject(aParent),
    iValid(false),
    iFixedPath(false),
    iParent(aParent),
    iProxy(NULL),
    iInterfaceName(QOfonoExtCellInfoProxy::staticInterfaceName())
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
        iModem->interfaces().contains(iInterfaceName)) {
        if (!iProxy) {
            iProxy = new QOfonoExtCellInfoProxy(OFONO_SERVICE, iModem->objectPath(), OFONO_BUS, this);
            if (iProxy->isValid()) {
                connect(iProxy,
                    SIGNAL(CellAdded(QDBusObjectPath)),
                    SLOT(onCellAdded(QDBusObjectPath)));
                connect(iProxy,
                    SIGNAL(CellRemoved(QDBusObjectPath)),
                    SLOT(onCellRemoved(QDBusObjectPath)));
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

void QOfonoExtCellInfo::Private::onCellAdded(QDBusObjectPath aPath)
{
    QString path = aPath.path();
    if (!iCells.contains(path)) {
        iCells.append(path);
        iCells.sort();
        Q_EMIT iParent->cellsChanged();
    }
}

void QOfonoExtCellInfo::Private::onCellRemoved(QDBusObjectPath aPath)
{
    QString path = aPath.path();
    if (iCells.removeOne(path)) {
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
