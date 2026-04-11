#include "EqApplet.h"
#include "NyiOverlay.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QLabel>

namespace NereusSDR {

EqApplet::EqApplet(RadioModel* model, QWidget* parent)
    : AppletWidget(model, parent)
{
    buildUI();
}

void EqApplet::buildUI()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(4, 2, 4, 2);
    root->setSpacing(2);

    // Top row: RX / TX / ON
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QPushButton* rxBtn = new QPushButton(QStringLiteral("RX"), this);
        rxBtn->setCheckable(true);
        rxBtn->setFixedHeight(20);
        NyiOverlay::markNyi(rxBtn, QStringLiteral("Phase 3I-3"));
        row->addWidget(rxBtn);

        QPushButton* txBtn = new QPushButton(QStringLiteral("TX"), this);
        txBtn->setCheckable(true);
        txBtn->setFixedHeight(20);
        NyiOverlay::markNyi(txBtn, QStringLiteral("Phase 3I-3"));
        row->addWidget(txBtn);

        QPushButton* onBtn = new QPushButton(QStringLiteral("ON"), this);
        onBtn->setCheckable(true);
        onBtn->setFixedHeight(20);
        NyiOverlay::markNyi(onBtn, QStringLiteral("Phase 3I-3"));
        row->addWidget(onBtn);

        root->addLayout(row);
    }

    // 10 EQ band sliders in a horizontal row
    {
        static const char* kBands[] = {
            "32", "63", "125", "250", "500", "1k", "2k", "4k", "8k", "16k"
        };

        QHBoxLayout* sliderRow = new QHBoxLayout();
        sliderRow->setSpacing(2);

        for (const char* band : kBands) {
            QVBoxLayout* col = new QVBoxLayout();
            col->setSpacing(1);
            col->setAlignment(Qt::AlignHCenter);

            QSlider* sl = new QSlider(Qt::Vertical, this);
            sl->setRange(-12, 12);
            sl->setValue(0);
            sl->setFixedWidth(16);
            NyiOverlay::markNyi(sl, QStringLiteral("Phase 3I-3"));
            col->addWidget(sl, 1);

            QLabel* lbl = new QLabel(QString::fromLatin1(band), this);
            lbl->setAlignment(Qt::AlignCenter);
            lbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 8px;"));
            col->addWidget(lbl);

            sliderRow->addLayout(col);
        }

        root->addLayout(sliderRow);
    }

    // Bottom row: preset combo + Reset button
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QComboBox* presetCombo = new QComboBox(this);
        presetCombo->setFixedHeight(22);
        presetCombo->addItem(QStringLiteral("Flat"));
        presetCombo->addItem(QStringLiteral("Bass Boost"));
        presetCombo->addItem(QStringLiteral("Treble Boost"));
        presetCombo->addItem(QStringLiteral("Custom"));
        NyiOverlay::markNyi(presetCombo, QStringLiteral("Phase 3I-3"));
        row->addWidget(presetCombo, 1);

        QPushButton* resetBtn = new QPushButton(QStringLiteral("Reset"), this);
        resetBtn->setFixedHeight(22);
        NyiOverlay::markNyi(resetBtn, QStringLiteral("Phase 3I-3"));
        row->addWidget(resetBtn);

        root->addLayout(row);
    }

    root->addStretch();
}

void EqApplet::syncFromModel() { /* NYI */ }

} // namespace NereusSDR
