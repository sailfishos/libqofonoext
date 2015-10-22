/****************************************************************************
**
** Copyright (C) 2015 Jolla Ltd.
** Contact: slava.monich@jollamobile.com
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

#ifndef QOFONOEXTSIMSETTINGS_H
#define QOFONOEXTSIMSETTINGS_H

#include "qofonoext_types.h"

class QOFONOEXT_EXPORT QOfonoExtSimSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)
    Q_PROPERTY(bool enable4G READ enable4G NOTIFY enable4GChanged)
    Q_PROPERTY(QString modemPath READ modemPath WRITE setModemPath NOTIFY modemPathChanged)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)

public:
    explicit QOfonoExtSimSettings(QObject* aParent = NULL);
    ~QOfonoExtSimSettings();

    bool valid() const;
    bool enable4G() const;
    QString modemPath() const;
    QString displayName() const;

    void setModemPath(QString aPath);
    void setDisplayName(QString aName);

Q_SIGNALS:
    void validChanged(bool value);
    void enable4GChanged(bool value);
    void modemPathChanged(QString value);
    void displayNameChanged(QString value);

private:
    class Private;
    Private* iPrivate;
};

#endif // QOFONOEXTSIMSETTINGS_H
