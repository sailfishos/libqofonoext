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

#ifndef QOFONOEXTMODEMLISTMODEL_H
#define QOFONOEXTMODEMLISTMODEL_H

#include "qofonoextmodemmanager.h"

#include <QAbstractListModel>

class QOfonoExtModemListModel :
    public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Role {
        PathRole = Qt::UserRole,
        EnabledRole,
        DefaultDataRole,
        DefaultVoiceRole,
        SimPresentRole,
        IMEIRole,
        IMEISVRole
    };

    explicit QOfonoExtModemListModel(QObject* parent = Q_NULLPTR);

    bool valid() const;
    int count() const;

    QHash<int,QByteArray> roleNames() const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex&) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex&, int) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex&, const QVariant&, int) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void validChanged(bool aValid);
    void countChanged(int aCount);

private Q_SLOTS:
    void onValidChanged(bool aValid);
    void onAvailableModemsChanged(const QStringList&);
    void onEnabledModemsChanged(const QStringList&);
    void onDefaultDataModemChanged(QString);
    void onDefaultVoiceModemChanged(QString);
    void onPresentSimChanged(int, bool);
    void onImeiCodesChanged(const QStringList&);
    void onImeisvCodesChanged(const QStringList&);

private:
    void defaultModemChanged(Role, int, int);
    void roleChanged(Role aRole, const QStringList&, const QStringList&);

private:
    QSharedPointer<QOfonoExtModemManager> iModemManager;
    QStringList iAvailableModems;
    QStringList iEnabledModems;
    QString iDefaultVoiceModem;
    QString iDefaultDataModem;
    QStringList iImeiList;
    QStringList iImeisvList;
};

#endif // QOFONOEXTMODEMLISTMODEL_H
