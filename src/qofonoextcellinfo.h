/****************************************************************************
**
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

#ifndef QOFONOEXTCELLINFO_H
#define QOFONOEXTCELLINFO_H

#include "qofonoext_types.h"

class QOFONOEXT_EXPORT QOfonoExtCellInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString modemPath READ modemPath WRITE setModemPath NOTIFY modemPathChanged)
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)
    Q_PROPERTY(QStringList cells READ cells NOTIFY cellsChanged)

public:
    explicit QOfonoExtCellInfo(QObject* aParent = Q_NULLPTR);
    QOfonoExtCellInfo(QString aModemPath, QObject* aParent = Q_NULLPTR); // Blocks (since 1.0.27)
    ~QOfonoExtCellInfo();

    // Shared instance(s) for C++ use
    static QSharedPointer<QOfonoExtCellInfo> instance(QString aModemPath);
    static QSharedPointer<QOfonoExtCellInfo> instance(QString aModemPath, bool aMayBlock); // Since 1.0.27

    QString modemPath() const;
    void setModemPath(QString aModemPath);

    bool valid() const;
    QStringList cells() const;

Q_SIGNALS:
    void validChanged();
    void modemPathChanged();
    void cellsChanged();
    void cellsAdded(QStringList cells);
    void cellsRemoved(QStringList cells);

private:
    class Private;
    Private* iPrivate;
};

#endif // QOFONOEXTCELLINFO_H
