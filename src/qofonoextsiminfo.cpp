/****************************************************************************
**
+* Copyright (C) 2026 Jolla Mobile Ltd
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

class QOfonoExtSimInfoProxy:
    public QDBusAbstractInterface
{
    Q_OBJECT

public:
    static const QString INTERFACE;
    QOfonoExtSimInfoProxy(const QString& aPath, QObject* aParent) :
        QDBusAbstractInterface(OFONO_SERVICE, aPath, qPrintable(INTERFACE),
            OFONO_BUS, aParent) {}

public Q_SLOTS: // METHODS
    QDBusPendingCall GetInterfaceVersion()
        { return asyncCall("GetInterfaceVersion"); }
    QDBusPendingCall GetAll()
        { return asyncCall("GetAll"); }
    QDBusPendingCall GetAll2()
        { return asyncCall("GetAll2"); }
    QDBusPendingCall SetCardLabel(const QString& aLabel)
        { return asyncCall("SetCardLabel", aLabel); }

Q_SIGNALS: // SIGNALS
    void CardIdentifierChanged(QString);
    void ServiceProviderNameChanged(QString);
    void SubscriberIdentityChanged(QString);
    void CardLabelChanged(QString);
};

const QString QOfonoExtSimInfoProxy::INTERFACE("org.nemomobile.ofono.SimInfo");

// ==========================================================================
// QOfonoExtSimInfo::Private
// ==========================================================================

class QOfonoExtSimInfo::Private :
    public QObject
{
    Q_OBJECT

public:
    QOfonoExtSimInfoProxy* iProxy;
    QSharedPointer<QOfonoModem> iModem;
    bool iValid;
    QString iModemPath;
    QString iCardIdentifier;
    QString iSubscriberIdentity;
    QString iServiceProviderName;
    QString iCardLabel;
    QScopedPointer<QString> iSetCardLabel;

    Private(QOfonoExtSimInfo*);

    QOfonoExtSimInfo* parentObject() const;
    QString modemPath() const;
    void setModemPath(const QString&);
    void setCardLabel(const QString&);
    void invalidate();
    void getInterfaceVersion();
    void getAll();
    void getAll2();

private: // SLOTS
    void checkInterfacePresence();
    void onGetInterfaceVersionFinished(QDBusPendingCallWatcher*);
    void onGetAllFinished(QDBusPendingCallWatcher*);
    void onGetAll2Finished(QDBusPendingCallWatcher*);
    void onCardIdentifierChanged(const QString&);
    void onSubscriberIdentityChanged(const QString&);
    void onServiceProviderNameChanged(const QString&);
    void onCardLabelChanged(const QString&);
};

QOfonoExtSimInfo::Private::Private(
    QOfonoExtSimInfo* aParent) :
    QObject(aParent),
    iProxy(Q_NULLPTR),
    iValid(false)
{}

QOfonoExtSimInfo*
QOfonoExtSimInfo::Private::parentObject() const
{
    return qobject_cast<QOfonoExtSimInfo*>(parent());
}

QString
QOfonoExtSimInfo::Private::modemPath() const
{
    return iModem.isNull() ? QString() : iModem->objectPath();
}

void
QOfonoExtSimInfo::Private::setModemPath(
    const QString& aPath)
{
    if (aPath != modemPath()) {
        if (aPath.isEmpty()) {
            iModem.clear();
            invalidate();
        } else {
            if (iModem) iModem->disconnect(this);
            iModem = QOfonoModem::instance(aPath);
            connect(iModem.data(), &QOfonoModem::validChanged,
                this, &Private::checkInterfacePresence);
            connect(iModem.data(), &QOfonoModem::interfacesChanged,
                this, &Private::checkInterfacePresence);
            invalidate();
            checkInterfacePresence();
        }
        Q_EMIT parentObject()->modemPathChanged(modemPath());
    }
}

void
QOfonoExtSimInfo::Private::setCardLabel(
    const QString& aLabel)
{
    // It won't work if API v2 is not supported by ofono... But at least we try
    if (iProxy) {
        iProxy->SetCardLabel(aLabel);
    } else {
        iSetCardLabel.reset(new QString(aLabel));
    }
}

void
QOfonoExtSimInfo::Private::checkInterfacePresence()
{
    if (iModem && iModem->isValid() &&
        iModem->interfaces().contains(QOfonoExtSimInfoProxy::INTERFACE)) {
        if (!iProxy) {
            iProxy = new QOfonoExtSimInfoProxy(iModem->objectPath(), this);
            if (iProxy->isValid()) {
                connect(iProxy, &QOfonoExtSimInfoProxy::CardIdentifierChanged,
                    this, &Private::onCardIdentifierChanged);
                connect(iProxy, &QOfonoExtSimInfoProxy::SubscriberIdentityChanged,
                    this, &Private::onSubscriberIdentityChanged);
                connect(iProxy, &QOfonoExtSimInfoProxy::ServiceProviderNameChanged,
                    this, &Private::onServiceProviderNameChanged);
                connect(iProxy, &QOfonoExtSimInfoProxy::CardLabelChanged,
                    this, &Private::onCardLabelChanged);
                if (iSetCardLabel) {
                    // Hope that API v2 is supported by ofono
                    iProxy->SetCardLabel(*iSetCardLabel);
                    iSetCardLabel.reset();
                }
                getInterfaceVersion();
            } else {
                invalidate();
            }
        }
    } else {
        invalidate();
    }
}

void
QOfonoExtSimInfo::Private::invalidate()
{
    if (iProxy) {
        delete iProxy;
        iProxy = Q_NULLPTR;
    }
    if (iValid) {
        iValid = false;
        Q_EMIT parentObject()->validChanged(false);
    }
}

void
QOfonoExtSimInfo::Private::getInterfaceVersion()
{
    connect(new QDBusPendingCallWatcher(iProxy->GetInterfaceVersion(), iProxy),
        &QDBusPendingCallWatcher::finished, this,
        &Private::onGetInterfaceVersionFinished);
}

void
QOfonoExtSimInfo::Private::getAll()
{
    connect(new QDBusPendingCallWatcher(iProxy->GetAll(), iProxy),
        &QDBusPendingCallWatcher::finished, this,
        &Private::onGetAllFinished);
}

void
QOfonoExtSimInfo::Private::getAll2()
{
    connect(new QDBusPendingCallWatcher(iProxy->GetAll2(), iProxy),
        &QDBusPendingCallWatcher::finished, this,
        &Private::onGetAll2Finished);
}

void
QOfonoExtSimInfo::Private::onGetInterfaceVersionFinished(
    QDBusPendingCallWatcher* aWatcher)
{
    QDBusPendingReply<int> reply(*aWatcher);

    if (reply.isError()) {
        // Repeat the call on timeout
        qWarning() << reply.error();
        if (QOfonoExt::isTimeout(reply.error())) {
            getInterfaceVersion();
        }
    } else if (reply.value() < 2) {
        getAll();
    } else {
        getAll2();
    }
    aWatcher->deleteLater();
}

void
QOfonoExtSimInfo::Private::onGetAllFinished(
    QDBusPendingCallWatcher* aWatcher)
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
        QOfonoExtSimInfo* obj = parentObject();
        const QString iccid(reply.argumentAt<1>());
        const QString imsi(reply.argumentAt<2>());
        const QString spn(reply.argumentAt<3>());

        if (iCardIdentifier != iccid) {
            iCardIdentifier = iccid;
            Q_EMIT obj->cardIdentifierChanged(iccid);
        }
        if (iSubscriberIdentity != imsi) {
            iSubscriberIdentity = imsi;
            Q_EMIT obj->subscriberIdentityChanged(imsi);
        }
        if (iServiceProviderName != spn) {
            iServiceProviderName = spn;
            Q_EMIT obj->serviceProviderNameChanged(spn);
        }
        if (!iValid) {
            iValid = true;
            Q_EMIT obj->validChanged(iValid);
        }
    }
    aWatcher->deleteLater();
}

void
QOfonoExtSimInfo::Private::onGetAll2Finished(
    QDBusPendingCallWatcher* aWatcher)
{
    QDBusPendingReply<int,          // InterfaceVersion
                      QString,      // CardIdentifier
                      QString,      // SubscriberIdentity
                      QString,      // ServiceProviderName
                      QString>      // CardLabel
        reply(*aWatcher);

    if (reply.isError()) {
        // Repeat the call on timeout
        qWarning() << reply.error();
        if (QOfonoExt::isTimeout(reply.error())) {
            getAll2();
        }
    } else {
        QOfonoExtSimInfo* obj = parentObject();
        const QString iccid(reply.argumentAt<1>());
        const QString imsi(reply.argumentAt<2>());
        const QString spn(reply.argumentAt<3>());
        const QString label(reply.argumentAt<4>());

        if (iCardIdentifier != iccid) {
            iCardIdentifier = iccid;
            Q_EMIT obj->cardIdentifierChanged(iccid);
        }
        if (iSubscriberIdentity != imsi) {
            iSubscriberIdentity = imsi;
            Q_EMIT obj->subscriberIdentityChanged(imsi);
        }
        if (iServiceProviderName != spn) {
            iServiceProviderName = spn;
            Q_EMIT obj->serviceProviderNameChanged(spn);
        }
        if (iCardLabel != label) {
            iCardLabel = label;
            Q_EMIT obj->cardLabelChanged(label);
        }
        if (!iValid) {
            iValid = true;
            Q_EMIT obj->validChanged(iValid);
        }
    }
    aWatcher->deleteLater();
}

void
QOfonoExtSimInfo::Private::onCardIdentifierChanged(
    const QString& aValue)
{
    if (iCardIdentifier != aValue) {
        iCardIdentifier = aValue;
        Q_EMIT parentObject()->cardIdentifierChanged(aValue);
    }
}

void
QOfonoExtSimInfo::Private::onSubscriberIdentityChanged(
    const QString& aValue)
{
    if (iSubscriberIdentity != aValue) {
        iSubscriberIdentity = aValue;
        Q_EMIT parentObject()->subscriberIdentityChanged(aValue);
    }
}

void
QOfonoExtSimInfo::Private::onServiceProviderNameChanged(
    const QString& aValue)
{
    if (iServiceProviderName != aValue) {
        iServiceProviderName = aValue;
        QOfonoExtSimInfo* obj = parentObject();

        Q_EMIT obj->serviceProviderNameChanged(aValue);
        if (iCardLabel.isEmpty()) {
            Q_EMIT obj->cardLabelChanged(aValue);
        }
    }
}

void
QOfonoExtSimInfo::Private::onCardLabelChanged(
    const QString& aValue)
{
    if (iCardLabel != aValue) {
        iCardLabel = aValue;
        Q_EMIT parentObject()->cardLabelChanged(aValue);
    }
}

// ==========================================================================
// QOfonoExtSimInfo
// ==========================================================================

QOfonoExtSimInfo::QOfonoExtSimInfo(
    QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{}

QOfonoExtSimInfo::~QOfonoExtSimInfo()
{}

bool
QOfonoExtSimInfo::valid() const
{
    return iPrivate->iValid;
}

QString
QOfonoExtSimInfo::modemPath() const
{
    return iPrivate->modemPath();
}

void
QOfonoExtSimInfo::setModemPath(
    QString aPath)
{
    iPrivate->setModemPath(aPath);
}

QString
QOfonoExtSimInfo::cardIdentifier() const
{
    return iPrivate->iCardIdentifier;
}

QString
QOfonoExtSimInfo::subscriberIdentity() const
{
    return iPrivate->iSubscriberIdentity;
}

QString
QOfonoExtSimInfo::serviceProviderName() const
{
    return iPrivate->iServiceProviderName;
}

// Since 1.2.0
QString
QOfonoExtSimInfo::cardLabel() const
{
    return iPrivate->iCardLabel;
}

void
QOfonoExtSimInfo::setCardLabel(
    QString aLabel)
{
    iPrivate->setCardLabel(aLabel);
}

#include "qofonoextsiminfo.moc"
