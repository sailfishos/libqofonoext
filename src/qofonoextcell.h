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

#ifndef QOFONOEXTCELL_H
#define QOFONOEXTCELL_H

#include "qofonoext_types.h"

class QOFONOEXT_EXPORT QOfonoExtCell : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)
    Q_PROPERTY(Type type READ type NOTIFY typeChanged)
    Q_PROPERTY(bool registered READ registered NOTIFY registeredChanged)
    Q_PROPERTY(int mcc READ mcc NOTIFY mccChanged)
    Q_PROPERTY(int mnc READ mnc NOTIFY mncChanged)
    Q_PROPERTY(int signalStrength READ signalStrength NOTIFY signalStrengthChanged)
    Q_PROPERTY(int lac READ lac NOTIFY lacChanged)
    Q_PROPERTY(int cid READ cid NOTIFY cidChanged)
    Q_PROPERTY(int bitErrorRate READ bitErrorRate NOTIFY bitErrorRateChanged)
    Q_PROPERTY(int psc READ psc NOTIFY pscChanged)
    Q_PROPERTY(int ci READ ci NOTIFY ciChanged)
    Q_PROPERTY(int pci READ pci NOTIFY pciChanged)
    Q_PROPERTY(int tac READ tac NOTIFY tacChanged)
    Q_PROPERTY(int rsrp READ rsrp NOTIFY rsrpChanged)
    Q_PROPERTY(int rsrq READ rsrq NOTIFY rsrqChanged)
    Q_PROPERTY(int rssnr READ rssnr NOTIFY rssnrChanged)
    Q_PROPERTY(int cqi READ cqi NOTIFY cqiChanged)
    Q_PROPERTY(int timingAdvance READ timingAdvance NOTIFY timingAdvanceChanged)
    Q_ENUMS(Type)

public:
    enum Type {
        UNKNOWN,
        GSM,
        WCDMA,
        LTE
    };

    explicit QOfonoExtCell(QObject* aParent = NULL);
    QOfonoExtCell(QString aPath);
    ~QOfonoExtCell();

    QString path() const;
    void setPath(QString aPath);

    bool valid() const;
    Type type() const;
    bool registered() const;

    // All types:
    int mcc() const;
    int mnc() const;
    int signalStrength() const;

    // GSM and WCDMA:
    int lac() const;
    int cid() const;
    int bitErrorRate() const;

    // WCDMA:
    int psc() const;

    // LTE:
    int ci() const;
    int pci() const;
    int tac() const;
    int rsrp() const;
    int rsrq() const;
    int rssnr() const;
    int cqi() const;
    int timingAdvance() const;

Q_SIGNALS:
    void validChanged();
    void pathChanged();
    void typeChanged();
    void registeredChanged();
    void mccChanged();
    void mncChanged();
    void signalStrengthChanged();
    void lacChanged();
    void cidChanged();
    void bitErrorRateChanged();
    void pscChanged();
    void ciChanged();
    void pciChanged();
    void tacChanged();
    void rsrpChanged();
    void rsrqChanged();
    void rssnrChanged();
    void cqiChanged();
    void timingAdvanceChanged();
    void propertyChanged(QString name, int value);
    void removed();

private:
    class Private;
    Private* iPrivate;
};

#endif // QOFONOEXTCELL_H
