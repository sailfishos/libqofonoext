/****************************************************************************
**
** Copyright (C) 2016 Jolla Ltd.
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

#ifndef QOFONOEXTSIMLISTMODEL_H
#define QOFONOEXTSIMLISTMODEL_H

#include "qofonosimwatcher.h"
#include "qofonoextsiminfo.h"

class QOfonoExtSimListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Role {
        PathRole = Qt::UserRole + 1,
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
        BarredDialingRole
    };

    explicit QOfonoExtSimListModel(QObject* aParent = NULL);

    bool valid() const;
    int count() const;

protected:
    QHash<int,QByteArray> roleNames() const;
    int rowCount(const QModelIndex& aParent) const;
    QVariant data(const QModelIndex& aIndex, int aRole) const;

Q_SIGNALS:
    void validChanged();
    void countChanged();
    void simAdded(QOfonoExtSimInfo* sim);
    void simRemoved(QString path);

private Q_SLOTS:
    void onPresentSimListChanged();
    void onValidChanged();

private:
    bool isValid() const;

private:
    class SimData;
    QOfonoSimWatcher* iSimWatcher;
    QList<SimData*> iSimList;
    bool iValid;
};

#endif // QOFONOEXTSIMLISTMODEL_H
