#include "FmApplet.h"
#include "NyiOverlay.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QRadioButton>
#include <QDoubleSpinBox>
#include <QButtonGroup>

namespace NereusSDR {

FmApplet::FmApplet(RadioModel* model, QWidget* parent)
    : AppletWidget(model, parent)
{
    buildUI();
}

void FmApplet::buildUI()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(4, 2, 4, 2);
    root->setSpacing(2);

    // CTCSS row: enable button + tone combo
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QPushButton* ctcssBtn = new QPushButton(QStringLiteral("CTCSS"), this);
        ctcssBtn->setCheckable(true);
        ctcssBtn->setFixedHeight(20);
        NyiOverlay::markNyi(ctcssBtn, QStringLiteral("Phase 3I-3"));
        row->addWidget(ctcssBtn);

        QComboBox* toneCombo = new QComboBox(this);
        toneCombo->setFixedHeight(22);
        toneCombo->addItem(QStringLiteral("67.0 Hz"));
        toneCombo->addItem(QStringLiteral("71.9 Hz"));
        toneCombo->addItem(QStringLiteral("77.0 Hz"));
        toneCombo->addItem(QStringLiteral("100.0 Hz"));
        toneCombo->addItem(QStringLiteral("131.8 Hz"));
        toneCombo->addItem(QStringLiteral("162.2 Hz"));
        NyiOverlay::markNyi(toneCombo, QStringLiteral("Phase 3I-3"));
        row->addWidget(toneCombo, 1);

        root->addLayout(row);
    }

    // Deviation radio buttons: 5.0k / 2.5k
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QLabel* lbl = new QLabel(QStringLiteral("Dev"), this);
        lbl->setFixedWidth(36);
        lbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        row->addWidget(lbl);

        QButtonGroup* devGroup = new QButtonGroup(this);

        QRadioButton* dev5k = new QRadioButton(QStringLiteral("5.0k"), this);
        dev5k->setChecked(true);
        NyiOverlay::markNyi(dev5k, QStringLiteral("Phase 3I-3"));
        devGroup->addButton(dev5k);
        row->addWidget(dev5k);

        QRadioButton* dev25k = new QRadioButton(QStringLiteral("2.5k"), this);
        NyiOverlay::markNyi(dev25k, QStringLiteral("Phase 3I-3"));
        devGroup->addButton(dev25k);
        row->addWidget(dev25k);

        root->addLayout(row);
    }

    // Offset spinner row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QLabel* lbl = new QLabel(QStringLiteral("Offset"), this);
        lbl->setFixedWidth(36);
        lbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        row->addWidget(lbl);

        QDoubleSpinBox* offsetSpin = new QDoubleSpinBox(this);
        offsetSpin->setRange(-10.0, 10.0);
        offsetSpin->setValue(0.0);
        offsetSpin->setSuffix(QStringLiteral(" MHz"));
        offsetSpin->setDecimals(3);
        offsetSpin->setFixedHeight(22);
        NyiOverlay::markNyi(offsetSpin, QStringLiteral("Phase 3I-3"));
        row->addWidget(offsetSpin, 1);

        root->addLayout(row);
    }

    // Simplex toggle row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QPushButton* simplexBtn = new QPushButton(QStringLiteral("Simplex"), this);
        simplexBtn->setCheckable(true);
        simplexBtn->setFixedHeight(20);
        NyiOverlay::markNyi(simplexBtn, QStringLiteral("Phase 3I-3"));
        row->addWidget(simplexBtn);
        row->addStretch();

        root->addLayout(row);
    }

    root->addStretch();
}

void FmApplet::syncFromModel() { /* NYI */ }

} // namespace NereusSDR
