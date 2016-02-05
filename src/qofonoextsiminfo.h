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

#ifndef QOFONOEXTSIMINFO_H
#define QOFONOEXTSIMINFO_H

#include "qofonoext_types.h"

class QOFONOEXT_EXPORT QOfonoExtSimInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)
    Q_PROPERTY(QString modemPath READ modemPath WRITE setModemPath NOTIFY modemPathChanged)
    Q_PROPERTY(QString cardIdentifier READ cardIdentifier NOTIFY cardIdentifierChanged)
    Q_PROPERTY(QString subscriberIdentity READ subscriberIdentity NOTIFY subscriberIdentityChanged)
    Q_PROPERTY(QString serviceProviderName READ serviceProviderName NOTIFY serviceProviderNameChanged)

public:
    explicit QOfonoExtSimInfo(QObject* aParent = NULL);
    ~QOfonoExtSimInfo();

    bool valid() const;
    QString modemPath() const;
    QString cardIdentifier() const;
    QString subscriberIdentity() const;
    QString serviceProviderName() const;

    void setModemPath(QString aPath);

Q_SIGNALS:
    void validChanged(bool value);
    void modemPathChanged(QString value);
    void cardIdentifierChanged(QString value);
    void subscriberIdentityChanged(QString value);
    void serviceProviderNameChanged(QString value);

private:
    class Private;
    Private* iPrivate;
};

#endif // QOFONOEXTSIMINFO_H
