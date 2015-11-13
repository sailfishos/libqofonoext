/*
    Copyright (C) 2015 Jolla Ltd.
    Contact: Aaron McCarthy <aaron.mccarthy@jollamobile.com>
*/

#include "qofonoextdeclarativemodemmanager.h"

QOfonoExtDeclarativeModemManager::QOfonoExtDeclarativeModemManager(QObject *parent)
:   QOfonoExtModemManager(parent)
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
    foreach (bool present, QOfonoExtModemManager::presentSims())
        list.append(present);
    return list;
}
