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

#ifndef QOFONOEXTSIMLISTMODEL_H
#define QOFONOEXTSIMLISTMODEL_H

#include "qofonosimwatcher.h"
#include "qofonoextsiminfo.h"
#include "qofonoextmodemmanager.h"

class QOfonoExtSimListModel :
    public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Role {
        PathRole = Qt::UserRole + 1,
        SlotRole,
        ValidRole,
        SubscriberIdentityRole,
        MobileCountryCodeRole,
        MobileNetworkCodeRole,
        ServiceProviderNameRole,
        SubscriberNumbersRole,
        ServiceNumbersRole,
        PinRequiredRole,
        LockedPinsRole,
        CardIdentifierRole,
        PreferredLanguagesRole,
        PinRetriesRole,
        FixedDialingRole,
        BarredDialingRole,
        LabelRole // Since 1.2.0
    };

    explicit QOfonoExtSimListModel(QObject* parent = Q_NULLPTR);

    bool valid() const;
    int count() const;

protected:
    Qt::ItemFlags flags(const QModelIndex&) const Q_DECL_OVERRIDE;
    QHash<int,QByteArray> roleNames() const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex&) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex&, int) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex&, const QVariant&, int) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void validChanged();
    void countChanged();
    void simAdded(QOfonoExtSimInfo* sim);
    void simRemoved(QString path);

private Q_SLOTS:
    void onPresentSimListChanged();

private:
    void checkValid();
    bool isValid() const;

private:
    class SimData;
    friend class SimData;
    QSharedPointer<QOfonoExtModemManager> iModemManager;
    QOfonoSimWatcher* iSimWatcher;
    QList<SimData*> iSimList;
    bool iValid;
};

#endif // QOFONOEXTSIMLISTMODEL_H
