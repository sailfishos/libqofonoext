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

#ifndef QOFONOEXT_TYPES_H
#define QOFONOEXT_TYPES_H

#include <QtCore>

#ifndef QOFONOEXT_EXPORT
#  if defined(QOFONOEXT_LIBRARY)
#    define QOFONOEXT_EXPORT Q_DECL_EXPORT
#  else
#    define QOFONOEXT_EXPORT Q_DECL_IMPORT
#  endif
#endif // QOFONOEXT_EXPORT

#endif // QOFONOEXT_TYPES_H
