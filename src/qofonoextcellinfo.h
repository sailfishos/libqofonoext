/****************************************************************************
**
** Copyright (C) 2016-2020 Jolla Ltd.
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

#ifndef QOFONOEXTCELLINFO_H
#define QOFONOEXTCELLINFO_H

#include "qofonoext_types.h"

class QOFONOEXT_EXPORT QOfonoExtCellInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString modemPath READ modemPath WRITE setModemPath NOTIFY modemPathChanged)
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)
    Q_PROPERTY(QStringList cells READ cells NOTIFY cellsChanged)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)

public:
    explicit QOfonoExtCellInfo(QObject* aParent = Q_NULLPTR);
    QOfonoExtCellInfo(QString aModemPath, QObject* aParent = Q_NULLPTR); // Blocks (since 1.0.27)
    ~QOfonoExtCellInfo();

    // Shared instance(s) for C++ use
    static QSharedPointer<QOfonoExtCellInfo> instance(QString aModemPath);
    static QSharedPointer<QOfonoExtCellInfo> instance(QString aModemPath, bool aMayBlock); // Since 1.0.27

    QString modemPath() const;
    void setModemPath(QString aModemPath);
    void setActive(bool val);

    bool valid() const;
    QStringList cells() const;
    bool active() const;

Q_SIGNALS:
    void validChanged();
    void modemPathChanged();
    void cellsChanged();
    void cellsAdded(QStringList cells);
    void cellsRemoved(QStringList cells);
    void activeChanged();

private:
    class Private;
    Private* iPrivate;
};

#endif // QOFONOEXTCELLINFO_H
