/****************************************************************************
**
** Copyright (C) 2015-2023 Slava Monich <slava@monich.com>
** Copyright (C) 2015-2021 Jolla Ltd.
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

#include "qofonoextcell.h"
#include "qofonoextcellinfo.h"
#include "qofonoext_p.h"

namespace {
    const QString kTypeGsm("gsm");
    const QString kTypeWcdma("wcdma");
    const QString kTypeLte("lte");
    const QString kTypeNr("nr");

    const QString kMethodGetAll("GetAll");
}

#define QOFONOEXT_INVALID_VALUE   ((int)QOfonoExtCell::InvalidValue)
#define QOFONOEXT_INVALID_VALUE64 ((qint64)QOfonoExtCell::InvalidValue64)

#ifdef INT64_MAX
Q_STATIC_ASSERT(QOFONOEXT_INVALID_VALUE64 == INT64_MAX);
#endif

#define CELL_PROPERTIES(p) \
    p(mcc) p(mnc) p(signalStrength) p(lac) p(cid) p(arfcn) p(bsic) \
    p(bitErrorRate) p(psc) p(uarfcn) p(ci) p(pci) p(tac) p(earfcn) p(rsrp) \
    p(rsrq) p(rssnr) p(cqi) p(timingAdvance) p(nrarfcn) \
    p(ssRsrp) p(ssRsrq) p(ssSinr) p(csiRsrp) p(csiRsrq) p(csiSinr)

#define CELL_PROPERTIES_64(p) \
    p(nci)

// ==========================================================================
// QOfonoExtCellProxy
//
// qdbusxml2cpp doesn't really do much, and has a number of limitations,
// such as the limits on number of arguments for QDBusPendingReply template.
// It's easier to write these proxies by hand.
// ==========================================================================

class QOfonoExtCellProxy: public QDBusAbstractInterface
{
    Q_OBJECT

public:
    QOfonoExtCellProxy(const QString &aPath, QObject *aParent)
        : QDBusAbstractInterface(OFONO_SERVICE, aPath,
                                 "org.nemomobile.ofono.Cell", OFONO_BUS, aParent) {}

public Q_SLOTS: // METHODS
    QDBusPendingCall GetAllAsync()
        { return asyncCall(kMethodGetAll); }
    QDBusMessage GetAllSync()
        { return call(kMethodGetAll); }

Q_SIGNALS: // SIGNALS
    void PropertyChanged(QString aName, QDBusVariant aValue);
    void RegisteredChanged(bool aRegistered);
    void Removed();
};

// ==========================================================================
// QOfonoExtCell::Private
// ==========================================================================

class QOfonoExtCell::Private : public QOfonoExtCellProxy
{
    Q_OBJECT

public:
    enum Property {
        PropertyUnknown = -1,
        #define Property_(x) Property_##x,
        CELL_PROPERTIES(Property_)
        #define Property64_(x) Property64_##x,
        CELL_PROPERTIES_64(Property64_)
        PropertyCount
    };

    struct PropertyDesc {
        QString name;
        void (QOfonoExtCell::*signal)();
        void (*propertyChanged)(QOfonoExtCell*, QString, qint64);
    };

    static const PropertyDesc Properties[PropertyCount];

    typedef QDBusPendingReply <
        int,          // 0. version
        QString,      // 1. type
        bool,         // 2. registered
        QVariantMap>  // 3. properties
        GetAllReply;

    Private(QString aPath, QOfonoExtCell* aParent);
    void getAllSyncInit();

    static int valueInt(Private* aThis, Property aProperty);
    static qint64 valueInt64(Private* aThis, Property aProperty);
    static Type typeFromString(QString aType);
    static Property propertyFromString(QString aProperty);
    static int getRssiDbm(int aSignalStrength);
    static int inRange(int aValue, int aRangeMin, int aRangeMax);

private:
    QOfonoExtCell* cell();
    void getAllAsync();
    bool pathValid();
    bool updateSignalLevelDbm();
    void handleGetAllReply(GetAllReply aReply, bool aEmitSignals);
    void invalidateValues();
    static void propertyChanged32(QOfonoExtCell* aCell, QString aName, qint64 aValue);
    static void propertyChanged64(QOfonoExtCell* aCell, QString aName, qint64 aValue);

public Q_SLOTS:
    void updateAllAsync();

private Q_SLOTS:
    void onGetAllFinished(QDBusPendingCallWatcher* aWatcher);
    void onPropertyChanged(QString aName, QDBusVariant aValue);
    void onRegisteredChanged(bool aRegistered);

public:
    bool iValid;
    bool iRegistered;
    qint64 iProperties[PropertyCount];
    int iSignalLevelDbm;
    QOfonoExtCell::Type iType;

private:
    QDBusPendingCallWatcher* iPendingGetAll;
    QSharedPointer<QOfonoExtCellInfo> iCellInfo;
};

void QOfonoExtCell::Private::propertyChanged32(QOfonoExtCell* aCell, QString aName, qint64 aValue)
{
    Q_EMIT aCell->propertyChanged(aName, (int)aValue);
}

void QOfonoExtCell::Private::propertyChanged64(QOfonoExtCell* aCell, QString aName, qint64 aValue)
{
    Q_EMIT aCell->propertyChanged64(aName, aValue);
}

const QOfonoExtCell::Private::PropertyDesc QOfonoExtCell::Private::Properties[] = {
    #define PropertyDesc_(x) {QString(#x), &QOfonoExtCell::x##Changed, propertyChanged32},
    CELL_PROPERTIES(PropertyDesc_)
    #define PropertyDesc64_(x) {QString(#x), &QOfonoExtCell::x##Changed, propertyChanged64},
    CELL_PROPERTIES_64(PropertyDesc_)
};

QOfonoExtCell::Private::Private(QString aPath, QOfonoExtCell* aParent) :
    QOfonoExtCellProxy(aPath, aParent),
    iValid(false),
    iRegistered(false),
    iSignalLevelDbm(QOFONOEXT_INVALID_VALUE),
    iType(UNKNOWN),
    iPendingGetAll(Q_NULLPTR)
{
    // Extract modem path from the cell path, e.g. "/ril_0/cell_0" => "/ril_0"
    iCellInfo = QOfonoExtCellInfo::instance(aPath.left(aPath.lastIndexOf('/')));
    invalidateValues();
    connect(this, SIGNAL(Removed()),
        aParent, SIGNAL(removed()));
    connect(this,
        SIGNAL(PropertyChanged(QString,QDBusVariant)),
        SLOT(onPropertyChanged(QString,QDBusVariant)));
    connect(this,
        SIGNAL(RegisteredChanged(bool)),
        SLOT(onRegisteredChanged(bool)));
    connect(iCellInfo.data(),
        SIGNAL(cellsChanged()),
        SLOT(updateAllAsync()));
    connect(iCellInfo.data(),
        SIGNAL(validChanged()),
        SLOT(updateAllAsync()));
}

inline QOfonoExtCell* QOfonoExtCell::Private::cell()
{
    return qobject_cast<QOfonoExtCell*>(parent());
}

QOfonoExtCell::Type QOfonoExtCell::Private::typeFromString(QString aType)
{
    return (aType == kTypeGsm) ? GSM :
           (aType == kTypeLte) ? LTE :
           (aType == kTypeWcdma) ? WCDMA :
           (aType == kTypeNr) ? NR :
           UNKNOWN;
}

QOfonoExtCell::Private::Property QOfonoExtCell::Private::propertyFromString(QString aProperty)
{
    for (int i=PropertyUnknown+1; i<PropertyCount; i++) {
        if (Properties[i].name == aProperty) {
            return (Property)i;
        }
    }
    return PropertyUnknown;
}

int QOfonoExtCell::Private::valueInt(Private* aThis, QOfonoExtCell::Private::Property aProperty)
{
    return aThis ? aThis->iProperties[aProperty] : QOFONOEXT_INVALID_VALUE;
}

qint64 QOfonoExtCell::Private::valueInt64(Private* aThis, QOfonoExtCell::Private::Property aProperty)
{
    return aThis ? aThis->iProperties[aProperty] : QOFONOEXT_INVALID_VALUE64;
}

bool QOfonoExtCell::Private::pathValid()
{
    return iCellInfo->valid() && iCellInfo->cells().contains(path());
}

void QOfonoExtCell::Private::updateAllAsync()
{
    if (pathValid()) {
        if (!iValid && !iPendingGetAll) {
            getAllAsync();
        }
    } else {
        delete iPendingGetAll;
        iPendingGetAll = Q_NULLPTR;

        if (iValid) {
            iValid = false;
            Q_EMIT cell()->validChanged();
        }
    }
}

int QOfonoExtCell::Private::getRssiDbm(int aValue)
{
    // Range for RSSI in ASU (0-31, 99) as defined in TS 27.007 8.69
    return (aValue < 0 || aValue > 31) ? QOFONOEXT_INVALID_VALUE : (-113 + (2 * aValue));
}

int QOfonoExtCell::Private::inRange(int aValue, int aMin, int aMax)
{
    return (aValue < aMin || aValue > aMax) ? QOFONOEXT_INVALID_VALUE : aValue;
}

void QOfonoExtCell::Private::getAllSyncInit()
{
    delete iPendingGetAll;
    iPendingGetAll = NULL;

    GetAllReply reply(GetAllSync());
    if (!reply.isError()) {
        handleGetAllReply(reply, false);
    }
}

void QOfonoExtCell::Private::getAllAsync()
{
    delete iPendingGetAll;
    iPendingGetAll = new QDBusPendingCallWatcher(GetAllAsync(), this);
    connect(iPendingGetAll,
        SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onGetAllFinished(QDBusPendingCallWatcher*)));
}

void QOfonoExtCell::Private::onGetAllFinished(QDBusPendingCallWatcher* aWatcher)
{
    iPendingGetAll = Q_NULLPTR;
    if (aWatcher->isError()) {
        // Repeat the call on timeout
        QDBusError error(aWatcher->error());
        qWarning() << error;
        if (QOfonoExt::isTimeout(error)) {
            getAllAsync();
        }
    } else {
        handleGetAllReply(*aWatcher, true);
    }
    aWatcher->deleteLater();
}

void QOfonoExtCell::Private::handleGetAllReply(GetAllReply aReply, bool aEmitSignals)
{
    const Type prevType = iType;
    const bool wasRegistered = iRegistered;
    const int prevSignalLevelDbm = iSignalLevelDbm;

    // Ignore argumentAt<0> version
    iType = typeFromString(aReply.argumentAt<1>());
    iRegistered = aReply.argumentAt<2>();
    const QVariantMap variants(aReply.argumentAt<3>());

    // Unpack properties (they are all integers)
    qint64 prevProps[PropertyCount];
    memcpy(prevProps, iProperties, sizeof(iProperties));

    int i;
    invalidateValues();
    for (QVariantMap::ConstIterator it = variants.constBegin();
         it != variants.constEnd(); it++) {
        const QString key(it.key());
        const QVariant value(it.value());
        bool ok = false;
        qint64 intValue = value.toLongLong(&ok);
        if (ok) {
            Property p = propertyFromString(key);
            if (p != PropertyUnknown) {
                iProperties[p] = intValue;
            }
        }
    }

    // This one is a combination of other properties, updated separately
    updateSignalLevelDbm();

    // Emit signals
    if (aEmitSignals) {
        QOfonoExtCell* parent = cell();
        for (i=0; i<PropertyCount; i++) {
            if (iProperties[i] != prevProps[i]) {
                (parent->*(Properties[i].signal))();
                Properties[i].propertyChanged(parent, Properties[i].name, iProperties[i]);
            }
        }

        iValid = true;
        if (prevType != iType) {
            Q_EMIT parent->typeChanged();
        }
        if (wasRegistered != iRegistered) {
            Q_EMIT parent->registeredChanged();
        }
        if (prevSignalLevelDbm != iSignalLevelDbm) {
            Q_EMIT parent->signalLevelDbmChanged();
        }
        Q_EMIT parent->signalLevelDbmChanged();
        Q_EMIT parent->validChanged();
    }
}

void QOfonoExtCell::Private::invalidateValues()
{
    #define _Init(x) iProperties[Property_##x] = QOFONOEXT_INVALID_VALUE;
    CELL_PROPERTIES(_Init)
    #define _Init64(x) iProperties[Property64_##x] = QOFONOEXT_INVALID_VALUE64;
    CELL_PROPERTIES_64(_Init64)
}

void QOfonoExtCell::Private::onPropertyChanged(QString aName, QDBusVariant aValue)
{
    bool ok = false;
    qint64 intValue = aValue.variant().toLongLong(&ok);
    if (ok) {
        Property p = propertyFromString(aName);
        if (p != PropertyUnknown && iProperties[p] != intValue) {
            QOfonoExtCell* parent = cell();
            iProperties[p] = intValue;
            Q_EMIT (parent->*(Properties[p].signal))();
            Q_EMIT parent->propertyChanged(aName, intValue);
            switch (p) {
            case Property_signalStrength:
            case Property_rsrp:
                if (updateSignalLevelDbm()) {
                    Q_EMIT parent->signalLevelDbmChanged();
                }
                break;
            case Property_ssRsrp:
                if (updateSignalLevelDbm()) {
                    Q_EMIT parent->signalLevelDbmChanged();
                }
                break;
            default:
                break;
            }
        }
    }
}

void QOfonoExtCell::Private::onRegisteredChanged(bool aRegistered)
{
    iRegistered = aRegistered;
    Q_EMIT cell()->registeredChanged();
}

bool QOfonoExtCell::Private::updateSignalLevelDbm()
{
    int signalLevelDbm = QOFONOEXT_INVALID_VALUE;

    switch (iType) {
    case NR:
        // Return SS-RSRP value. Reference: 3GPP TS 36.133, sub-clause 9.11
        signalLevelDbm = inRange(-iProperties[Property_ssRsrp], -140, -44);
        break;
    case LTE:
        // Return RSRP value. Reference: 3GPP TS 36.133, sub-clause 9.1.4
        signalLevelDbm = inRange(-iProperties[Property_rsrp], -140, -44);
        break;
    case WCDMA:
    case GSM:
        // Return RSSI. Reference: TS 27.007 sub clause 8.5
        signalLevelDbm = getRssiDbm(iProperties[Property_signalStrength]);
        break;
    case UNKNOWN:
        break;
    }

    if (iSignalLevelDbm != signalLevelDbm) {
        iSignalLevelDbm = signalLevelDbm;
        return true;
    }
    return false;
}

// ==========================================================================
// QOfonoExtCell
// ==========================================================================

QOfonoExtCell::QOfonoExtCell(QObject* aParent) :
    QObject(aParent),
    iPrivate(Q_NULLPTR)
{
}

QOfonoExtCell::QOfonoExtCell(QString aPath) :
    iPrivate(new Private(aPath, this))
{
    iPrivate->updateAllAsync();
}

QOfonoExtCell::QOfonoExtCell(QString aPath, bool aMayBlock) : // Since 1.0.27
    iPrivate(new Private(aPath, this))
{
    if (aMayBlock) {
        iPrivate->getAllSyncInit();
    } else {
        iPrivate->updateAllAsync();
    }
}

QOfonoExtCell::~QOfonoExtCell()
{
}

bool QOfonoExtCell::valid() const
{
    return iPrivate && iPrivate->iValid;
}

QOfonoExtCell::Type QOfonoExtCell::type() const
{
    return iPrivate ? iPrivate->iType : UNKNOWN;
}

bool QOfonoExtCell::registered() const
{
    return iPrivate && iPrivate->iRegistered;
}

QString QOfonoExtCell::path() const
{
    return iPrivate ? iPrivate->path() : QString();
}

void QOfonoExtCell::setPath(QString aPath)
{
    if (path() != aPath) {
        const bool wasValid = valid();
        const bool wasRegistered = registered();
        const Type prevType = type();
        delete iPrivate;
        iPrivate = new Private(aPath, this);
        iPrivate->updateAllAsync();
        if (valid() != wasValid) {
            Q_EMIT validChanged();
        }
        if (registered() != wasRegistered) {
            Q_EMIT registeredChanged();
        }
        if (type() != prevType) {
            Q_EMIT typeChanged();
        }
        Q_EMIT pathChanged();
    }
}

int QOfonoExtCell::signalLevelDbm() const
{
    return iPrivate ? iPrivate->iSignalLevelDbm : QOFONOEXT_INVALID_VALUE;
}

#define PropertyGet_(x) \
    int QOfonoExtCell::x() const {\
        return Private::valueInt(iPrivate, Private::Property_##x); \
    }
CELL_PROPERTIES(PropertyGet_)
#define PropertyGet64_(x) \
    qint64 QOfonoExtCell::x() const {\
        return Private::valueInt64(iPrivate, Private::Property64_##x); \
    }
CELL_PROPERTIES_64(PropertyGet64_)

#include "qofonoextcell.moc"
