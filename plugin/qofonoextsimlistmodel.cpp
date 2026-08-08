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

#include "qofonoextsimlistmodel.h"

#include <QQmlEngine>

class QOfonoExtSimListModel::SimData :
    public QObject
{
    friend class QOfonoExtSimListModel;
    Q_OBJECT

public:
    SimData(QOfonoExtSimListModel*, QSharedPointer<QOfonoExtModemManager>,
        QOfonoSimManager::SharedPointer, int index = -1);

private: // SLOTS
    template <Role role> void propertyChanged();
    void onValidChanged();
    void onCardLabelChanged();

private:
    QOfonoExtSimListModel* parentModel() const;
    bool isValid() const;
    int slotNumber() const;

public:
    QSharedPointer<QOfonoExtModemManager> iModemManager;
    QOfonoSimManager::SharedPointer iSim;
    QOfonoExtSimInfo* iCache;
    int iIndex;
    int iSlot;
    bool iValid;
};

QOfonoExtSimListModel::SimData::SimData(
    QOfonoExtSimListModel* aParent,
    QSharedPointer<QOfonoExtModemManager> aModemManager,
    QOfonoSimManager::SharedPointer aSimManager,
    int aIndex) :
    QObject(aParent),
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

    connect(iCache, &QOfonoExtSimInfo::validChanged,
        this, &SimData::onValidChanged);
    connect(iCache, &QOfonoExtSimInfo::subscriberIdentityChanged,
        this, &SimData::propertyChanged<SubscriberIdentityRole>);
    connect(iCache, &QOfonoExtSimInfo::serviceProviderNameChanged,
        this, &SimData::propertyChanged<ServiceProviderNameRole>);
    connect(iCache, &QOfonoExtSimInfo::cardLabelChanged,
        this, &SimData::propertyChanged<LabelRole>);

    connect(iModemManager.data(), &QOfonoExtModemManager::validChanged,
        this, &SimData::onValidChanged);

    connect(sim, &QOfonoSimManager::mobileCountryCodeChanged,
        this, &SimData::propertyChanged<MobileCountryCodeRole>);
    connect(sim, &QOfonoSimManager::mobileNetworkCodeChanged,
        this, &SimData::propertyChanged<MobileNetworkCodeRole>);
    connect(sim, &QOfonoSimManager::subscriberNumbersChanged,
        this, &SimData::propertyChanged<SubscriberNumbersRole>);
    connect(sim, &QOfonoSimManager::serviceNumbersChanged,
        this, &SimData::propertyChanged<ServiceNumbersRole>);
    connect(sim, &QOfonoSimManager::pinRequiredChanged,
        this, &SimData::propertyChanged<PinRequiredRole>);
    connect(sim, &QOfonoSimManager::lockedPinsChanged,
        this, &SimData::propertyChanged<LockedPinsRole>);
    connect(sim, &QOfonoSimManager::cardIdentifierChanged,
        this, &SimData::propertyChanged<CardIdentifierRole>);
    connect(sim, &QOfonoSimManager::preferredLanguagesChanged,
        this, &SimData::propertyChanged<PreferredLanguagesRole>);
    connect(sim, &QOfonoSimManager::pinRetriesChanged,
        this, &SimData::propertyChanged<PinRetriesRole>);
    connect(sim, &QOfonoSimManager::fixedDialingChanged,
        this, &SimData::propertyChanged<FixedDialingRole>);
    connect(sim, &QOfonoSimManager::barredDialingChanged,
        this, &SimData::propertyChanged<BarredDialingRole>);
}

QOfonoExtSimListModel*
QOfonoExtSimListModel::SimData::parentModel() const
{
    return qobject_cast<QOfonoExtSimListModel*>(parent());
}

template <QOfonoExtSimListModel::Role role>
void
QOfonoExtSimListModel::SimData::propertyChanged()
{
    if (iIndex >= 0) {
        QOfonoExtSimListModel* model = parentModel();
        const QModelIndex modelIndex(model->index(iIndex));
        QVector<int> roles;

        roles.append(role);
        Q_EMIT model->dataChanged(modelIndex, modelIndex, roles);
    }
}

bool
QOfonoExtSimListModel::SimData::isValid() const
{
    // QOfonoSimWatcher guarantees that QOfonoSimManager is valid
    return iModemManager->valid() && iCache->valid();
}

int
QOfonoExtSimListModel::SimData::slotNumber() const
{
    // The first slot is 1, second slot 2 and so on
    return iModemManager->availableModems().indexOf(iSim->modemPath()) + 1;
}

void
QOfonoExtSimListModel::SimData::onValidChanged()
{
    const bool valid = isValid();

    if (valid) {
        // Once set, slot number doesn't change
        const int slot = slotNumber();

        if (slot && iSlot != slot) {
            iSlot = slot;
            propertyChanged<SlotRole>();
        }
    }
    if (iValid != valid) {
        iValid = valid;
        propertyChanged<ValidRole>();
        parentModel()->checkValid();
    }
}

// ==========================================================================
// QOfonoExtSimListModel
// ==========================================================================

QOfonoExtSimListModel::QOfonoExtSimListModel(
    QObject* aParent) :
    QAbstractListModel(aParent),
    iModemManager(QOfonoExtModemManager::instance()),
    iSimWatcher(new QOfonoSimWatcher(this)),
    iValid(false)
{
    iSimWatcher->setRequireSubscriberIdentity(false);
    QList<QOfonoSimManager::SharedPointer> sims(iSimWatcher->presentSimList());

    for (int i = 0; i < sims.count(); i++) {
        iSimList.append(new SimData(this, iModemManager, sims.at(i), i));
    }
    iValid = isValid();
    connect(iSimWatcher, &QOfonoSimWatcher::validChanged,
        this, &QOfonoExtSimListModel::onPresentSimListChanged);
    connect(iSimWatcher, &QOfonoSimWatcher::presentSimListChanged,
        this, &QOfonoExtSimListModel::onPresentSimListChanged);
}

bool
QOfonoExtSimListModel::valid() const
{
    return iValid;
}

int
QOfonoExtSimListModel::count() const
{
    return iSimList.count();
}

Qt::ItemFlags
QOfonoExtSimListModel::flags(
    const QModelIndex& aIndex) const
{
    return QAbstractListModel::flags(aIndex) | Qt::ItemIsEditable;
}

QHash<int,QByteArray>
QOfonoExtSimListModel::roleNames() const
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
    roles[LabelRole]               = "label";
    return roles;
}

int
QOfonoExtSimListModel::rowCount(
    const QModelIndex&) const
{
    return iSimList.count();
}

QVariant
QOfonoExtSimListModel::data(
    const QModelIndex& aIndex,
    int aRole) const
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
        case LabelRole:               return d->iCache->cardLabel();
        }
    } else {
        qWarning() << aIndex << aRole;
    }
    return QVariant();
}

bool
QOfonoExtSimListModel::setData(
    const QModelIndex& aIndex,
    const QVariant& aValue,
    int aRole)
{
    const int row = aIndex.row();

    if (row >= 0 && row < iSimList.count()) {
        if (aRole == LabelRole) {
            const SimData* d = iSimList.at(row);

            d->iCache->setCardLabel(aValue.toString());
            return true;
        }
    }
    return QAbstractListModel::setData(aIndex, aValue, aRole);
}

void
QOfonoExtSimListModel::onPresentSimListChanged()
{
    QList<QOfonoSimManager::SharedPointer> sims;

    if (iSimWatcher->isValid()) {
        sims = iSimWatcher->presentSimList();
    }

    const bool countHasChanged(iSimList.count() != sims.count());
    const bool wasValid = iValid;
    QStringList paths;
    int i;

    paths.reserve(sims.count());
    for (i = 0; i < sims.count(); i++) {
        paths.append(sims.at(i)->modemPath());
    }

    // Remove stale entries
    for (i = iSimList.count()-1; i >= 0; i--) {
        QString path(iSimList.at(i)->iSim->modemPath());
        if (!paths.contains(path)) {
            beginRemoveRows(QModelIndex(), i, i);
            delete iSimList.takeAt(i);
            endRemoveRows();
            Q_EMIT simRemoved(path);
        }
    }

    // Add new entries
    for (i = 0; i < sims.count(); i++) {
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

bool
QOfonoExtSimListModel::isValid() const
{
    bool valid = iSimWatcher->isValid();

    for (int i = 0; valid && i < iSimList.count(); i++) {
        if (!iSimList.at(i)->iValid) {
            valid = false;
        }
    }
    return valid;
}

void
QOfonoExtSimListModel::checkValid()
{
    const bool wasValid = iValid;

    iValid = isValid();
    if (iValid != wasValid) {
        Q_EMIT validChanged();
    }
}

#include "qofonoextsimlistmodel.moc"
