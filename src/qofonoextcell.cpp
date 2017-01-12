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

#include "qofonoextcell.h"
#include "qofonoextcellinfo.h"
#include "qofonoext_p.h"

static const QString kTypeGsm("gsm");
static const QString kTypeWcdma("wcdma");
static const QString kTypeLte("lte");

#define CELL_PROPERTIES(p) \
    p(mcc) p(mnc) p(signalStrength) p(lac) p(cid) p(bitErrorRate) p(psc) \
    p(ci) p(pci) p(tac) p(rsrp) p(rsrq) p(rssnr) p(cqi) p(timingAdvance)

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
    QOfonoExtCellProxy(QString aPath, QObject* aParent) :
        QDBusAbstractInterface(OFONO_SERVICE, aPath,
            "org.nemomobile.ofono.Cell", OFONO_BUS, aParent) {}

public Q_SLOTS: // METHODS
    QDBusPendingCall GetInterfaceVersion()
        { return asyncCall("GetInterfaceVersion"); }
    QDBusPendingCall GetAll()
        { return asyncCall("GetAll"); }

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
        PropertyCount
    };

    struct PropertyDesc {
        QString name;
        void (QOfonoExtCell::*signal)();
    };

    static const PropertyDesc Properties[PropertyCount];

    Private(QString aPath, QOfonoExtCell* aParent);
    static int value(Private* aThis, Property aProperty);
    static Type typeFromString(QString aType);
    static Property propertyFromString(QString aProperty);

private:
    void getAll();

private Q_SLOTS:
    void onCellsChanged();
    void onGetAllFinished(QDBusPendingCallWatcher* aWatcher);
    void onPropertyChanged(QString aName, QDBusVariant aValue);
    void onRegisteredChanged(bool aRegistered);

public:
    bool iValid;
    bool iFixedPath;
    bool iRegistered;
    int iProperties[PropertyCount];
    QOfonoExtCell::Type iType;

private:
    QOfonoExtCell* iParent;
    QDBusPendingCallWatcher* iPendingGetAll;
    QSharedPointer<QOfonoExtCellInfo> iCellInfo;
};

const QOfonoExtCell::Private::PropertyDesc QOfonoExtCell::Private::Properties[] = {
    #define PropertyDesc_(x) {QString(#x), &QOfonoExtCell::x##Changed},
    CELL_PROPERTIES(PropertyDesc_)
};

QOfonoExtCell::Private::Private(QString aPath, QOfonoExtCell* aParent) :
    QOfonoExtCellProxy(aPath, aParent),
    iValid(false),
    iFixedPath(false),
    iRegistered(false),
    iType(UNKNOWN),
    iParent(aParent),
    iPendingGetAll(NULL)
{
    // Extract modem path from the cell path, e.g. "/ril_0/cell_0" => "/ril_0"
    iCellInfo = QOfonoExtCellInfo::instance(aPath.left(aPath.lastIndexOf('/')));
    memset(iProperties, -1, sizeof(iProperties));
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
        SLOT(onCellsChanged()));
    connect(iCellInfo.data(),
        SIGNAL(validChanged()),
        SLOT(onCellsChanged()));
    onCellsChanged();
}

QOfonoExtCell::Type QOfonoExtCell::Private::typeFromString(QString aType)
{
    return (aType == kTypeGsm) ? GSM :
           (aType == kTypeLte) ? LTE :
           (aType == kTypeWcdma) ? WCDMA :
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

int QOfonoExtCell::Private::value(Private* aThis, QOfonoExtCell::Private::Property aProperty)
{
    return aThis ? aThis->iProperties[aProperty] : -1;
}

void QOfonoExtCell::Private::onCellsChanged()
{
    bool pathValid = iCellInfo->valid() && iCellInfo->cells().contains(path());
    if (pathValid) {
        if (!iValid && !iPendingGetAll) {
            getAll();
        }
    } else {
        if (iPendingGetAll) {
            delete iPendingGetAll;
            iPendingGetAll = NULL;
        }
        if (iValid) {
            iValid = false;
            Q_EMIT iParent->validChanged();
        }
    }
}

void QOfonoExtCell::Private::getAll()
{
    delete iPendingGetAll;
    iPendingGetAll = new QDBusPendingCallWatcher(GetAll(), this);
    connect(iPendingGetAll,
        SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onGetAllFinished(QDBusPendingCallWatcher*)));
}

void QOfonoExtCell::Private::onGetAllFinished(QDBusPendingCallWatcher* aWatcher)
{
    QDBusPendingReply <
        int,          // 0. version
        QString,      // 1. type
        bool,         // 2. registered
        QVariantMap>  // 3. properties
    reply(*aWatcher);
    iPendingGetAll = NULL;
    if (reply.isError()) {
        // Repeat the call on timeout
        qWarning() << reply.error();
        if (QOfonoExt::isTimeout(reply.error())) {
            getAll();
        }
    } else {
        const bool wasRegistered = iRegistered;
        const Type prevType = iType;
        iType = typeFromString(reply.argumentAt<1>());
        iRegistered = reply.argumentAt<2>();

        // Unpack properties (they are all integers)
        int prevProps[PropertyCount];
        memcpy(prevProps, iProperties, sizeof(iProperties));
        memset(iProperties, -1, sizeof(iProperties));
        QVariantMap variants = reply.argumentAt<3>();
        QStringList keys = variants.keys();
        for (int i=0; i<keys.count(); i++) {
            QString key = keys.at(i);
            QVariant value = variants.value(key);
            bool ok = false;
            int intValue = value.toInt(&ok);
            if (ok) {
                Property p = propertyFromString(key);
                if (p != PropertyUnknown) {
                    iProperties[p] = intValue;
                }
            }
        }

        // Emit signals
        for (int i=0; i<PropertyCount; i++) {
            if (iProperties[i] != prevProps[i]) {
                (iParent->*(Properties[i].signal))();
                Q_EMIT iParent->propertyChanged(Properties[i].name, iProperties[i]);
            }
        }

        iValid = true;
        if (prevType != iType) {
            Q_EMIT iParent->typeChanged();
        }
        if (wasRegistered != iRegistered) {
            Q_EMIT iParent->registeredChanged();
        }
        Q_EMIT iParent->validChanged();
    }
    aWatcher->deleteLater();
}

void QOfonoExtCell::Private::onPropertyChanged(QString aName, QDBusVariant aValue)
{
    bool ok = false;
    int intValue = aValue.variant().toInt(&ok);
    if (ok) {
        Property p = propertyFromString(aName);
        if (p != PropertyUnknown && iProperties[p] != intValue) {
            iProperties[p] = intValue;
            Q_EMIT (iParent->*(Properties[p].signal))();
            Q_EMIT iParent->propertyChanged(aName, intValue);
        }
    }
}

void QOfonoExtCell::Private::onRegisteredChanged(bool aRegistered)
{
    iRegistered = aRegistered;
    Q_EMIT iParent->registeredChanged();
}

// ==========================================================================
// QOfonoExtCell
// ==========================================================================

QOfonoExtCell::QOfonoExtCell(QObject* aParent) :
    QObject(aParent),
    iPrivate(NULL)
{
}

QOfonoExtCell::QOfonoExtCell(QString aPath) :
    iPrivate(NULL)
{
    setPath(aPath);
    if (iPrivate) {
        iPrivate->iFixedPath = true;
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
        if (iPrivate && iPrivate->iFixedPath) {
            qWarning() << "Attempting to change fixed path" << path();
        } else {
            const bool wasValid = valid();
            const bool wasRegistered = registered();
            const Type prevType = type();
            delete iPrivate;
            iPrivate = new Private(aPath, this);
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
}

#define PropertyGet_(x) \
    int QOfonoExtCell::x() const {\
        return Private::value(iPrivate, Private::Property_##x); \
    }
CELL_PROPERTIES(PropertyGet_)

#include "qofonoextcell.moc"
