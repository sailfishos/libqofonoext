/****************************************************************************
**
** Copyright (C) 2015-2017 Jolla Ltd.
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

#include "qofonoextmodemmanager.h"
#include "qofonoext_p.h"

// ==========================================================================
// QOfonoExtModemManagerProxy
//
// qdbusxml2cpp doesn't really do much, and has a number of limitations,
// such as the limits on number of arguments for QDBusPendingReply template.
// It's easier to write these proxies by hand.
// ==========================================================================

class QOfonoExtModemManagerProxy: public QDBusAbstractInterface
{
    Q_OBJECT

public:

    class Error {
    public:
        Error() : iCount(0) {}
    public:
        QString iName;
        int iCount;
    };


    QOfonoExtModemManagerProxy(QObject* aParent) :
        QDBusAbstractInterface(OFONO_SERVICE, "/",
            "org.nemomobile.ofono.ModemManager", OFONO_BUS, aParent),
        iInterfaceVersion(0) {}

public Q_SLOTS: // METHODS
    QDBusPendingCall GetInterfaceVersion()
        { return asyncCall("GetInterfaceVersion"); }
    QDBusPendingCall GetAll()
        { return asyncCall("GetAll"); }
    QDBusPendingCall GetAll2()
        { return asyncCall("GetAll2"); }
    QDBusPendingCall GetAll3()
        { return asyncCall("GetAll3"); }
    QDBusPendingCall GetAll4()
        { return asyncCall("GetAll4"); }
    QDBusPendingCall GetAll5()
        { return asyncCall("GetAll5"); }
    QDBusPendingCall GetAll6()
        { return asyncCall("GetAll6"); }
    QDBusPendingCall GetAll7()
        { return asyncCall("GetAll7"); }
    QDBusPendingCall GetAll8()
        { return asyncCall("GetAll8"); }
    QDBusPendingCall SetDefaultDataSim(QString aImsi)
        { return asyncCall("SetDefaultDataSim", aImsi); }
    QDBusPendingCall SetDefaultVoiceSim(const QString &aImsi)
        { return asyncCall("SetDefaultVoiceSim", aImsi); }
    QDBusPendingCall SetEnabledModems(QList<QDBusObjectPath> aModems)
        { return asyncCall("SetEnabledModems", qVariantFromValue(aModems)); }

Q_SIGNALS: // SIGNALS
    void DefaultDataModemChanged(QString aPath);
    void DefaultDataSimChanged(QString aImsi);
    void DefaultVoiceModemChanged(QString aPath);
    void DefaultVoiceSimChanged(QString aIimsi);
    void EnabledModemsChanged(QList<QDBusObjectPath> aModems);
    void MmsModemChanged(QString aPath);
    void MmsSimChanged(QString aImsi);
    void PresentSimsChanged(int aIndex, bool aPresent);
    void ReadyChanged(bool aReady);
    void ModemError(QDBusObjectPath aModem, QString aName, QString aMessage);

public:
    // Becomes non-zero after GetInterfaceVersion succeeds:
    int iInterfaceVersion;
};

Q_DECLARE_METATYPE(QOfonoExtModemManagerProxy::Error)

QDBusArgument& operator<<(QDBusArgument& aArg, const QOfonoExtModemManagerProxy::Error& aValue)
{
    aArg.beginStructure();
    aArg << aValue.iName;
    aArg << aValue.iCount;
    aArg.endStructure();
    return aArg;
}

const QDBusArgument& operator>>(const QDBusArgument &aArg, QOfonoExtModemManagerProxy::Error& aValue)
{
    aArg.beginStructure();
    aArg >> aValue.iName;
    aArg >> aValue.iCount;
    aArg.endStructure();
    return aArg;
}

// ==========================================================================
// QOfonoExtModemManager::Private
// ==========================================================================

class QOfonoExtModemManager::Private : public QObject
{
    Q_OBJECT
    typedef QList<QOfonoExtModemManagerProxy::Error> ErrorList;
    typedef QList<ErrorList> ModemErrors;

public:
    static QWeakPointer<QOfonoExtModemManager> sSharedInstance;

    QOfonoExtModemManager* iParent;
    QOfonoExtModemManagerProxy* iProxy;
    QDBusPendingCallWatcher* iInitCall;
    QStringList iAvailableModems;
    QStringList iEnabledModems;
    QString iDefaultVoiceModem;
    QString iDefaultDataModem;
    QString iDefaultVoiceSim;
    QString iDefaultDataSim;
    QList<bool> iPresentSims;
    QStringList iIMEIs;
    QStringList iIMEISVs;
    QString iMmsSim;
    QString iMmsModem;
    int iPresentSimCount;
    int iActiveSimCount;
    int iInterfaceVersion;
    bool iReady;
    bool iValid;
    int iErrorCount;

    Private(QOfonoExtModemManager* aParent);

    static QStringList toStringList(QList<QDBusObjectPath> aList);
    static QList<QDBusObjectPath> toPathList(QStringList aList);
    QStringList dummyStringList();

    void getAll();
    void getInterfaceVersion();
    void presentSimsChanged(QList<bool> aOldList);
    void updateSimCounts();
    void updateEnabledModems(QStringList aModems);
    void updateDefaultDataModem(QString aPath);
    void updateDefaultVoiceModem(QString aPath);
    void updateDefaultDataSim(QString aImsi);
    void updateDefaultVoiceSim(QString aImsi);
    void updateMmsSim(QString aImsi);
    void updateMmsModem(QString aPath);
    void updateReady(bool aReady);

private Q_SLOTS:
    void onServiceRegistered();
    void onServiceUnregistered();
    void onGetInterfaceVersionFinished(QDBusPendingCallWatcher* aWatcher);
    void onGetAllFinished(QDBusPendingCallWatcher* aWatcher);
    void onEnabledModemsChanged(QList<QDBusObjectPath> aModems);
    void onDefaultVoiceModemChanged(QString aModemPath);
    void onDefaultDataModemChanged(QString aModemPath);
    void onDefaultVoiceSimChanged(QString aImsi);
    void onDefaultDataSimChanged(QString aImsi);
    void onPresentSimsChanged(int aIndex, bool aPresent);
    void onMmsSimChanged(QString aImsi);
    void onMmsModemChanged(QString aModemPath);
    void onReadyChanged(bool aReady);
    void onModemError(QDBusObjectPath aModem, QString aName, QString aMessage);
};

QWeakPointer<QOfonoExtModemManager> QOfonoExtModemManager::Private::sSharedInstance;

QStringList QOfonoExtModemManager::Private::toStringList(QList<QDBusObjectPath> aList)
{
    QStringList stringList;
    const int n = aList.count();
    for (int i=0; i<n; i++) {
        stringList.append(aList.at(i).path());
    }
    return stringList;
}

QList<QDBusObjectPath> QOfonoExtModemManager::Private::toPathList(QStringList aList)
{
    QList<QDBusObjectPath> pathList;
    const int n = aList.count();
    for (int i=0; i<n; i++) {
        pathList.append(QDBusObjectPath(aList.at(i)));
    }
    return pathList;
}

QOfonoExtModemManager::Private::Private(QOfonoExtModemManager* aParent) :
    QObject(aParent),
    iParent(aParent),
    iProxy(NULL),
    iPresentSimCount(0),
    iActiveSimCount(0),
    iInterfaceVersion(0),
    iReady(false),
    iValid(false),
    iErrorCount(0)
{
    qRegisterMetaType<QOfonoExtModemManagerProxy::Error>("QOfonoExtModemManagerProxy::Error");
    qDBusRegisterMetaType<QOfonoExtModemManagerProxy::Error>();

    QDBusServiceWatcher* ofonoWatcher = new QDBusServiceWatcher(OFONO_SERVICE,
        OFONO_BUS, QDBusServiceWatcher::WatchForRegistration |
        QDBusServiceWatcher::WatchForUnregistration, this);

    connect(ofonoWatcher, SIGNAL(serviceRegistered(QString)),
        this, SLOT(onServiceRegistered()));
    connect(ofonoWatcher, SIGNAL(serviceUnregistered(QString)),
        this, SLOT(onServiceUnregistered()));

    if (OFONO_BUS.interface()->isServiceRegistered(OFONO_SERVICE)) {
        onServiceRegistered();
    }
}

void QOfonoExtModemManager::Private::onServiceRegistered()
{
    const bool wasValid = iValid;
    if (!iProxy) {
        iProxy = new QOfonoExtModemManagerProxy(this);
        if (iProxy->isValid()) {
            iValid = false;
            connect(iProxy,
                SIGNAL(EnabledModemsChanged(QList<QDBusObjectPath>)),
                SLOT(onEnabledModemsChanged(QList<QDBusObjectPath>)));
            connect(iProxy,
                SIGNAL(DefaultDataModemChanged(QString)),
                SLOT(onDefaultDataModemChanged(QString)));
            connect(iProxy,
                SIGNAL(DefaultVoiceModemChanged(QString)),
                SLOT(onDefaultVoiceModemChanged(QString)));
            connect(iProxy,
                SIGNAL(DefaultDataSimChanged(QString)),
                SLOT(onDefaultDataSimChanged(QString)));
            connect(iProxy,
                SIGNAL(DefaultVoiceSimChanged(QString)),
                SLOT(onDefaultVoiceSimChanged(QString)));
            connect(iProxy,
                SIGNAL(PresentSimsChanged(int,bool)),
                SLOT(onPresentSimsChanged(int,bool)));
            getInterfaceVersion();
        } else {
            delete iProxy;
            iProxy = NULL;
        }
    }
    if (wasValid != iValid) {
        Q_EMIT iParent->validChanged(iValid);
    }
}

void QOfonoExtModemManager::Private::onServiceUnregistered()
{
    if (iProxy) {
        // iProxy is the parent of iInitCall
        iInitCall = NULL;
        delete iProxy;
        iProxy = NULL;
    }
    if (iValid) {
        iValid = false;
        Q_EMIT iParent->validChanged(iValid);
    }
}

void QOfonoExtModemManager::Private::getInterfaceVersion()
{
    iInitCall = new QDBusPendingCallWatcher(iProxy->GetInterfaceVersion(), iProxy);
    connect(iInitCall, SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onGetInterfaceVersionFinished(QDBusPendingCallWatcher*)));
}

void QOfonoExtModemManager::Private::getAll()
{
    iInitCall = new QDBusPendingCallWatcher(
        (iInterfaceVersion == 2) ? QDBusPendingCall(iProxy->GetAll2()) :
        (iInterfaceVersion == 3) ? QDBusPendingCall(iProxy->GetAll3()) :
        (iInterfaceVersion == 4) ? QDBusPendingCall(iProxy->GetAll4()) :
        (iInterfaceVersion == 5) ? QDBusPendingCall(iProxy->GetAll5()) :
        (iInterfaceVersion == 6) ? QDBusPendingCall(iProxy->GetAll6()) :
        (iInterfaceVersion == 7) ? QDBusPendingCall(iProxy->GetAll7()) :
        QDBusPendingCall(iProxy->GetAll8()), iProxy);
    connect(iInitCall, SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onGetAllFinished(QDBusPendingCallWatcher*)));
}

void QOfonoExtModemManager::Private::onGetInterfaceVersionFinished(QDBusPendingCallWatcher* aWatcher)
{
    QDBusPendingReply<int> reply(*aWatcher);
    iInitCall = NULL;
    if (reply.isError()) {
        // Repeat the call on timeout
        qWarning() << reply.error();
        if (QOfonoExt::isTimeout(reply.error())) {
            getInterfaceVersion();
        }
    } else {
        const int version = reply.value();
        // Make sure we don't connect signals more than once
        if (version > iProxy->iInterfaceVersion) {
            if (version >= 4 && iProxy->iInterfaceVersion < 4) {
                connect(iProxy,
                    SIGNAL(MmsSimChanged(QString)),
                    SLOT(onMmsSimChanged(QString)));
                connect(iProxy,
                    SIGNAL(MmsModemChanged(QString)),
                    SLOT(onMmsModemChanged(QString)));
            }
            if (version >= 5 && iProxy->iInterfaceVersion < 5) {
                connect(iProxy,
                    SIGNAL(ReadyChanged(bool)),
                    SLOT(onReadyChanged(bool)));
            }
            if (version >= 6 && iProxy->iInterfaceVersion < 6) {
                connect(iProxy,
                    SIGNAL(ModemError(QDBusObjectPath,QString,QString)),
                    SLOT(onModemError(QDBusObjectPath,QString,QString)));
            }
            iProxy->iInterfaceVersion = version;
        }
        if (iInterfaceVersion != version) {
            iInterfaceVersion = version;
            iParent->interfaceVersionChanged(version);
        }
        getAll();
    }
    aWatcher->deleteLater();
}

void QOfonoExtModemManager::Private::onGetAllFinished(QDBusPendingCallWatcher* aWatcher)
{
    QDBusPendingReply<int,      // InterfaceVersion
        QList<QDBusObjectPath>, // AvailableModems
        QList<QDBusObjectPath>, // EnabledModems
        QString,                // DefaultDataSim
        QString,                // DefaultVoiceSim
        QString,                // DefaultDataModem
        QString,                // DefaultVoiceModem
        QList<bool> >           // PresentSims
        reply(*aWatcher);
    iInitCall = NULL;
    if (reply.isError()) {
        // Repeat the call on timeout
        qWarning() << reply.error();
        if (QOfonoExt::isTimeout(reply.error())) {
            getAll();
        }
    } else {
        const int version = reply.argumentAt<0>();
        QStringList list = toStringList(reply.argumentAt<1>());
        if (iAvailableModems != list) {
            iAvailableModems = list;
            Q_EMIT iParent->availableModemsChanged(iAvailableModems);
        }
        updateEnabledModems(toStringList(reply.argumentAt<2>()));
        updateDefaultDataSim(reply.argumentAt<3>());
        updateDefaultVoiceSim(reply.argumentAt<4>());
        updateDefaultDataModem(reply.argumentAt<5>());
        updateDefaultVoiceModem(reply.argumentAt<6>());

        QList<bool> oldList = iPresentSims;
        iPresentSims = reply.argumentAt<7>();
        presentSimsChanged(oldList);

        if (version >= 3) {
            // 8: imei
            list = reply.argumentAt(8).toStringList();
        } else {
            list = dummyStringList();
        }

        if (iIMEIs != list) {
            iIMEIs = list;
            Q_EMIT iParent->imeiCodesChanged(iIMEIs);
        }

        if (version >= 4) {
            // 9: mmsSim
            // 10: mmsModem
            updateMmsSim(reply.argumentAt(9).toString());
            updateMmsModem(reply.argumentAt(10).toString());
        }

        if (version >= 5) {
            // 11: ready
            updateReady(reply.argumentAt(11).toBool());
        } else {
            // Old ofono is always ready :)
            updateReady(true);
        }

        int errorCount = 0;
        if (version >= 6) {
            // 12: modemErrors
            ModemErrors me = qdbus_cast<ModemErrors>(reply.argumentAt(12));
            const int n = me.count();
            for (int i=0; i<n; i++) {
                const ErrorList& errors = me.at(i);
                const int k = errors.count();
                for (int j=0; j<k; j++) {
                    errorCount += errors.at(j).iCount;
                }
            }
        }

        if (version >= 7) {
            // 13: imeisv
            list = reply.argumentAt(13).toStringList();
        } else {
            list = dummyStringList();
        }

        if (version >= 8) {
            // 14: errors
            ErrorList errors = qdbus_cast<ErrorList>(reply.argumentAt(14));
            const int k = errors.count();
            for (int i=0; i<k; i++) {
                errorCount += errors.at(i).iCount;
            }
        }

        if (iErrorCount != errorCount) {
            iErrorCount = errorCount;
            Q_EMIT iParent->errorCountChanged(errorCount);
        }

        if (iIMEISVs != list) {
            iIMEISVs = list;
            Q_EMIT iParent->imeisvCodesChanged(iIMEISVs);
        }

        if (!iValid) {
            iValid = true;
            Q_EMIT iParent->validChanged(iValid);
        }
    }
    aWatcher->deleteLater();
}

QStringList QOfonoExtModemManager::Private::dummyStringList()
{
    QStringList list;
    const int n = iAvailableModems.count();
    for (int i=0; i<n; i++) {
        list.append(QString());
    }
    return list;
}

void QOfonoExtModemManager::Private::presentSimsChanged(QList<bool> aOldList)
{
    int i;
    const int n = iPresentSims.count();
    QList<bool> changed;
    changed.reserve(n);
    for (i=0; i<n; i++) {
        const bool prev = (i<aOldList.count() && aOldList.at(i));
        changed.append(iPresentSims.at(i) != prev);
    }
    updateSimCounts();
    for (i=0; i<n; i++) {
        if (changed.at(i)) {
            Q_EMIT iParent->presentSimChanged(i, iPresentSims.at(i));
        }
    }
    if (aOldList != iPresentSims) {
        Q_EMIT iParent->presentSimsChanged(iPresentSims);
    }
}

void QOfonoExtModemManager::Private::updateSimCounts()
{
    const int oldPresentSimCount = iPresentSimCount;
    const int oldActiveSimCount = iActiveSimCount;

    iPresentSimCount = 0;
    iActiveSimCount = 0;

    const int n = iPresentSims.count();
    for (int i=0; i<n; i++) {
        if (iPresentSims.at(i)) {
            iPresentSimCount++;
            if (i < iAvailableModems.count() &&
                iEnabledModems.contains(iAvailableModems.at(i))) {
                iActiveSimCount++;
            }
        }
    }
    if (oldPresentSimCount != iPresentSimCount) {
        Q_EMIT iParent->presentSimCountChanged(iPresentSimCount);
    }
    if (oldActiveSimCount != iActiveSimCount) {
        Q_EMIT iParent->activeSimCountChanged(iActiveSimCount);
    }
}

void QOfonoExtModemManager::Private::updateEnabledModems(QStringList aModems)
{
    if (iEnabledModems != aModems) {
        iEnabledModems = aModems;
        Q_EMIT iParent->enabledModemsChanged(aModems);
    }
    updateSimCounts();
}

void QOfonoExtModemManager::Private::updateDefaultDataModem(QString aPath)
{
    if (iDefaultDataModem != aPath) {
        iDefaultDataModem = aPath;
        Q_EMIT iParent->defaultDataModemChanged(aPath);
    }
}

void QOfonoExtModemManager::Private::updateDefaultVoiceModem(QString aPath)
{
    if (iDefaultVoiceModem != aPath) {
        iDefaultVoiceModem = aPath;
        Q_EMIT iParent->defaultVoiceModemChanged(aPath);
    }
}

void QOfonoExtModemManager::Private::updateDefaultDataSim(QString aImsi)
{
    if (iDefaultDataSim != aImsi) {
        iDefaultDataSim = aImsi;
        Q_EMIT iParent->defaultDataSimChanged(aImsi);
    }
}

void QOfonoExtModemManager::Private::updateDefaultVoiceSim(QString aImsi)
{
    if (iDefaultVoiceSim != aImsi) {
        iDefaultVoiceSim = aImsi;
        Q_EMIT iParent->defaultVoiceSimChanged(aImsi);
    }
}

void QOfonoExtModemManager::Private::updateMmsSim(QString aImsi)
{
    if (iMmsSim != aImsi) {
        iMmsSim = aImsi;
        Q_EMIT iParent->mmsSimChanged(aImsi);
    }
}

void QOfonoExtModemManager::Private::updateMmsModem(QString aPath)
{
    if (iMmsModem != aPath) {
        iMmsModem = aPath;
        Q_EMIT iParent->mmsModemChanged(aPath);
    }
}

void QOfonoExtModemManager::Private::updateReady(bool aReady)
{
    if (iReady != aReady) {
        iReady = aReady;
        Q_EMIT iParent->readyChanged(aReady);
    }
}

void QOfonoExtModemManager::Private::onEnabledModemsChanged(QList<QDBusObjectPath> aModems)
{
    if (!iInitCall) {
        updateEnabledModems(toStringList(aModems));
    }
}

void QOfonoExtModemManager::Private::onDefaultDataModemChanged(QString aPath)
{
    if (!iInitCall) {
        updateDefaultDataModem(aPath);
    }
}

void QOfonoExtModemManager::Private::onDefaultVoiceModemChanged(QString aPath)
{
    if (!iInitCall) {
        updateDefaultVoiceModem(aPath);
    }
}

void QOfonoExtModemManager::Private::onDefaultDataSimChanged(QString aImsi)
{
    if (!iInitCall) {
        updateDefaultDataSim(aImsi);
    }
}

void QOfonoExtModemManager::Private::onDefaultVoiceSimChanged(QString aImsi)
{
    if (!iInitCall) {
        updateDefaultVoiceSim(aImsi);
    }
}

void QOfonoExtModemManager::Private::onPresentSimsChanged(int aIndex, bool aPresent)
{
    if (!iInitCall && aIndex >= 0 && aIndex < iPresentSims.count()) {
        QList<bool> oldList = iPresentSims;
        iPresentSims[aIndex] = aPresent;
        presentSimsChanged(oldList);
    }
}

void QOfonoExtModemManager::Private::onMmsSimChanged(QString aImsi)
{
    if (!iInitCall) {
        updateMmsSim(aImsi);
    }
}

void QOfonoExtModemManager::Private::onMmsModemChanged(QString aPath)
{
    if (!iInitCall) {
        updateMmsModem(aPath);
    }
}

void QOfonoExtModemManager::Private::onReadyChanged(bool aReady)
{
    if (!iInitCall) {
        updateReady(aReady);
    }
}

void QOfonoExtModemManager::Private::onModemError(QDBusObjectPath aPath, QString aName, QString aMessage)
{
    if (!iInitCall) {
        iErrorCount++;
        Q_EMIT iParent->errorCountChanged(iErrorCount);
        Q_EMIT iParent->modemError(aPath.path(), aName, aMessage);
    }
}

// ==========================================================================
// QOfonoExtModemManager
// ==========================================================================

QOfonoExtModemManager::QOfonoExtModemManager(QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

QOfonoExtModemManager::~QOfonoExtModemManager()
{
}

bool QOfonoExtModemManager::valid() const
{
    return iPrivate->iValid;
}

int QOfonoExtModemManager::interfaceVersion() const
{
    return iPrivate->iInterfaceVersion;
}

QStringList QOfonoExtModemManager::availableModems() const
{
    return iPrivate->iAvailableModems;
}

QStringList QOfonoExtModemManager::enabledModems() const
{
    return iPrivate->iEnabledModems;
}

QString QOfonoExtModemManager::defaultVoiceModem() const
{
    return iPrivate->iDefaultVoiceModem;
}

QString QOfonoExtModemManager::defaultDataModem() const
{
    return iPrivate->iDefaultDataModem;
}

QString QOfonoExtModemManager::defaultVoiceSim() const
{
    return iPrivate->iDefaultVoiceSim;
}

QString QOfonoExtModemManager::defaultDataSim() const
{
    return iPrivate->iDefaultDataSim;
}

QList<bool> QOfonoExtModemManager::presentSims() const
{
    return iPrivate->iPresentSims;
}

QStringList QOfonoExtModemManager::imeiCodes() const
{
    return iPrivate->iIMEIs;
}

QStringList QOfonoExtModemManager::imeisvCodes() const
{
    return iPrivate->iIMEISVs;
}

QString QOfonoExtModemManager::mmsSim() const
{
    return iPrivate->iMmsSim;
}

QString QOfonoExtModemManager::mmsModem() const
{
    return iPrivate->iMmsModem;
}

bool QOfonoExtModemManager::ready() const
{
    return iPrivate->iReady;
}

int QOfonoExtModemManager::presentSimCount() const
{
    return iPrivate->iPresentSimCount;
}

int QOfonoExtModemManager::activeSimCount() const
{
    return iPrivate->iActiveSimCount;
}

int QOfonoExtModemManager::errorCount() const
{
    return iPrivate->iErrorCount;
}

QString QOfonoExtModemManager::imeiAt(int aIndex) const
{
    if (aIndex >= 0 && aIndex < iPrivate->iIMEIs.count()) {
        return iPrivate->iIMEIs.at(aIndex);
    } else {
        return QString();
    }
}

QString QOfonoExtModemManager::imeisvAt(int aIndex) const
{
    if (aIndex >= 0 && aIndex < iPrivate->iIMEISVs.count()) {
        return iPrivate->iIMEISVs.at(aIndex);
    } else {
        return QString();
    }
}

bool QOfonoExtModemManager::simPresentAt(int aIndex) const
{
    if (aIndex >= 0 && aIndex < iPrivate->iPresentSims.count()) {
        return iPrivate->iPresentSims.at(aIndex);
    } else {
        return false;
    }
}

void QOfonoExtModemManager::setEnabledModems(QStringList aModems)
{
    if (iPrivate->iProxy) {
        iPrivate->iProxy->SetEnabledModems(Private::toPathList(aModems));
    }
    // Optimistically cache the changes
    if (iPrivate->iEnabledModems != aModems) {
        iPrivate->iEnabledModems = aModems;
        Q_EMIT enabledModemsChanged(aModems);
    }
}

void QOfonoExtModemManager::setDefaultDataSim(QString aImsi)
{
    if (iPrivate->iProxy) {
        iPrivate->iProxy->SetDefaultDataSim(aImsi);
    }
    // Optimistically cache the changes
    if (iPrivate->iDefaultDataSim != aImsi) {
        iPrivate->iDefaultDataSim = aImsi;
        Q_EMIT defaultDataSimChanged(aImsi);
    }
}

void QOfonoExtModemManager::setDefaultVoiceSim(QString aImsi)
{
    if (iPrivate->iProxy) {
        iPrivate->iProxy->SetDefaultVoiceSim(aImsi);
    }
    // Optimistically cache the changes
    if (iPrivate->iDefaultVoiceSim != aImsi) {
        iPrivate->iDefaultVoiceSim = aImsi;
        Q_EMIT defaultVoiceSimChanged(aImsi);
    }
}

QSharedPointer<QOfonoExtModemManager> QOfonoExtModemManager::instance()
{
    QSharedPointer<QOfonoExtModemManager> instance = Private::sSharedInstance;
    if (instance.isNull()) {
        instance = QSharedPointer<QOfonoExtModemManager>::create();
        Private::sSharedInstance = instance;
    }
    return instance;
}

#include "qofonoextmodemmanager.moc"
