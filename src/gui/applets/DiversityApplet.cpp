#include "DiversityApplet.h"
#include "NyiOverlay.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QLabel>

namespace NereusSDR {

DiversityApplet::DiversityApplet(RadioModel* model, QWidget* parent)
    : AppletWidget(model, parent)
{
    buildUI();
}

void DiversityApplet::buildUI()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(4, 2, 4, 2);
    root->setSpacing(2);

    // Enable toggle row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QPushButton* enableBtn = new QPushButton(QStringLiteral("Enable"), this);
        enableBtn->setCheckable(true);
        enableBtn->setFixedHeight(20);
        NyiOverlay::markNyi(enableBtn, QStringLiteral("Phase 3F"));
        row->addWidget(enableBtn);
        row->addStretch();

        root->addLayout(row);
    }

    // Source combo row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QLabel* lbl = new QLabel(QStringLiteral("Source"), this);
        lbl->setFixedWidth(36);
        lbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        row->addWidget(lbl);

        QComboBox* sourceCombo = new QComboBox(this);
        sourceCombo->setFixedHeight(22);
        sourceCombo->addItem(QStringLiteral("RX1"));
        sourceCombo->addItem(QStringLiteral("RX2"));
        NyiOverlay::markNyi(sourceCombo, QStringLiteral("Phase 3F"));
        row->addWidget(sourceCombo, 1);

        root->addLayout(row);
    }

    // Gain slider row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QLabel* lbl = new QLabel(QStringLiteral("Gain"), this);
        lbl->setFixedWidth(36);
        lbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        row->addWidget(lbl);

        QSlider* gainSlider = new QSlider(Qt::Horizontal, this);
        gainSlider->setRange(0, 100);
        gainSlider->setFixedHeight(18);
        NyiOverlay::markNyi(gainSlider, QStringLiteral("Phase 3F"));
        row->addWidget(gainSlider, 1);

        root->addLayout(row);
    }

    // Phase slider row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QLabel* lbl = new QLabel(QStringLiteral("Phase"), this);
        lbl->setFixedWidth(36);
        lbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        row->addWidget(lbl);

        QSlider* phaseSlider = new QSlider(Qt::Horizontal, this);
        phaseSlider->setRange(0, 360);
        phaseSlider->setFixedHeight(18);
        NyiOverlay::markNyi(phaseSlider, QStringLiteral("Phase 3F"));
        row->addWidget(phaseSlider, 1);

        root->addLayout(row);
    }

    root->addStretch();
}

void DiversityApplet::syncFromModel() { /* NYI */ }

} // namespace NereusSDR
