/*
    Copyright (C) 2015 Jolla Ltd.
    Contact: Aaron McCarthy <aaron.mccarthy@jollamobile.com>
*/

#ifndef QOFONOEXTDECLARATIVEMODEMMANAGER_H
#define QOFONOEXTDECLARATIVEMODEMMANAGER_H

#include "qofonoextmodemmanager.h"

class QOfonoExtDeclarativeModemManager : public QOfonoExtModemManager
{
    Q_OBJECT

    Q_PROPERTY(QVariantList presentSims READ presentSims NOTIFY presentSimsChanged)

public:
    QOfonoExtDeclarativeModemManager(QObject *parent = 0);
    ~QOfonoExtDeclarativeModemManager();

    QVariantList presentSims() const;

signals:
    void presentSimsChanged();
};

#endif // QOFONOEXTDECLARATIVEMODEMMANAGER_H
