/****************************************************************************
**
** Copyright (C) 2016 Jolla Ltd.
** Contact: Slava Monich <slava.monich@jolla.com>
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

#include "qofonoextcellwatcher.h"
#include "qofonoextcellinfo.h"
#include "qofonomanager.h"

class QOfonoExtCellWatcher::Private : public QObject {
    Q_OBJECT

public:
    Private(QOfonoExtCellWatcher* aParent);

    QOfonoExtCellWatcher* iParent;
    QSharedPointer<QOfonoManager> iOfonoManager;
    QList<QSharedPointer<QOfonoExtCellInfo> > iCellInfoList;
    QList<QSharedPointer<QOfonoExtCell> > iValidCells;
    QMap<QString, QSharedPointer<QOfonoExtCell> > iKnownCells;

private:
    void updateCellInfo();
    QStringList updateKnownCells();

public Q_SLOTS:
    void updateValidCells();
};

QOfonoExtCellWatcher::Private::Private(QOfonoExtCellWatcher* aParent) :
    QObject(aParent),
    iParent(aParent),
    iOfonoManager(QOfonoManager::instance())
{
    connect(iOfonoManager.data(),
        SIGNAL(availableChanged(bool)),
        SLOT(updateValidCells()));
    connect(iOfonoManager.data(),
        SIGNAL(modemsChanged(QStringList)),
        SLOT(updateValidCells()));
    updateValidCells();
}

void QOfonoExtCellWatcher::Private::updateCellInfo()
{
    int i;
    QStringList modems;
    if (iOfonoManager->available()) {
        modems = iOfonoManager->modems();
    }
    modems.sort();
    bool changed = true;
    if (modems.count() == iCellInfoList.count()) {
        changed = false;
        for (i=0; i<modems.count(); i++) {
            if (iCellInfoList.at(i)->modemPath() != modems.at(i)) {
                changed = true;
                break;
            }
        }
    }
    if (changed) {
        QList<QSharedPointer<QOfonoExtCellInfo> > bak(iCellInfoList);
        for (i=0; i<iCellInfoList.count(); i++) {
            iCellInfoList.at(i)->disconnect(this);
        }
        iCellInfoList.clear();
        for (i=0; i<modems.count(); i++) {
            QSharedPointer<QOfonoExtCellInfo> cellInfo =
                QOfonoExtCellInfo::instance(modems.at(i));
            iCellInfoList.append(cellInfo);
            connect(cellInfo.data(),
                SIGNAL(cellsChanged()),
                SLOT(updateValidCells()));
        }
    }
}

QStringList QOfonoExtCellWatcher::Private::updateKnownCells()
{
    updateCellInfo();

    QStringList allCells;
    int i;
    for (i=0; i<iCellInfoList.count(); i++) {
        allCells.append(iCellInfoList.at(i)->cells());
    }

    QStringList knownCells(iKnownCells.keys());
    allCells.sort();
    knownCells.sort();
    if (allCells != knownCells) {
        QMap<QString, QSharedPointer<QOfonoExtCell> > bak(iKnownCells);
        iKnownCells.clear();
        for (i=0; i<allCells.count(); i++) {
            QString path(allCells.at(i));
            QSharedPointer<QOfonoExtCell> cell = bak.value(path);
            if (cell.isNull()) {
                cell = QSharedPointer<QOfonoExtCell>(new QOfonoExtCell(path), &QObject::deleteLater);
            }
            iKnownCells.insert(path, cell);
            if (!bak.remove(path)) {
                // This is the first time we are seeing this cell
                connect(cell.data(),
                    SIGNAL(validChanged()),
                    SLOT(updateValidCells()));
            }
        }
        // Disconnect those cells that we no longer need
        QList<QSharedPointer<QOfonoExtCell> > leftover = bak.values();
        for (i=0; i<leftover.count(); i++) {
            leftover.at(i)->disconnect(this);
        }
    }

    return allCells;
}

void QOfonoExtCellWatcher::Private::updateValidCells()
{
    QStringList knownCells = updateKnownCells();
    QStringList validCells;
    int i;
    for (i=0; i<knownCells.count(); i++) {
        QString path(knownCells.at(i));
        QSharedPointer<QOfonoExtCell> cell = iKnownCells.value(path);
        if (cell->valid()) {
            validCells.append(path);
        }
    }

    bool changed = true;
    if (validCells.count() == iValidCells.count()) {
        changed = false;
        for (i=0; i<validCells.count(); i++) {
            if (iValidCells.at(i)->path() != validCells.at(i)) {
                changed = true;
                break;
            }
        }
    }

    if (changed && validCells.count() == knownCells.count()) {
        iValidCells.clear();
        for (i=0; i<validCells.count(); i++) {
            iValidCells.append(iKnownCells.value(validCells.at(i)));
        }
        if (iParent) {
            Q_EMIT iParent->cellsChanged();
        }
    }
}

QOfonoExtCellWatcher::QOfonoExtCellWatcher(QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

QOfonoExtCellWatcher::~QOfonoExtCellWatcher()
{
    iPrivate->iParent = NULL;
    iPrivate->deleteLater();
}

QList<QSharedPointer<QOfonoExtCell> > QOfonoExtCellWatcher::cells() const
{
    return iPrivate->iValidCells;
}

#include "qofonoextcellwatcher.moc"
