/****************************************************************************
**
** Copyright (C) 2016 Jolla Ltd.
** Contact: slava.monich@jolla.com
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

#include "qofonoextsiminfo.h"
#include "qofonoext_p.h"

#include "siminfo_interface.h"

#include <qofonomodem.h>

class QOfonoExtSimInfo::Private : public QObject
{
    Q_OBJECT

public:
    QOfonoExtSimInfo* iParent;
    QOfonoExtSimInfoProxy* iProxy;
    QSharedPointer<QOfonoModem> iModem;
    QString iInterfaceName;
    bool iValid;
    QString iModemPath;
    QString iCardIdentifier;
    QString iSubscriberIdentity;
    QString iServiceProviderName;

    Private(QOfonoExtSimInfo* aParent);

    QString modemPath() const;
    void setModemPath(QString aPath);
    void invalidate();

private Q_SLOTS:
    void checkInterfacePresence();
    void onGetAllFinished(QDBusPendingCallWatcher* aWatcher);
    void onCardIdentifierChanged(QString aCardIdentifier);
    void onSubscriberIdentityChanged(QString aSubscriberIdentity);
    void onServiceProviderNameChanged(QString aServiceProviderName);
};

QOfonoExtSimInfo::Private::Private(QOfonoExtSimInfo* aParent) :
    QObject(aParent),
    iParent(aParent),
    iProxy(NULL),
    iInterfaceName(QOfonoExtSimInfoProxy::staticInterfaceName()),
    iValid(false)
{
}

QString QOfonoExtSimInfo::Private::modemPath() const
{
    return iModem.isNull() ? QString() : iModem->objectPath();
}

void QOfonoExtSimInfo::Private::setModemPath(QString aPath)
{
    if (aPath != modemPath()) {
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
}

void QOfonoExtSimInfo::Private::checkInterfacePresence()
{
    if (iModem && iModem->isValid() &&
        iModem->interfaces().contains(iInterfaceName)) {
        if (!iProxy) {
            iProxy = new QOfonoExtSimInfoProxy(OFONO_SERVICE, iModem->objectPath(), OFONO_BUS, this);
            if (iProxy->isValid()) {
                connect(new QDBusPendingCallWatcher(iProxy->GetAll(), iProxy),
                    SIGNAL(finished(QDBusPendingCallWatcher*)),
                    SLOT(onGetAllFinished(QDBusPendingCallWatcher*)));
                connect(iProxy,
                    SIGNAL(CardIdentifierChanged(QString)),
                    SLOT(onCardIdentifierChanged(QString)));
                connect(iProxy,
                    SIGNAL(SubscriberIdentityChanged(QString)),
                    SLOT(onSubscriberIdentityChanged(QString)));
                connect(iProxy,
                    SIGNAL(ServiceProviderNameChanged(QString)),
                    SLOT(onServiceProviderNameChanged(QString)));
            } else {
                invalidate();
            }
        }
    } else {
        invalidate();
    }
}

void QOfonoExtSimInfo::Private::invalidate()
{
    if (iProxy) {
        delete iProxy;
        iProxy = NULL;
    }
    if (iValid) {
        iValid = false;
        Q_EMIT iParent->validChanged(false);
    }
}

void QOfonoExtSimInfo::Private::onGetAllFinished(QDBusPendingCallWatcher* aWatcher)
{
    QDBusPendingReply<int,      // InterfaceVersion
        QString,                // CardIdentifier
        QString,                // SubscriberIdentity
        QString>                // ServiceProviderName
        reply(*aWatcher);
    if (reply.isError()) {
        qWarning() << reply.error();
    } else {
        QString iccid = reply.argumentAt<1>();
        if (iCardIdentifier != iccid) {
            iCardIdentifier = iccid;
            Q_EMIT iParent->cardIdentifierChanged(iccid);
        }
        QString imsi = reply.argumentAt<2>();
        if (iSubscriberIdentity != imsi) {
            iSubscriberIdentity = imsi;
            Q_EMIT iParent->subscriberIdentityChanged(imsi);
        }
        QString spn = reply.argumentAt<3>();
        if (iServiceProviderName != spn) {
            iServiceProviderName = spn;
            Q_EMIT iParent->serviceProviderNameChanged(spn);
        }
        if (!iValid) {
            iValid = true;
            Q_EMIT iParent->validChanged(iValid);
        }
    }
    aWatcher->deleteLater();
}

void QOfonoExtSimInfo::Private::onCardIdentifierChanged(QString aValue)
{
    if (iCardIdentifier != aValue) {
        iCardIdentifier = aValue;
        Q_EMIT iParent->cardIdentifierChanged(aValue);
    }
}

void QOfonoExtSimInfo::Private::onSubscriberIdentityChanged(QString aValue)
{
    if (iSubscriberIdentity != aValue) {
        iSubscriberIdentity = aValue;
        Q_EMIT iParent->subscriberIdentityChanged(aValue);
    }
}

void QOfonoExtSimInfo::Private::onServiceProviderNameChanged(QString aValue)
{
    if (iServiceProviderName != aValue) {
        iServiceProviderName = aValue;
        Q_EMIT iParent->serviceProviderNameChanged(aValue);
    }
}

// ==========================================================================
// QOfonoExtSimInfo
// ==========================================================================

QOfonoExtSimInfo::QOfonoExtSimInfo(QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

QOfonoExtSimInfo::~QOfonoExtSimInfo()
{
}

bool QOfonoExtSimInfo::valid() const
{
    return iPrivate->iValid;
}

QString QOfonoExtSimInfo::modemPath() const
{
    return iPrivate->modemPath();
}

QString QOfonoExtSimInfo::cardIdentifier() const
{
    return iPrivate->iCardIdentifier;
}

QString QOfonoExtSimInfo::subscriberIdentity() const
{
    return iPrivate->iSubscriberIdentity;
}

QString QOfonoExtSimInfo::serviceProviderName() const
{
    return iPrivate->iServiceProviderName;
}

void QOfonoExtSimInfo::setModemPath(QString aPath)
{
    iPrivate->setModemPath(aPath);
}

#include "qofonoextsiminfo.moc"
