/****************************************************************************
**
+* Copyright (C) 2026 Jolla Mobile Ltd
** Copyright (C) 2015-2022 Jolla Ltd.
** Copyright (C) 2015-2022 Slava Monich <slava.monich@jolla.com>
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

class QOfonoExtModemManagerProxy:
    public QDBusAbstractInterface
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
    QDBusPendingCall SetDefaultDataSim(const QString& aImsi)
        { return asyncCall("SetDefaultDataSim", aImsi); }
    QDBusPendingCall SetDefaultVoiceSim(const QString& aImsi)
        { return asyncCall("SetDefaultVoiceSim", aImsi); }
    QDBusPendingCall SetEnabledModems(const QList<QDBusObjectPath>& aModems)
        { return asyncCall("SetEnabledModems", QVariant::fromValue(aModems)); }

Q_SIGNALS: // SIGNALS
    void DefaultDataModemChanged(QString);
    void DefaultDataSimChanged(QString);
    void DefaultVoiceModemChanged(QString);
    void DefaultVoiceSimChanged(QString);
    void EnabledModemsChanged(QList<QDBusObjectPath>);
    void MmsModemChanged(QString);
    void MmsSimChanged(QString);
    void PresentSimsChanged(int, bool);
    void ReadyChanged(bool);
    void ModemError(QDBusObjectPath, QString, QString);

public:
    // Becomes non-zero after GetInterfaceVersion succeeds:
    int iInterfaceVersion;
};

Q_DECLARE_METATYPE(QOfonoExtModemManagerProxy::Error)

QDBusArgument&
operator<<(
    QDBusArgument& aArg,
    const QOfonoExtModemManagerProxy::Error& aValue)
{
    aArg.beginStructure();
    aArg << aValue.iName;
    aArg << aValue.iCount;
    aArg.endStructure();
    return aArg;
}

const QDBusArgument&
operator>>(
    const QDBusArgument& aArg,
    QOfonoExtModemManagerProxy::Error& aValue)
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

class QOfonoExtModemManager::Private :
    public QObject
{
    Q_OBJECT
    typedef QList<QOfonoExtModemManagerProxy::Error> ErrorList;
    typedef QList<ErrorList> ModemErrors;

public:
    static QWeakPointer<QOfonoExtModemManager> sSharedInstance;

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

    Private(QOfonoExtModemManager*);

    static QStringList toStringList(const QList<QDBusObjectPath>&);
    static QList<QDBusObjectPath> toPathList(const QStringList&);

    QOfonoExtModemManager* parentObject() const;
    QStringList dummyStringList();

    void getAll();
    void getInterfaceVersion();
    void presentSimsChanged(const QList<bool>&);
    void updateSimCounts();
    void updateEnabledModems(const QStringList&);
    void updateDefaultDataModem(const QString&);
    void updateDefaultVoiceModem(const QString&);
    void updateDefaultDataSim(const QString&);
    void updateDefaultVoiceSim(const QString&);
    void updateMmsSim(const QString&);
    void updateMmsModem(const QString&);
    void updateReady(bool);

private: // SLOTS
    void onServiceRegistered();
    void onServiceUnregistered();
    void onGetInterfaceVersionFinished(QDBusPendingCallWatcher*);
    void onGetAllFinished(QDBusPendingCallWatcher*);
    void onEnabledModemsChanged(const QList<QDBusObjectPath>&);
    void onDefaultVoiceModemChanged(const QString&);
    void onDefaultDataModemChanged(const QString&);
    void onDefaultVoiceSimChanged(const QString&);
    void onDefaultDataSimChanged(const QString&);
    void onPresentSimsChanged(int, bool);
    void onMmsSimChanged(const QString&);
    void onMmsModemChanged(const QString&);
    void onReadyChanged(bool);
    void onModemError(const QDBusObjectPath&, const QString&, const QString&);
};

/* static */
QWeakPointer<QOfonoExtModemManager> QOfonoExtModemManager::Private::sSharedInstance;

/* static */
QStringList
QOfonoExtModemManager::Private::toStringList(
    const QList<QDBusObjectPath>& aList)
{
    QStringList stringList;
    const int n = aList.count();

    stringList.reserve(n);
    for (int i = 0; i < n; i++) {
        stringList.append(aList.at(i).path());
    }
    return stringList;
}

/* static */
QList<QDBusObjectPath>
QOfonoExtModemManager::Private::toPathList(
    const QStringList& aList)
{
    QList<QDBusObjectPath> pathList;
    const int n = aList.count();

    pathList.reserve(n);
    for (int i = 0; i < n; i++) {
        pathList.append(QDBusObjectPath(aList.at(i)));
    }
    return pathList;
}

QOfonoExtModemManager::Private::Private(
    QOfonoExtModemManager* aParent) :
    QObject(aParent),
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

    connect(ofonoWatcher, &QDBusServiceWatcher::serviceRegistered,
        this, &Private::onServiceRegistered);
    connect(ofonoWatcher, &QDBusServiceWatcher::serviceUnregistered,
        this, &Private::onServiceUnregistered);

    if (OFONO_BUS.interface()->isServiceRegistered(OFONO_SERVICE)) {
        onServiceRegistered();
    }
}

QOfonoExtModemManager*
QOfonoExtModemManager::Private::parentObject() const
{
    return qobject_cast<QOfonoExtModemManager*>(parent());
}

void
QOfonoExtModemManager::Private::onServiceRegistered()
{
    const bool wasValid = iValid;

    if (!iProxy) {
        iProxy = new QOfonoExtModemManagerProxy(this);
        if (iProxy->isValid()) {
            iValid = false;
            connect(iProxy, &QOfonoExtModemManagerProxy::EnabledModemsChanged,
                this, &Private::onEnabledModemsChanged);
            connect(iProxy, &QOfonoExtModemManagerProxy::DefaultDataModemChanged,
                this, &Private::onDefaultDataModemChanged);
            connect(iProxy, &QOfonoExtModemManagerProxy::DefaultVoiceModemChanged,
                this, &Private::onDefaultVoiceModemChanged);
            connect(iProxy, &QOfonoExtModemManagerProxy::DefaultDataSimChanged,
                this, &Private::onDefaultDataSimChanged);
            connect(iProxy, &QOfonoExtModemManagerProxy::DefaultVoiceSimChanged,
                this, &Private::onDefaultVoiceSimChanged);
            connect(iProxy, &QOfonoExtModemManagerProxy::PresentSimsChanged,
                this, &Private::onPresentSimsChanged);
            getInterfaceVersion();
        } else {
            delete iProxy;
            iProxy = NULL;
        }
    }
    if (wasValid != iValid) {
        Q_EMIT parentObject()->validChanged(iValid);
    }
}

void
QOfonoExtModemManager::Private::onServiceUnregistered()
{
    if (iProxy) {
        // iProxy is the parent of iInitCall
        iInitCall = NULL;
        delete iProxy;
        iProxy = NULL;
    }
    if (iValid) {
        iValid = false;
        Q_EMIT parentObject()->validChanged(iValid);
    }
}

void
QOfonoExtModemManager::Private::getInterfaceVersion()
{
    iInitCall = new QDBusPendingCallWatcher(iProxy->GetInterfaceVersion(), iProxy);
    connect(iInitCall, &QDBusPendingCallWatcher::finished,
        this, &Private::onGetInterfaceVersionFinished);
}

void
QOfonoExtModemManager::Private::getAll()
{
    iInitCall = new QDBusPendingCallWatcher(
        (iInterfaceVersion == 2) ? QDBusPendingCall(iProxy->GetAll2()) :
        (iInterfaceVersion == 3) ? QDBusPendingCall(iProxy->GetAll3()) :
        (iInterfaceVersion == 4) ? QDBusPendingCall(iProxy->GetAll4()) :
        (iInterfaceVersion == 5) ? QDBusPendingCall(iProxy->GetAll5()) :
        (iInterfaceVersion == 6) ? QDBusPendingCall(iProxy->GetAll6()) :
        (iInterfaceVersion == 7) ? QDBusPendingCall(iProxy->GetAll7()) :
        QDBusPendingCall(iProxy->GetAll8()), iProxy);
    connect(iInitCall, &QDBusPendingCallWatcher::finished,
        this, &Private::onGetAllFinished);
}

void
QOfonoExtModemManager::Private::onGetInterfaceVersionFinished(
    QDBusPendingCallWatcher* aWatcher)
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
                connect(iProxy, &QOfonoExtModemManagerProxy::MmsSimChanged,
                    this, &Private::onMmsSimChanged);
                connect(iProxy, &QOfonoExtModemManagerProxy::MmsModemChanged,
                    this, &Private::onMmsModemChanged);
            }
            if (version >= 5 && iProxy->iInterfaceVersion < 5) {
                connect(iProxy, &QOfonoExtModemManagerProxy::ReadyChanged,
                    this, &Private::onReadyChanged);
            }
            if (version >= 6 && iProxy->iInterfaceVersion < 6) {
                connect(iProxy, &QOfonoExtModemManagerProxy::ModemError,
                    this, &Private::onModemError);
            }
            iProxy->iInterfaceVersion = version;
        }
        if (iInterfaceVersion != version) {
            iInterfaceVersion = version;
            Q_EMIT parentObject()->interfaceVersionChanged(version);
        }
        getAll();
    }
    aWatcher->deleteLater();
}

void
QOfonoExtModemManager::Private::onGetAllFinished(
    QDBusPendingCallWatcher* aWatcher)
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
        QStringList list(toStringList(reply.argumentAt<1>()));
        QOfonoExtModemManager* obj = parentObject();

        if (iAvailableModems != list) {
            iAvailableModems = list;
            Q_EMIT obj->availableModemsChanged(iAvailableModems);
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
            Q_EMIT obj->imeiCodesChanged(iIMEIs);
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

            for (int i = 0; i < n; i++) {
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

            for (int i = 0; i < k; i++) {
                errorCount += errors.at(i).iCount;
            }
        }

        if (iErrorCount != errorCount) {
            iErrorCount = errorCount;
            Q_EMIT obj->errorCountChanged(errorCount);
        }

        if (iIMEISVs != list) {
            iIMEISVs = list;
            Q_EMIT obj->imeisvCodesChanged(iIMEISVs);
        }

        if (!iValid) {
            iValid = true;
            Q_EMIT obj->validChanged(iValid);
        }
    }
    aWatcher->deleteLater();
}

QStringList
QOfonoExtModemManager::Private::dummyStringList()
{
    QStringList list;
    const int n = iAvailableModems.count();

    list.reserve(n);
    for (int i = 0; i < n; i++) {
        list.append(QString());
    }
    return list;
}

void
QOfonoExtModemManager::Private::presentSimsChanged(
    const QList<bool>& aOldList)
{
    QOfonoExtModemManager* obj = parentObject();
    int i;
    const int n = iPresentSims.count();
    QList<bool> changed;

    changed.reserve(n);
    for (i = 0; i < n; i++) {
        const bool prev = (i<aOldList.count() && aOldList.at(i));
        changed.append(iPresentSims.at(i) != prev);
    }
    updateSimCounts();
    for (i = 0; i < n; i++) {
        if (changed.at(i)) {
            Q_EMIT obj->presentSimChanged(i, iPresentSims.at(i));
        }
    }
    if (aOldList != iPresentSims) {
        Q_EMIT obj->presentSimsChanged(iPresentSims);
    }
}

void
QOfonoExtModemManager::Private::updateSimCounts()
{
    QOfonoExtModemManager* obj = parentObject();
    const int oldPresentSimCount = iPresentSimCount;
    const int oldActiveSimCount = iActiveSimCount;

    iPresentSimCount = 0;
    iActiveSimCount = 0;

    const int n = iPresentSims.count();
    for (int i = 0; i < n; i++) {
        if (iPresentSims.at(i)) {
            iPresentSimCount++;
            if (i < iAvailableModems.count() &&
                iEnabledModems.contains(iAvailableModems.at(i))) {
                iActiveSimCount++;
            }
        }
    }
    if (oldPresentSimCount != iPresentSimCount) {
        Q_EMIT obj->presentSimCountChanged(iPresentSimCount);
    }
    if (oldActiveSimCount != iActiveSimCount) {
        Q_EMIT obj->activeSimCountChanged(iActiveSimCount);
    }
}

void
QOfonoExtModemManager::Private::updateEnabledModems(
    const QStringList& aModems)
{
    if (iEnabledModems != aModems) {
        iEnabledModems = aModems;
        Q_EMIT parentObject()->enabledModemsChanged(aModems);
    }
    updateSimCounts();
}

void
QOfonoExtModemManager::Private::updateDefaultDataModem(
    const QString& aPath)
{
    if (iDefaultDataModem != aPath) {
        iDefaultDataModem = aPath;
        Q_EMIT parentObject()->defaultDataModemChanged(aPath);
    }
}

void
QOfonoExtModemManager::Private::updateDefaultVoiceModem(
    const QString& aPath)
{
    if (iDefaultVoiceModem != aPath) {
        iDefaultVoiceModem = aPath;
        Q_EMIT parentObject()->defaultVoiceModemChanged(aPath);
    }
}

void
QOfonoExtModemManager::Private::updateDefaultDataSim(
    const QString& aImsi)
{
    if (iDefaultDataSim != aImsi) {
        iDefaultDataSim = aImsi;
        Q_EMIT parentObject()->defaultDataSimChanged(aImsi);
    }
}

void
QOfonoExtModemManager::Private::updateDefaultVoiceSim(
    const QString& aImsi)
{
    if (iDefaultVoiceSim != aImsi) {
        iDefaultVoiceSim = aImsi;
        Q_EMIT parentObject()->defaultVoiceSimChanged(aImsi);
    }
}

void
QOfonoExtModemManager::Private::updateMmsSim(
    const QString& aImsi)
{
    if (iMmsSim != aImsi) {
        iMmsSim = aImsi;
        Q_EMIT parentObject()->mmsSimChanged(aImsi);
    }
}

void
QOfonoExtModemManager::Private::updateMmsModem(
    const QString& aPath)
{
    if (iMmsModem != aPath) {
        iMmsModem = aPath;
        Q_EMIT parentObject()->mmsModemChanged(aPath);
    }
}

void
QOfonoExtModemManager::Private::updateReady(
    bool aReady)
{
    if (iReady != aReady) {
        iReady = aReady;
        Q_EMIT parentObject()->readyChanged(aReady);
    }
}

void
QOfonoExtModemManager::Private::onEnabledModemsChanged(
    const QList<QDBusObjectPath>& aModems)
{
    if (!iInitCall) {
        updateEnabledModems(toStringList(aModems));
    }
}

void
QOfonoExtModemManager::Private::onDefaultDataModemChanged(
    const QString& aPath)
{
    if (!iInitCall) {
        updateDefaultDataModem(aPath);
    }
}

void
QOfonoExtModemManager::Private::onDefaultVoiceModemChanged(
    const QString& aPath)
{
    if (!iInitCall) {
        updateDefaultVoiceModem(aPath);
    }
}

void
QOfonoExtModemManager::Private::onDefaultDataSimChanged(
     const QString& aImsi)
{
    if (!iInitCall) {
        updateDefaultDataSim(aImsi);
    }
}

void
QOfonoExtModemManager::Private::onDefaultVoiceSimChanged(
    const QString& aImsi)
{
    if (!iInitCall) {
        updateDefaultVoiceSim(aImsi);
    }
}

void
QOfonoExtModemManager::Private::onPresentSimsChanged(
    int aIndex,
    bool aPresent)
{
    if (!iInitCall && aIndex >= 0 && aIndex < iPresentSims.count()) {
        const QList<bool> oldList(iPresentSims);

        iPresentSims[aIndex] = aPresent;
        presentSimsChanged(oldList);
    }
}

void
QOfonoExtModemManager::Private::onMmsSimChanged(
    const QString& aImsi)
{
    if (!iInitCall) {
        updateMmsSim(aImsi);
    }
}

void
QOfonoExtModemManager::Private::onMmsModemChanged(
    const QString& aPath)
{
    if (!iInitCall) {
        updateMmsModem(aPath);
    }
}

void
QOfonoExtModemManager::Private::onReadyChanged(
    bool aReady)
{
    if (!iInitCall) {
        updateReady(aReady);
    }
}

void
QOfonoExtModemManager::Private::onModemError(
    const QDBusObjectPath& aPath,
    const QString& aName,
    const QString& aMessage)
{
    if (!iInitCall) {
        QOfonoExtModemManager* obj = parentObject();

        iErrorCount++;
        Q_EMIT obj->errorCountChanged(iErrorCount);
        Q_EMIT obj->modemError(aPath.path(), aName, aMessage);
    }
}

// ==========================================================================
// QOfonoExtModemManager
// ==========================================================================

QOfonoExtModemManager::QOfonoExtModemManager(
    QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{}

QOfonoExtModemManager::~QOfonoExtModemManager()
{}

bool
QOfonoExtModemManager::valid() const
{
    return iPrivate->iValid;
}

int
QOfonoExtModemManager::interfaceVersion() const
{
    return iPrivate->iInterfaceVersion;
}

QStringList
QOfonoExtModemManager::availableModems() const
{
    return iPrivate->iAvailableModems;
}

QStringList
QOfonoExtModemManager::enabledModems() const
{
    return iPrivate->iEnabledModems;
}

QString
QOfonoExtModemManager::defaultVoiceModem() const
{
    return iPrivate->iDefaultVoiceModem;
}

QString
QOfonoExtModemManager::defaultDataModem() const
{
    return iPrivate->iDefaultDataModem;
}

QString
QOfonoExtModemManager::defaultVoiceSim() const
{
    return iPrivate->iDefaultVoiceSim;
}

QString
QOfonoExtModemManager::defaultDataSim() const
{
    return iPrivate->iDefaultDataSim;
}

QList<bool>
QOfonoExtModemManager::presentSims() const
{
    return iPrivate->iPresentSims;
}

QStringList
QOfonoExtModemManager::imeiCodes() const
{
    return iPrivate->iIMEIs;
}

QStringList
QOfonoExtModemManager::imeisvCodes() const
{
    return iPrivate->iIMEISVs;
}

QString
QOfonoExtModemManager::mmsSim() const
{
    return iPrivate->iMmsSim;
}

QString
QOfonoExtModemManager::mmsModem() const
{
    return iPrivate->iMmsModem;
}

bool
QOfonoExtModemManager::ready() const
{
    return iPrivate->iReady;
}

int
QOfonoExtModemManager::presentSimCount() const
{
    return iPrivate->iPresentSimCount;
}

int
QOfonoExtModemManager::activeSimCount() const
{
    return iPrivate->iActiveSimCount;
}

int
QOfonoExtModemManager::errorCount() const
{
    return iPrivate->iErrorCount;
}

QString
QOfonoExtModemManager::imeiAt(
    int aIndex) const
{
    if (aIndex >= 0 && aIndex < iPrivate->iIMEIs.count()) {
        return iPrivate->iIMEIs.at(aIndex);
    } else {
        return QString();
    }
}

QString
QOfonoExtModemManager::imeisvAt(
    int aIndex) const
{
    if (aIndex >= 0 && aIndex < iPrivate->iIMEISVs.count()) {
        return iPrivate->iIMEISVs.at(aIndex);
    } else {
        return QString();
    }
}

bool
QOfonoExtModemManager::simPresentAt(
    int aIndex) const
{
    if (aIndex >= 0 && aIndex < iPrivate->iPresentSims.count()) {
        return iPrivate->iPresentSims.at(aIndex);
    } else {
        return false;
    }
}

void
QOfonoExtModemManager::setEnabledModems(
    QStringList aModems)
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

void
QOfonoExtModemManager::setDefaultDataSim(
    QString aImsi)
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

void
QOfonoExtModemManager::setDefaultVoiceSim(
    QString aImsi)
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

/* static */
QSharedPointer<QOfonoExtModemManager>
QOfonoExtModemManager::instance()
{
    QSharedPointer<QOfonoExtModemManager> instance = Private::sSharedInstance;

    if (instance.isNull()) {
        instance = QSharedPointer<QOfonoExtModemManager>::create();
        Private::sSharedInstance = instance;
    }
    return instance;
}

#include "qofonoextmodemmanager.moc"
