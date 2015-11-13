/****************************************************************************
**
** Copyright (C) 2015 Jolla Ltd.
** Contact: slava.monichi@jolla.com
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

#include "qofonoextdeclarativeplugin.h"
#include "qofonoextmodemlistmodel.h"
#include "qofonoextdeclarativemodemmanager.h"
#include "qofonoextsimsettings.h"

#include <QtQml>

void QOfonoExtDeclarativePlugin::registerTypes(const char* aUri, int aMajor, int aMinor)
{
    Q_ASSERT(QLatin1String(aUri) == "org.nemomobile.ofono");
    qmlRegisterType<QOfonoExtDeclarativeModemManager>(aUri, aMajor, aMinor, "OfonoModemManager");
    qmlRegisterType<QOfonoExtModemListModel>(aUri, aMajor, aMinor, "OfonoModemListModel");
    qmlRegisterType<QOfonoExtSimSettings>(aUri, aMajor, aMinor, "OfonoSimSettings");
}

void QOfonoExtDeclarativePlugin::registerTypes(const char* aUri)
{
    registerTypes(aUri, 1, 0);
}
