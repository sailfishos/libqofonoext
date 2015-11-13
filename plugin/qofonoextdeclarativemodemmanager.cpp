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

#include "qofonoextdeclarativemodemmanager.h"

QOfonoExtDeclarativeModemManager::QOfonoExtDeclarativeModemManager(QObject* aParent) :
    QOfonoExtModemManager(aParent)
{
    connect(this, SIGNAL(presentSimsChanged(QOfonoExtBoolList)),
            this, SIGNAL(presentSimsChanged()));
}

QOfonoExtDeclarativeModemManager::~QOfonoExtDeclarativeModemManager()
{
}

QVariantList QOfonoExtDeclarativeModemManager::presentSims() const
{
    QVariantList list;
    foreach (bool present, QOfonoExtModemManager::presentSims()) {
        list.append(present);
    }
    return list;
}
