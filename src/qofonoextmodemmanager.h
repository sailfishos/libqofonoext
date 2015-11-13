/****************************************************************************
**
** Copyright (C) 2015 Jolla Ltd.
** Contact: slava.monich@jollamobile.com
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

#ifndef QOFONOEXTMODEMMANAGER_H
#define QOFONOEXTMODEMMANAGER_H

#include "qofonoext_types.h"

class QOFONOEXT_EXPORT QOfonoExtModemManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)
    Q_PROPERTY(QStringList availableModems READ availableModems NOTIFY availableModemsChanged)
    Q_PROPERTY(QStringList enabledModems READ enabledModems WRITE setEnabledModems NOTIFY enabledModemsChanged)
    Q_PROPERTY(QString defaultDataModem READ defaultDataModem NOTIFY defaultDataModemChanged)
    Q_PROPERTY(QString defaultVoiceModem READ defaultVoiceModem NOTIFY defaultVoiceModemChanged)
    Q_PROPERTY(QString defaultDataSim READ defaultDataSim WRITE setDefaultDataSim NOTIFY defaultDataSimChanged)
    Q_PROPERTY(QString defaultVoiceSim READ defaultVoiceSim WRITE setDefaultVoiceSim NOTIFY defaultVoiceSimChanged)
    Q_PROPERTY(QList<bool> presentSims READ presentSims NOTIFY presentSimsChanged)
    Q_PROPERTY(int presentSimCount READ presentSimCount NOTIFY presentSimCountChanged)
    Q_PROPERTY(int activeSimCount READ activeSimCount NOTIFY activeSimCountChanged)

public:
    explicit QOfonoExtModemManager(QObject* aParent = NULL);
    ~QOfonoExtModemManager();

    bool valid() const;
    QStringList availableModems() const;
    QStringList enabledModems() const;
    QString defaultDataModem() const;
    QString defaultVoiceModem() const;
    QString defaultDataSim() const;
    QString defaultVoiceSim() const;
    QList<bool> presentSims() const;
    int presentSimCount() const;
    int activeSimCount() const;

    bool simPresentAt(int aIndex) const;

    void setEnabledModems(QStringList aModems);
    void setDefaultDataSim(QString aImsi);
    void setDefaultVoiceSim(QString aImsi);

    static QSharedPointer<QOfonoExtModemManager> instance();

Q_SIGNALS:
    void validChanged(bool value);
    void availableModemsChanged(QStringList value);
    void enabledModemsChanged(QStringList value);
    void defaultVoiceModemChanged(QString value);
    void defaultDataModemChanged(QString value);
    void defaultVoiceSimChanged(QString value);
    void defaultDataSimChanged(QString value);
    void presentSimsChanged(QList<bool> value);
    void presentSimChanged(int index, bool present);
    void presentSimCountChanged(int value);
    void activeSimCountChanged(int value);

private:
    class Private;
    Private* iPrivate;
};

#endif // QOFONOEXTMODEMMANAGER_H
