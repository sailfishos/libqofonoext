/****************************************************************************
**
** Copyright (C) 2015-2016 Jolla Ltd.
** Contact: slava.monich@jollamobile.com
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

#include "qofonoextsimsettings.h"
#include "qofonoext_p.h"

#include "simsettings_interface.h"

#include <qofonomodem.h>

class QOfonoExtSimSettings::Private : public QObject
{
    Q_OBJECT

public:
    QOfonoExtSimSettings* iParent;
    QOfonoExtSimSettingsProxy* iProxy;
    QSharedPointer<QOfonoModem> iModem;
    QString iInterfaceName;
    bool iValid;
    bool iEnable4G;
    QString iModemPath;
    QString iDisplayName;

    Private(QOfonoExtSimSettings* aParent);

    QString modemPath() const;
    void setModemPath(QString aPath);
    void setDisplayName(QString aName);
    void invalidate();

private Q_SLOTS:
    void checkInterfacePresence();
    void onGetAllFinished(QDBusPendingCallWatcher* aWatcher);
    void onDisplayNameChanged(QString aName);
};

QOfonoExtSimSettings::Private::Private(QOfonoExtSimSettings* aParent) :
    QObject(aParent),
    iParent(aParent),
    iProxy(NULL),
    iInterfaceName(QOfonoExtSimSettingsProxy::staticInterfaceName()),
    iValid(false)
{
}

QString QOfonoExtSimSettings::Private::modemPath() const
{
    return iModem.isNull() ? QString() : iModem->objectPath();
}

void QOfonoExtSimSettings::Private::setModemPath(QString aPath)
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

void QOfonoExtSimSettings::Private::checkInterfacePresence()
{
    if (iModem && iModem->isValid() &&
        iModem->interfaces().contains(iInterfaceName)) {
        if (!iProxy) {
            iProxy = new QOfonoExtSimSettingsProxy(OFONO_SERVICE, iModem->objectPath(), OFONO_BUS, this);
            if (iProxy->isValid()) {
                connect(new QDBusPendingCallWatcher(iProxy->GetAll(), iProxy),
                    SIGNAL(finished(QDBusPendingCallWatcher*)),
                    SLOT(onGetAllFinished(QDBusPendingCallWatcher*)));
                connect(iProxy,
                    SIGNAL(DisplayNameChanged(QString)),
                    SLOT(onDisplayNameChanged(QString)));
            } else {
                invalidate();
            }
        }
    } else {
        invalidate();
    }
}

void QOfonoExtSimSettings::Private::invalidate()
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

void QOfonoExtSimSettings::Private::setDisplayName(QString aName)
{
    if (iProxy) {
        iProxy->SetDisplayName(aName);
    }
    // Optimistically cache the changes
    if (iDisplayName != aName) {
        iDisplayName = aName;
        Q_EMIT iParent->displayNameChanged(aName);
    }
}

void QOfonoExtSimSettings::Private::onGetAllFinished(QDBusPendingCallWatcher* aWatcher)
{
    QDBusPendingReply<int,      // InterfaceVersion
        bool,                   // Enable4G
        QString>                // DisplayName
        reply(*aWatcher);
    if (reply.isError()) {
        qWarning() << reply.error();
    } else {
        bool enable4G = reply.argumentAt<1>();
        if (iEnable4G != enable4G) {
            iEnable4G = enable4G;
            Q_EMIT iParent->enable4GChanged(enable4G);
        }
        QString name = reply.argumentAt<2>();
        if (iDisplayName != name) {
            iDisplayName = name;
            Q_EMIT iParent->displayNameChanged(name);
        }
        if (!iValid) {
            iValid = true;
            Q_EMIT iParent->validChanged(iValid);
        }
    }
    aWatcher->deleteLater();
}

void QOfonoExtSimSettings::Private::onDisplayNameChanged(QString aName)
{
    if (iDisplayName != aName) {
        iDisplayName = aName;
        Q_EMIT iParent->displayNameChanged(aName);
    }
}

// ==========================================================================
// QOfonoExtSimSettings
// ==========================================================================

QOfonoExtSimSettings::QOfonoExtSimSettings(QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

QOfonoExtSimSettings::~QOfonoExtSimSettings()
{
}

bool QOfonoExtSimSettings::valid() const
{
    return iPrivate->iValid;
}

bool QOfonoExtSimSettings::enable4G() const
{
    return iPrivate->iEnable4G;
}

QString QOfonoExtSimSettings::modemPath() const
{
    return iPrivate->modemPath();
}

QString QOfonoExtSimSettings::displayName() const
{
    return iPrivate->iDisplayName;
}

void QOfonoExtSimSettings::setDisplayName(QString aName)
{
    iPrivate->setDisplayName(aName);
}

void QOfonoExtSimSettings::setModemPath(QString aPath)
{
    iPrivate->setModemPath(aPath);
}

#include "qofonoextsimsettings.moc"
