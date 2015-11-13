/****************************************************************************
**
** Copyright (C) 2015 Jolla Ltd.
** Contact: slava.monichi@jolla.com
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

QOfonoExtModemListModel::QOfonoExtModemListModel(QObject* aParent) :
    QAbstractListModel(aParent),
    iModemManager(QOfonoExtModemManager::instance()),
    iAvailableModems(iModemManager->availableModems()),
    iEnabledModems(iModemManager->enabledModems()),
    iDefaultVoiceModem(iModemManager->defaultVoiceModem()),
    iDefaultDataModem(iModemManager->defaultDataModem())
{
    connect(iModemManager.data(),
        SIGNAL(validChanged(bool)),
        SLOT(onValidChanged(bool)));
    connect(iModemManager.data(),
        SIGNAL(availableModemsChanged(QStringList)),
        SLOT(onAvailableModemsChanged(QStringList)));
    connect(iModemManager.data(),
        SIGNAL(enabledModemsChanged(QStringList)),
        SLOT(onEnabledModemsChanged(QStringList)));
    connect(iModemManager.data(),
        SIGNAL(defaultDataModemChanged(QString)),
        SLOT(onDefaultDataModemChanged(QString)));
    connect(iModemManager.data(),
        SIGNAL(defaultVoiceModemChanged(QString)),
        SLOT(onDefaultVoiceModemChanged(QString)));
    connect(iModemManager.data(),
        SIGNAL(presentSimChanged(int,bool)),
        SLOT(onPresentSimChanged(int,bool)));
}

bool QOfonoExtModemListModel::valid() const
{
    return iModemManager->valid();
}

int QOfonoExtModemListModel::count() const
{
    return iAvailableModems.count();
}

QHash<int,QByteArray> QOfonoExtModemListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[PathRole]         = "path";
    roles[EnabledRole]      = "enabled";
    roles[DefaultDataRole]  = "defaultForData";
    roles[DefaultVoiceRole] = "defaultForVoice";
    roles[SimPresentRole]   = "simPresent";
    roles[IMEIRole]         = "imei";
    return roles;
}

int QOfonoExtModemListModel::rowCount(const QModelIndex& aParent) const
{
    return iAvailableModems.count();
}

QVariant QOfonoExtModemListModel::data(const QModelIndex& aIndex, int aRole) const
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
        }
    }
    qWarning() << aIndex << aRole;
    return QVariant();
}

bool QOfonoExtModemListModel::setData(const QModelIndex& aIndex, const QVariant& aValue, int aRole)
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

void QOfonoExtModemListModel::onValidChanged(bool aValid)
{
    if (aValid) {
        beginResetModel();
        endResetModel();
    }
    Q_EMIT validChanged(aValid);
}

void QOfonoExtModemListModel::onAvailableModemsChanged(QStringList aModems)
{
    const bool countHasChanged = iAvailableModems.count() != aModems.count();
    beginResetModel();
    iAvailableModems = aModems;
    endResetModel();
    if (countHasChanged) {
        Q_EMIT countChanged(iAvailableModems.count());
    }
}

void QOfonoExtModemListModel::onEnabledModemsChanged(QStringList aModems)
{
    if (iEnabledModems != aModems) {
        QStringList prevModems = iEnabledModems;
        iEnabledModems = aModems;
        const int n = iAvailableModems.count();
        QVector<int> role;
        role.append(EnabledRole);
        for (int i=0; i<n; i++) {
            const QString& path(iAvailableModems.at(i));
            if (prevModems.contains(path) != aModems.contains(path)) {
                QModelIndex index(createIndex(i, 0));
                Q_EMIT dataChanged(index, index, role);
            }
        }
    }
}

void QOfonoExtModemListModel::onDefaultDataModemChanged(QString aModemPath)
{
    const int prevIndex = iAvailableModems.indexOf(iDefaultDataModem);
    iDefaultDataModem = aModemPath;
    defaultModemChanged(DefaultDataRole, prevIndex, iAvailableModems.indexOf(aModemPath));
}

void QOfonoExtModemListModel::onDefaultVoiceModemChanged(QString aModemPath)
{
    const int prevIndex = iAvailableModems.indexOf(iDefaultVoiceModem);
    iDefaultVoiceModem = aModemPath;
    defaultModemChanged(DefaultVoiceRole, prevIndex, iAvailableModems.indexOf(aModemPath));
}

void QOfonoExtModemListModel::onPresentSimChanged(int aIndex, bool aPresent)
{
    QVector<int> role;
    role.append(SimPresentRole);
    QModelIndex index(createIndex(aIndex, 0));
    Q_EMIT dataChanged(index, index, role);
}

void QOfonoExtModemListModel::defaultModemChanged(Role aRole, int aPrevRow, int aNewRow)
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
