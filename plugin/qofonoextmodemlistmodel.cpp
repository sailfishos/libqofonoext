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

#include "qofonoextmodemlistmodel.h"

QOfonoExtModemListModel::QOfonoExtModemListModel(
    QObject* aParent) :
    QAbstractListModel(aParent),
    iModemManager(QOfonoExtModemManager::instance()),
    iAvailableModems(iModemManager->availableModems()),
    iEnabledModems(iModemManager->enabledModems()),
    iDefaultVoiceModem(iModemManager->defaultVoiceModem()),
    iDefaultDataModem(iModemManager->defaultDataModem())
{
    QOfonoExtModemManager* mm = iModemManager.data();

    connect(mm, &QOfonoExtModemManager::validChanged,
        this, &QOfonoExtModemListModel::onValidChanged);
    connect(mm, &QOfonoExtModemManager::availableModemsChanged,
        this, &QOfonoExtModemListModel::onAvailableModemsChanged);
    connect(mm, &QOfonoExtModemManager::enabledModemsChanged,
        this, &QOfonoExtModemListModel::onEnabledModemsChanged);
    connect(mm, &QOfonoExtModemManager::defaultDataModemChanged,
        this, &QOfonoExtModemListModel::onDefaultDataModemChanged);
    connect(mm, &QOfonoExtModemManager::defaultVoiceModemChanged,
        this, &QOfonoExtModemListModel::onDefaultVoiceModemChanged);
    connect(mm, &QOfonoExtModemManager::presentSimChanged,
        this, &QOfonoExtModemListModel::onPresentSimChanged);
    connect(mm, &QOfonoExtModemManager::imeiCodesChanged,
        this, &QOfonoExtModemListModel::onImeiCodesChanged);
    connect(mm, &QOfonoExtModemManager::imeisvCodesChanged,
        this, &QOfonoExtModemListModel::onImeisvCodesChanged);
}

bool
QOfonoExtModemListModel::valid() const
{
    return iModemManager->valid();
}

int
QOfonoExtModemListModel::count() const
{
    return iAvailableModems.count();
}

QHash<int,QByteArray>
QOfonoExtModemListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[PathRole]         = "path";
    roles[EnabledRole]      = "enabled";
    roles[DefaultDataRole]  = "defaultForData";
    roles[DefaultVoiceRole] = "defaultForVoice";
    roles[SimPresentRole]   = "simPresent";
    roles[IMEIRole]         = "imei";
    roles[IMEISVRole]       = "imeisv";
    return roles;
}

int
QOfonoExtModemListModel::rowCount(
    const QModelIndex&) const
{
    return iAvailableModems.count();
}

QVariant
QOfonoExtModemListModel::data(
    const QModelIndex& aIndex,
    int aRole) const
{
    const int row = aIndex.row();

    if (row >= 0 && row < iAvailableModems.count()) {
        switch (aRole) {
        case PathRole:         return iAvailableModems.at(row);
        case EnabledRole:      return iEnabledModems.contains(iAvailableModems.at(row));
        case DefaultDataRole:  return iAvailableModems.indexOf(iDefaultDataModem) == row;
        case DefaultVoiceRole: return iAvailableModems.indexOf(iDefaultVoiceModem) == row;
        case SimPresentRole:   return iModemManager->simPresentAt(row);
        case IMEIRole:         return iModemManager->imeiAt(row);
        case IMEISVRole:       return iModemManager->imeisvAt(row);
        }
    }
    qWarning() << aIndex << aRole;
    return QVariant();
}

bool
QOfonoExtModemListModel::setData(
    const QModelIndex& aIndex,
    const QVariant& aValue,
    int aRole)
{
    const int row = aIndex.row();

    if (row >= 0 && row < iAvailableModems.count() && aRole == EnabledRole) {
        const bool enabled = aValue.toBool();
        const QString& path(iAvailableModems.at(row));
        const int index = iEnabledModems.indexOf(path);

        if (enabled != (index >= 0)) {
            QStringList enabledModems = iEnabledModems;
            if (enabled) {
                enabledModems.append(path);
            } else {
                enabledModems.removeAt(index);
            }
            iModemManager->setEnabledModems(enabledModems);
        }
        return true;
    }
    return false;
}

void
QOfonoExtModemListModel::onValidChanged(
    bool aValid)
{
    if (aValid) {
        beginResetModel();
        endResetModel();
    }
    Q_EMIT validChanged(aValid);
}

void
QOfonoExtModemListModel::onAvailableModemsChanged(
    const QStringList& aModems)
{
    const bool countHasChanged = iAvailableModems.count() != aModems.count();

    beginResetModel();
    iAvailableModems = aModems;
    endResetModel();
    if (countHasChanged) {
        Q_EMIT countChanged(iAvailableModems.count());
    }
}

void
QOfonoExtModemListModel::onEnabledModemsChanged(
    const QStringList& aModems)
{
    if (iEnabledModems != aModems) {
        const QStringList prevModems(iEnabledModems);
        const int n = iAvailableModems.count();
        QVector<int> role;

        role.append(EnabledRole);
        iEnabledModems = aModems;
        for (int i = 0; i < n; i++) {
            const QString& path(iAvailableModems.at(i));

            if (prevModems.contains(path) != aModems.contains(path)) {
                QModelIndex index(createIndex(i, 0));

                Q_EMIT dataChanged(index, index, role);
            }
        }
    }
}

void
QOfonoExtModemListModel::onDefaultDataModemChanged(
    QString aModemPath)
{
    const int prevIndex = iAvailableModems.indexOf(iDefaultDataModem);

    iDefaultDataModem = aModemPath;
    defaultModemChanged(DefaultDataRole, prevIndex, iAvailableModems.indexOf(aModemPath));
}

void
QOfonoExtModemListModel::onDefaultVoiceModemChanged(
    QString aModemPath)
{
    const int prevIndex = iAvailableModems.indexOf(iDefaultVoiceModem);

    iDefaultVoiceModem = aModemPath;
    defaultModemChanged(DefaultVoiceRole, prevIndex, iAvailableModems.indexOf(aModemPath));
}

void
QOfonoExtModemListModel::onPresentSimChanged(
    int aIndex,
    bool aPresent)
{
    const QModelIndex index(createIndex(aIndex, 0));
    QVector<int> role;

    role.append(SimPresentRole);
    Q_EMIT dataChanged(index, index, role);
}

void
QOfonoExtModemListModel::defaultModemChanged(
    Role aRole,
    int aPrevRow,
    int aNewRow)
{
    if (aPrevRow != aNewRow) {
        QVector<int> role;

        role.append(aRole);
        if (aPrevRow >= 0) {
            QModelIndex index(createIndex(aPrevRow, 0));

            Q_EMIT dataChanged(index, index, role);
        }
        if (aNewRow >= 0) {
            QModelIndex index(createIndex(aNewRow, 0));

            Q_EMIT dataChanged(index, index, role);
        }
    }
}

void
QOfonoExtModemListModel::onImeiCodesChanged(
    const QStringList& aList)
{
    const QStringList prev(iImeiList);

    iImeiList = aList;
    roleChanged(IMEIRole, prev, aList);
}

void
QOfonoExtModemListModel::onImeisvCodesChanged(
    const QStringList& aList)
{
    const QStringList prev(iImeisvList);

    iImeisvList = aList;
    roleChanged(IMEISVRole, prev, aList);
}

void
QOfonoExtModemListModel::roleChanged(
    Role aRole,
    const QStringList& aPrevList,
    const QStringList& aNewList)
{
    // This is slightly paranoid... All these 3 counts should be the same
    const int n1 = iAvailableModems.count();
    const int n2 = aPrevList.count();
    const int n3 = aNewList.count();
    const int n = qMin(n1, qMin(n2, n3));
    QVector<int> role;

    role.append(aRole);
    for (int i = 0; i < n; i++) {
        if (aPrevList.at(i) != aNewList.at(i)) {
            const QModelIndex index(createIndex(i, 0));

            Q_EMIT dataChanged(index, index, role);
        }
    }
}
