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

#ifndef QOFONOEXTSIMINFO_H
#define QOFONOEXTSIMINFO_H

#include "qofonoext_types.h"

class QOFONOEXT_EXPORT QOfonoExtSimInfo :
    public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)
    Q_PROPERTY(QString modemPath READ modemPath WRITE setModemPath NOTIFY modemPathChanged)
    Q_PROPERTY(QString cardIdentifier READ cardIdentifier NOTIFY cardIdentifierChanged)
    Q_PROPERTY(QString subscriberIdentity READ subscriberIdentity NOTIFY subscriberIdentityChanged)
    Q_PROPERTY(QString serviceProviderName READ serviceProviderName NOTIFY serviceProviderNameChanged)
    Q_PROPERTY(QString cardLabel READ cardLabel WRITE setCardLabel NOTIFY cardLabelChanged) // Since 1.2.0

public:
    explicit QOfonoExtSimInfo(QObject* parent = Q_NULLPTR);
    ~QOfonoExtSimInfo();

    bool valid() const;
    QString modemPath() const;
    QString cardIdentifier() const;
    QString subscriberIdentity() const;
    QString serviceProviderName() const;
    QString cardLabel() const; // Since 1.2.0

    void setModemPath(QString);
    void setCardLabel(QString); // Since 1.2.0

Q_SIGNALS:
    void validChanged(bool value);
    void modemPathChanged(QString value);
    void cardIdentifierChanged(QString value);
    void subscriberIdentityChanged(QString value);
    void serviceProviderNameChanged(QString value);
    void cardLabelChanged(QString value); // Since 1.2.0

private:
    class Private;
    Private* iPrivate;
};

#endif // QOFONOEXTSIMINFO_H
