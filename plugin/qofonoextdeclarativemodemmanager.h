/****************************************************************************
**
** Copyright (C) 2015 Jolla Ltd.
** Contact: Aaron McCarthy <aaron.mccarthy@jollamobile.com>
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

#ifndef QOFONOEXTDECLARATIVEMODEMMANAGER_H
#define QOFONOEXTDECLARATIVEMODEMMANAGER_H

#include "qofonoextmodemmanager.h"

class QOfonoExtDeclarativeModemManager : public QOfonoExtModemManager
{
    Q_OBJECT
    Q_PROPERTY(QVariantList presentSims READ presentSims NOTIFY presentSimsChanged)

public:
    QOfonoExtDeclarativeModemManager(QObject* aParent = NULL);
    ~QOfonoExtDeclarativeModemManager();

    QVariantList presentSims() const;

signals:
    void presentSimsChanged();
};

#endif // QOFONOEXTDECLARATIVEMODEMMANAGER_H
