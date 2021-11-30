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

#include "qofonoextdeclarativeplugin.h"
#include "qofonoextmodemlistmodel.h"
#include "qofonoextmodemmanager.h"
#include "qofonoextsiminfo.h"
#include "qofonoextsimlistmodel.h"
#include "qofonoextmodemlistmodel.h"
#include "qofonoextcellinfo.h"
#include "qofonoextcell.h"

#include <QtQml>

void QOfonoExtDeclarativePlugin::registerTypes(const char* aUri, int aMajor, int aMinor)
{
    Q_ASSERT(QLatin1String(aUri) == "org.nemomobile.ofono");
    qmlRegisterType<QOfonoExtModemManager>(aUri, aMajor, aMinor, "OfonoModemManager");
    qmlRegisterType<QOfonoExtModemListModel>(aUri, aMajor, aMinor, "OfonoModemListModel");
    qmlRegisterType<QOfonoExtSimInfo>(aUri, aMajor, aMinor, "OfonoSimInfo");
    qmlRegisterType<QOfonoExtSimListModel>(aUri, aMajor, aMinor, "OfonoExtSimListModel");
    qmlRegisterType<QOfonoExtCellInfo>(aUri, aMajor, aMinor, "OfonoExtCellInfo");
    qmlRegisterType<QOfonoExtCell>(aUri, aMajor, aMinor, "OfonoExtCell");
}

void QOfonoExtDeclarativePlugin::registerTypes(const char* aUri)
{
    registerTypes(aUri, 1, 0);
}
