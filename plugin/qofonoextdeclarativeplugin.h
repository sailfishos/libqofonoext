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

#ifndef QOFONOEXTDECLARATIVEPLUGIN_H
#define QOFONOEXTDECLARATIVEPLUGIN_H

#include "qofonoext_types.h"

#include <QQmlExtensionPlugin>

class QOFONOEXT_EXPORT QOfonoExtDeclarativePlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char* aUri);
    static void registerTypes(const char* aUri, int aMajor, int aMinor);
};

#endif // QOFONOEXTDECLARATIVEPLUGIN_H
