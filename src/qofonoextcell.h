/****************************************************************************
**
** Copyright (C) 2015-2023 Slava Monich <slava@monich.com>
** Copyright (C) 2015-2021 Jolla Ltd.
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
    Q_PROPERTY(int arfcn READ arfcn NOTIFY arfcnChanged)
    Q_PROPERTY(int bsic READ bsic NOTIFY bsicChanged)
    Q_PROPERTY(int bitErrorRate READ bitErrorRate NOTIFY bitErrorRateChanged)
    Q_PROPERTY(int psc READ psc NOTIFY pscChanged)
    Q_PROPERTY(int uarfcn READ uarfcn NOTIFY uarfcnChanged)
    Q_PROPERTY(int ci READ ci NOTIFY ciChanged)
    Q_PROPERTY(int pci READ pci NOTIFY pciChanged)
    Q_PROPERTY(int tac READ tac NOTIFY tacChanged)
    Q_PROPERTY(int earfcn READ earfcn NOTIFY earfcnChanged)
    Q_PROPERTY(int rsrp READ rsrp NOTIFY rsrpChanged)
    Q_PROPERTY(int rsrq READ rsrq NOTIFY rsrqChanged)
    Q_PROPERTY(int rssnr READ rssnr NOTIFY rssnrChanged)
    Q_PROPERTY(int cqi READ cqi NOTIFY cqiChanged)
    Q_PROPERTY(int timingAdvance READ timingAdvance NOTIFY timingAdvanceChanged)
    Q_PROPERTY(qint64 nci READ nci NOTIFY nciChanged)
    Q_PROPERTY(int nrarfcn READ nrarfcn NOTIFY nrarfcnChanged)
    Q_PROPERTY(int ssRsrp READ ssRsrp NOTIFY ssRsrpChanged)
    Q_PROPERTY(int ssRsrq READ ssRsrq NOTIFY ssRsrqChanged)
    Q_PROPERTY(int ssSinr READ ssSinr NOTIFY ssSinrChanged)
    Q_PROPERTY(int csiRsrp READ csiRsrp NOTIFY csiRsrpChanged)
    Q_PROPERTY(int csiRsrq READ csiRsrq NOTIFY csiRsrqChanged)
    Q_PROPERTY(int csiSinr READ csiSinr NOTIFY csiSinrChanged)
    Q_PROPERTY(int signalLevelDbm READ signalLevelDbm NOTIFY signalLevelDbmChanged)
    Q_ENUMS(Type)
    Q_ENUMS(Constants)

public:
    enum Type {
        Unknown,
        GSM,
        WCDMA,
        LTE,
        NR,
        UNKNOWN = Unknown // For backward compatibility
    };

    enum Constants {
        InvalidValue = INT_MAX,
        InvalidValue64 = LLONG_MAX
    };

    explicit QOfonoExtCell(QObject* aParent = Q_NULLPTR);
    QOfonoExtCell(QString aPath, bool aMayBlock); // Since 1.0.27
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
    int signalLevelDbm() const;

    // GSM:
    int arfcn() const;
    int bsic() const;

    // GSM and WCDMA:
    int lac() const;
    int cid() const;
    int bitErrorRate() const;

    // WCDMA:
    int psc() const;
    int uarfcn() const;

    // LTE:
    int ci() const;
    int earfcn() const;
    int rsrp() const;
    int rsrq() const;
    int rssnr() const;
    int cqi() const;
    int timingAdvance() const;

    // LTE and NR:
    int pci() const;
    int tac() const;

    // NR:
    qint64 nci() const;
    int nrarfcn() const;
    int ssRsrp() const;
    int ssRsrq() const;
    int ssSinr() const;
    int csiRsrp() const;
    int csiRsrq() const;
    int csiSinr() const;

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
    void arfcnChanged();
    void bsicChanged();
    void bitErrorRateChanged();
    void pscChanged();
    void uarfcnChanged();
    void ciChanged();
    void pciChanged();
    void tacChanged();
    void earfcnChanged();
    void rsrpChanged();
    void rsrqChanged();
    void rssnrChanged();
    void cqiChanged();
    void timingAdvanceChanged();
    void nciChanged();
    void nrarfcnChanged();
    void ssRsrpChanged();
    void ssRsrqChanged();
    void ssSinrChanged();
    void csiRsrpChanged();
    void csiRsrqChanged();
    void csiSinrChanged();
    void signalLevelDbmChanged();
    void propertyChanged(QString name, int value); // int properties
    void propertyChanged64(QString name, qint64 value); // 64-bit properties
    void removed();

private:
    class Private;
    Private* iPrivate;
};

#endif // QOFONOEXTCELL_H
