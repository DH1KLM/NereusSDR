#include "DigitalApplet.h"
#include "NyiOverlay.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>

namespace NereusSDR {

DigitalApplet::DigitalApplet(RadioModel* model, QWidget* parent)
    : AppletWidget(model, parent)
{
    buildUI();
}

void DigitalApplet::buildUI()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(4, 2, 4, 2);
    root->setSpacing(2);

    // VAC 1 row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QPushButton* vac1Btn = new QPushButton(QStringLiteral("VAC 1"), this);
        vac1Btn->setCheckable(true);
        vac1Btn->setFixedHeight(20);
        NyiOverlay::markNyi(vac1Btn, QStringLiteral("Phase 3-DAX"));
        row->addWidget(vac1Btn);

        QComboBox* vac1Dev = new QComboBox(this);
        vac1Dev->setFixedHeight(22);
        vac1Dev->setPlaceholderText(QStringLiteral("Device..."));
        NyiOverlay::markNyi(vac1Dev, QStringLiteral("Phase 3-DAX"));
        row->addWidget(vac1Dev, 1);

        root->addLayout(row);
    }

    // VAC 2 row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QPushButton* vac2Btn = new QPushButton(QStringLiteral("VAC 2"), this);
        vac2Btn->setCheckable(true);
        vac2Btn->setFixedHeight(20);
        NyiOverlay::markNyi(vac2Btn, QStringLiteral("Phase 3-DAX"));
        row->addWidget(vac2Btn);

        QComboBox* vac2Dev = new QComboBox(this);
        vac2Dev->setFixedHeight(22);
        vac2Dev->setPlaceholderText(QStringLiteral("Device..."));
        NyiOverlay::markNyi(vac2Dev, QStringLiteral("Phase 3-DAX"));
        row->addWidget(vac2Dev, 1);

        root->addLayout(row);
    }

    // Sample rate row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QLabel* lbl = new QLabel(QStringLiteral("Rate"), this);
        lbl->setFixedWidth(36);
        lbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        row->addWidget(lbl);

        QComboBox* rateCombo = new QComboBox(this);
        rateCombo->setFixedHeight(22);
        rateCombo->addItem(QStringLiteral("48000"));
        rateCombo->addItem(QStringLiteral("96000"));
        rateCombo->addItem(QStringLiteral("192000"));
        NyiOverlay::markNyi(rateCombo, QStringLiteral("Phase 3-DAX"));
        row->addWidget(rateCombo, 1);

        root->addLayout(row);
    }

    // Stereo/Mono toggle row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QPushButton* stereoBtn = new QPushButton(QStringLiteral("Stereo"), this);
        stereoBtn->setCheckable(true);
        stereoBtn->setChecked(true);
        stereoBtn->setFixedHeight(20);
        NyiOverlay::markNyi(stereoBtn, QStringLiteral("Phase 3-DAX"));
        row->addWidget(stereoBtn);
        row->addStretch();

        root->addLayout(row);
    }

    root->addStretch();
}

void DigitalApplet::syncFromModel() { /* NYI */ }

} // namespace NereusSDR
