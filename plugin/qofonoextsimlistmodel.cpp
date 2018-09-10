/****************************************************************************
**
** Copyright (C) 2016-2018 Jolla Ltd.
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

#include "qofonoextsimlistmodel.h"
#include <QQmlEngine>

class QOfonoExtSimListModel::SimData : public QObject {
public:
    friend class QOfonoExtSimListModel;
    Q_OBJECT

    SimData(QOfonoExtSimListModel* aParent,
            QSharedPointer<QOfonoExtModemManager> aModemManager,
            QOfonoSimManager::SharedPointer aSimManager,
            int aIndex = -1);

private Q_SLOTS:
    void onValidChanged();
    void onSubscriberIdentityChanged();
    void onMobileCountryCodeChanged();
    void onMobileNetworkCodeChanged();
    void onServiceProviderNameChanged();
    void onSubscriberNumbersChanged();
    void onServiceNumbersChanged();
    void onPinRequiredChanged();
    void onLockedPinsChanged();
    void onCardIdentifierChanged();
    void onPreferredLanguagesChanged();
    void onPinRetriesChanged();
    void onFixedDialingChanged();
    void onBarredDialingChanged();

private:
    void propertyChanged(Role role);
    bool isValid() const;
    int slotNumber() const;

public:
    QOfonoExtSimListModel* iParent;
    QSharedPointer<QOfonoExtModemManager> iModemManager;
    QOfonoSimManager::SharedPointer iSim;
    QOfonoExtSimInfo* iCache;
    int iIndex;
    int iSlot;
    bool iValid;
};

QOfonoExtSimListModel::SimData::SimData(QOfonoExtSimListModel* aParent,
    QSharedPointer<QOfonoExtModemManager> aModemManager,
    QOfonoSimManager::SharedPointer aSimManager, int aIndex) :
    QObject(aParent),
    iParent(aParent),
    iModemManager(aModemManager),
    iSim(aSimManager),
    iCache(new QOfonoExtSimInfo(this)),
    iIndex(aIndex)
{
    iSlot = slotNumber();
    iValid = isValid();

    QOfonoSimManager* sim = iSim.data();
    QQmlEngine::setObjectOwnership(iCache, QQmlEngine::CppOwnership);
    iCache->setModemPath(sim->modemPath());

    connect(iCache,
        SIGNAL(validChanged(bool)),
        SLOT(onValidChanged()));
    connect(iCache,
        SIGNAL(subscriberIdentityChanged(QString)),
        SLOT(onSubscriberIdentityChanged()));
    connect(iCache,
        SIGNAL(serviceProviderNameChanged(QString)),
        SLOT(onServiceProviderNameChanged()));

    connect(iModemManager.data(),
        SIGNAL(validChanged(bool)),
        SLOT(onValidChanged()));

    connect(sim,
        SIGNAL(mobileCountryCodeChanged(QString)),
        SLOT(onMobileCountryCodeChanged()));
    connect(sim,
        SIGNAL(mobileNetworkCodeChanged(QString)),
        SLOT(onMobileNetworkCodeChanged()));
    connect(sim,
        SIGNAL(subscriberNumbersChanged(QStringList)),
        SLOT(onSubscriberNumbersChanged()));
    connect(sim,
        SIGNAL(serviceNumbersChanged(QVariantMap)),
        SLOT(onServiceNumbersChanged()));
    connect(sim,
        SIGNAL(pinRequiredChanged(int)),
        SLOT(onPinRequiredChanged()));
    connect(sim,
        SIGNAL(lockedPinsChanged(QVariantList)),
        SLOT(onLockedPinsChanged()));
    connect(sim,
        SIGNAL(cardIdentifierChanged(QString)),
        SLOT(onCardIdentifierChanged()));
    connect(sim,
        SIGNAL(preferredLanguagesChanged(QStringList)),
        SLOT(onPreferredLanguagesChanged()));
    connect(sim,
        SIGNAL(pinRetriesChanged(QVariantMap)),
        SLOT(onPinRetriesChanged()));
    connect(sim,
        SIGNAL(fixedDialingChanged(bool)),
        SLOT(onFixedDialingChanged()));
    connect(sim,
        SIGNAL(barredDialingChanged(bool)),
        SLOT(onBarredDialingChanged()));
}

void QOfonoExtSimListModel::SimData::propertyChanged(Role role)
{
    if (iIndex >= 0) {
        QModelIndex modelIndex = iParent->index(iIndex);
        QVector<int> roles;
        roles.append(role);
        Q_EMIT iParent->dataChanged(modelIndex, modelIndex, roles);
    }
}

bool QOfonoExtSimListModel::SimData::isValid() const
{
    // QOfonoSimWatcher guarantees that QOfonoSimManager is valid
    return iModemManager->valid() && iCache->valid();
}

int QOfonoExtSimListModel::SimData::slotNumber() const
{
    // The first slot is 1, second slot 2 and so on
    return iModemManager->availableModems().indexOf(iSim->modemPath()) + 1;
}

void QOfonoExtSimListModel::SimData::onValidChanged()
{
    const bool valid = isValid();
    if (valid) {
        // Once set, slot number doesn't change
        const int slot = slotNumber();
        if (slot && iSlot != slot) {
            iSlot = slot;
            propertyChanged(SlotRole);
        }
    }
    if (iValid != valid) {
        iValid = valid;
        propertyChanged(ValidRole);
        iParent->checkValid();
    }
}

void QOfonoExtSimListModel::SimData::onSubscriberIdentityChanged()
{
    propertyChanged(SubscriberIdentityRole);
}

void QOfonoExtSimListModel::SimData::onMobileCountryCodeChanged()
{
    propertyChanged(MobileCountryCodeRole);
}

void QOfonoExtSimListModel::SimData::onMobileNetworkCodeChanged()
{
    propertyChanged(MobileNetworkCodeRole);
}

void QOfonoExtSimListModel::SimData::onServiceProviderNameChanged()
{
    propertyChanged(ServiceProviderNameRole);
}

void QOfonoExtSimListModel::SimData::onSubscriberNumbersChanged()
{
    propertyChanged(SubscriberNumbersRole);
}

void QOfonoExtSimListModel::SimData::onServiceNumbersChanged()
{
    propertyChanged(ServiceNumbersRole);
}

void QOfonoExtSimListModel::SimData::onPinRequiredChanged()
{
    propertyChanged(PinRequiredRole);
}

void QOfonoExtSimListModel::SimData::onLockedPinsChanged()
{
    propertyChanged(LockedPinsRole);
}

void QOfonoExtSimListModel::SimData::onCardIdentifierChanged()
{
    propertyChanged(CardIdentifierRole);
}

void QOfonoExtSimListModel::SimData::onPreferredLanguagesChanged()
{
    propertyChanged(PreferredLanguagesRole);
}

void QOfonoExtSimListModel::SimData::onPinRetriesChanged()
{
    propertyChanged(PinRetriesRole);
}

void QOfonoExtSimListModel::SimData::onFixedDialingChanged()
{
    propertyChanged(FixedDialingRole);
}

void QOfonoExtSimListModel::SimData::onBarredDialingChanged()
{
    propertyChanged(BarredDialingRole);
}

// ==========================================================================
// QOfonoExtSimListModel
// ==========================================================================

QOfonoExtSimListModel::QOfonoExtSimListModel(QObject *aParent) :
    QAbstractListModel(aParent),
    iModemManager(QOfonoExtModemManager::instance()),
    iSimWatcher(new QOfonoSimWatcher(this)),
    iValid(false)
{
    iSimWatcher->setRequireSubscriberIdentity(false);
    QList<QOfonoSimManager::SharedPointer> sims(iSimWatcher->presentSimList());
    for (int i=0; i<sims.count(); i++) {
        iSimList.append(new SimData(this, iModemManager, sims.at(i), i));
    }
    iValid = isValid();
    connect(iSimWatcher,
        SIGNAL(validChanged()),
        SLOT(onPresentSimListChanged()));
    connect(iSimWatcher,
        SIGNAL(presentSimListChanged()),
        SLOT(onPresentSimListChanged()));
}

bool QOfonoExtSimListModel::valid() const
{
    return iValid;
}

int QOfonoExtSimListModel::count() const
{
    return iSimList.count();
}

QHash<int,QByteArray> QOfonoExtSimListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[SlotRole]                = "slot";
    roles[PathRole]                = "path";
    roles[ValidRole]               = "valid";
    roles[SubscriberIdentityRole]  = "subscriberIdentity";
    roles[MobileCountryCodeRole]   = "mobileCountryCode";
    roles[MobileNetworkCodeRole]   = "mobileNetworkCode";
    roles[ServiceProviderNameRole] = "serviceProviderName";
    roles[SubscriberNumbersRole]   = "subscriberNumbers";
    roles[ServiceNumbersRole]      = "serviceNumbers";
    roles[PinRequiredRole]         = "pinRequired";
    roles[LockedPinsRole]          = "lockedPins";
    roles[CardIdentifierRole]      = "cardIdentifier";
    roles[PreferredLanguagesRole]  = "preferredLanguages";
    roles[PinRetriesRole]          = "pinRetries";
    roles[FixedDialingRole]        = "fixedDialing";
    roles[BarredDialingRole]       = "barredDialing";
    return roles;
}

int QOfonoExtSimListModel::rowCount(const QModelIndex&) const
{
    return iSimList.count();
}

QVariant QOfonoExtSimListModel::data(const QModelIndex& aIndex, int aRole) const
{
    const int row = aIndex.row();
    if (row >= 0 && row < iSimList.count()) {
        const SimData* d = iSimList.at(row);
        switch (aRole) {
        case SlotRole:                return d->iSlot;
        case ValidRole:               return d->iValid;
        case PathRole:                return d->iSim->modemPath();
        case MobileCountryCodeRole:   return d->iSim->mobileCountryCode();
        case MobileNetworkCodeRole:   return d->iSim->mobileNetworkCode();
        case SubscriberNumbersRole:   return d->iSim->subscriberNumbers();
        case ServiceNumbersRole:      return d->iSim->serviceNumbers();
        case PinRequiredRole:         return d->iSim->pinRequired();
        case LockedPinsRole:          return d->iSim->lockedPins();
        case CardIdentifierRole:      return d->iSim->cardIdentifier();
        case PreferredLanguagesRole:  return d->iSim->preferredLanguages();
        case PinRetriesRole:          return d->iSim->pinRetries();
        case FixedDialingRole:        return d->iSim->fixedDialing();
        case BarredDialingRole:       return d->iSim->barredDialing();
        case SubscriberIdentityRole:  return d->iCache->subscriberIdentity();
        case ServiceProviderNameRole: return d->iCache->serviceProviderName();
        }
    } else {
        qWarning() << aIndex << aRole;
    }
    return QVariant();
}

void QOfonoExtSimListModel::onPresentSimListChanged()
{
    QList<QOfonoSimManager::SharedPointer> sims;
    if (iSimWatcher->isValid()) {
        sims = iSimWatcher->presentSimList();
    }
    const bool countHasChanged(iSimList.count() != sims.count());
    const bool wasValid = iValid;
    QStringList paths;
    int i;
    for (i=0; i<sims.count(); i++) {
        paths.append(sims.at(i)->modemPath());
    }
    // Remove stale entries
    for (i=iSimList.count()-1; i>=0; i--) {
        QString path(iSimList.at(i)->iSim->modemPath());
        if (!paths.contains(path)) {
            beginRemoveRows(QModelIndex(), i, i);
            delete iSimList.takeAt(i);
            endRemoveRows();
            Q_EMIT simRemoved(path);
        }
    }
    // Add new entries
    for (i=0; i<sims.count(); i++) {
        if (iSimList.count() <= i ||
            iSimList.at(i)->iSim->modemPath() != paths.at(i)) {
            SimData* data = new SimData(this, iModemManager, sims.at(i), i);
            beginInsertRows(QModelIndex(), i, i);
            iSimList.insert(i, data);
            endInsertRows();
            Q_EMIT simAdded(data->iCache);
        } else {
            iSimList.at(i)->iIndex = i;
        }
    }
    if (countHasChanged) {
        Q_EMIT countChanged();
    }
    iValid = isValid();
    if (iValid != wasValid) {
        Q_EMIT validChanged();
    }
}

bool QOfonoExtSimListModel::isValid() const
{
    bool valid = iSimWatcher->isValid();
    for (int i=0; valid && i<iSimList.count(); i++) {
        if (!iSimList.at(i)->iValid) {
            valid = false;
        }
    }
    return valid;
}

void QOfonoExtSimListModel::checkValid()
{
    const bool wasValid = iValid;
    iValid = isValid();
    if (iValid != wasValid) {
        Q_EMIT validChanged();
    }
}

#include "qofonoextsimlistmodel.moc"
