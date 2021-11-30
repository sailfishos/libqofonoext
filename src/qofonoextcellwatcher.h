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

#ifndef QOFONOEXTCELLWATCHER_H
#define QOFONOEXTCELLWATCHER_H

#include "qofonoextcell.h"

// Watches available cells from all modems
class QOFONOEXT_EXPORT QOfonoExtCellWatcher : public QObject
{
    Q_OBJECT

public:
    explicit QOfonoExtCellWatcher(QObject* aParent = NULL);
    ~QOfonoExtCellWatcher();

    QList<QSharedPointer<QOfonoExtCell> > cells() const;

Q_SIGNALS:
    void cellsChanged();

private:
    class Private;
    Private* iPrivate;
};

#endif // QOFONOEXTCELLWATCHER_H
