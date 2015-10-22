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

#ifndef QOFONOEXTMODEMLISTMODEL_H
#define QOFONOEXTMODEMLISTMODEL_H

#include "qofonoextmodemmanager.h"

#include <QAbstractListModel>

class QOfonoExtModemListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Role {
        PathRole = Qt::UserRole,
        EnabledRole,
        DefaultDataRole,
        DefaultVoiceRole
    };

    explicit QOfonoExtModemListModel(QObject* aParent = NULL);

    bool valid() const;
    int count() const;

    QHash<int,QByteArray> roleNames() const;
    int rowCount(const QModelIndex& aParent) const;
    QVariant data(const QModelIndex& aIndex, int aRole) const;
    bool setData(const QModelIndex& aIndex, const QVariant& aValue, int aRole);

Q_SIGNALS:
    void validChanged(bool aValid);
    void countChanged(int aCount);

private Q_SLOTS:
    void onAvailableModemsChanged(QStringList aModems);
    void onEnabledModemsChanged(QStringList aModems);
    void onDefaultDataModemChanged(QString aModemPath);
    void onDefaultVoiceModemChanged(QString aModemPath);

private:
    void defaultModemChanged(Role aRole, int aPrevRow, int aNewRow);

private:
    QSharedPointer<QOfonoExtModemManager> iModemManager;
    QStringList iAvailableModems;
    QStringList iEnabledModems;
    QString iDefaultVoiceModem;
    QString iDefaultDataModem;
};

#endif // QOFONOEXTMODEMLISTMODEL_H
