/****************************************************************************
**
** Copyright (C) 2015-2021 Jolla Ltd.
** Copyright (C) 2015-2021 Slava Monich <slava.monich@jolla.com>
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

#include <qofonomodem.h>

// ==========================================================================
// QOfonoExtSimInfoProxy
//
// qdbusxml2cpp doesn't really do much, and has a number of limitations,
// such as the limits on number of arguments for QDBusPendingReply template.
// It's easier to write these proxies by hand.
// ==========================================================================

class QOfonoExtSimInfoProxy: public QDBusAbstractInterface
{
    Q_OBJECT

public:
    static const QString INTERFACE;
    QOfonoExtSimInfoProxy(QString aPath, QObject* aParent) :
        QDBusAbstractInterface(OFONO_SERVICE, aPath, qPrintable(INTERFACE),
            OFONO_BUS, aParent) {}

public Q_SLOTS: // METHODS
    QDBusPendingCall GetInterfaceVersion()
        { return asyncCall("GetInterfaceVersion"); }
    QDBusPendingCall GetAll()
        { return asyncCall("GetAll"); }

Q_SIGNALS: // SIGNALS
    void CardIdentifierChanged(QString aIccid);
    void ServiceProviderNameChanged(QString aSpn);
    void SubscriberIdentityChanged(QString aImsi);
};

const QString QOfonoExtSimInfoProxy::INTERFACE("org.nemomobile.ofono.SimInfo");

// ==========================================================================
// QOfonoExtSimInfo::Private
// ==========================================================================

class QOfonoExtSimInfo::Private : public QObject
{
    Q_OBJECT

public:
    QOfonoExtSimInfo* iParent;
    QOfonoExtSimInfoProxy* iProxy;
    QSharedPointer<QOfonoModem> iModem;
    bool iValid;
    QString iModemPath;
    QString iCardIdentifier;
    QString iSubscriberIdentity;
    QString iServiceProviderName;

    Private(QOfonoExtSimInfo* aParent);

    QString modemPath() const;
    void setModemPath(QString aPath);
    void invalidate();
    void getAll();

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
        iParent->modemPathChanged(modemPath());
    }
}

void QOfonoExtSimInfo::Private::checkInterfacePresence()
{
    if (iModem && iModem->isValid() &&
        iModem->interfaces().contains(QOfonoExtSimInfoProxy::INTERFACE)) {
        if (!iProxy) {
            iProxy = new QOfonoExtSimInfoProxy(iModem->objectPath(), this);
            if (iProxy->isValid()) {
                connect(iProxy,
                    SIGNAL(CardIdentifierChanged(QString)),
                    SLOT(onCardIdentifierChanged(QString)));
                connect(iProxy,
                    SIGNAL(SubscriberIdentityChanged(QString)),
                    SLOT(onSubscriberIdentityChanged(QString)));
                connect(iProxy,
                    SIGNAL(ServiceProviderNameChanged(QString)),
                    SLOT(onServiceProviderNameChanged(QString)));
                getAll();
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

void QOfonoExtSimInfo::Private::getAll()
{
    connect(new QDBusPendingCallWatcher(iProxy->GetAll(), iProxy),
        SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onGetAllFinished(QDBusPendingCallWatcher*)));
}

void QOfonoExtSimInfo::Private::onGetAllFinished(QDBusPendingCallWatcher* aWatcher)
{
    QDBusPendingReply<int,      // InterfaceVersion
        QString,                // CardIdentifier
        QString,                // SubscriberIdentity
        QString>                // ServiceProviderName
        reply(*aWatcher);
    if (reply.isError()) {
        // Repeat the call on timeout
        qWarning() << reply.error();
        if (QOfonoExt::isTimeout(reply.error())) {
            getAll();
        }
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
