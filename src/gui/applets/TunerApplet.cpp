#include "TunerApplet.h"
#include "NyiOverlay.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

namespace NereusSDR {

TunerApplet::TunerApplet(RadioModel* model, QWidget* parent)
    : AppletWidget(model, parent)
{
    buildUI();
}

void TunerApplet::buildUI()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(4, 2, 4, 2);
    root->setSpacing(2);

    // TUNE button row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QPushButton* tuneBtn = new QPushButton(QStringLiteral("TUNE"), this);
        tuneBtn->setFixedHeight(22);
        NyiOverlay::markNyi(tuneBtn, QStringLiteral("Aries ATU"));
        row->addWidget(tuneBtn);
        row->addStretch();

        root->addLayout(row);
    }

    // SWR label row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QLabel* swrLbl = new QLabel(QStringLiteral("SWR:"), this);
        swrLbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        row->addWidget(swrLbl);

        QLabel* swrVal = new QLabel(QStringLiteral("---"), this);
        swrVal->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        NyiOverlay::markNyi(swrVal, QStringLiteral("Aries ATU"));
        row->addWidget(swrVal, 1);

        root->addLayout(row);
    }

    // Operate / Bypass / Standby row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QPushButton* operateBtn = new QPushButton(QStringLiteral("Operate"), this);
        operateBtn->setCheckable(true);
        operateBtn->setFixedHeight(20);
        NyiOverlay::markNyi(operateBtn, QStringLiteral("Aries ATU"));
        row->addWidget(operateBtn);

        QPushButton* bypassBtn = new QPushButton(QStringLiteral("Bypass"), this);
        bypassBtn->setCheckable(true);
        bypassBtn->setFixedHeight(20);
        NyiOverlay::markNyi(bypassBtn, QStringLiteral("Aries ATU"));
        row->addWidget(bypassBtn);

        QPushButton* standbyBtn = new QPushButton(QStringLiteral("Standby"), this);
        standbyBtn->setCheckable(true);
        standbyBtn->setFixedHeight(20);
        NyiOverlay::markNyi(standbyBtn, QStringLiteral("Aries ATU"));
        row->addWidget(standbyBtn);

        root->addLayout(row);
    }

    // Antenna buttons row: 1 / 2 / 3
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QPushButton* ant1 = new QPushButton(QStringLiteral("1"), this);
        ant1->setCheckable(true);
        ant1->setFixedHeight(20);
        NyiOverlay::markNyi(ant1, QStringLiteral("Aries ATU"));
        row->addWidget(ant1);

        QPushButton* ant2 = new QPushButton(QStringLiteral("2"), this);
        ant2->setCheckable(true);
        ant2->setFixedHeight(20);
        NyiOverlay::markNyi(ant2, QStringLiteral("Aries ATU"));
        row->addWidget(ant2);

        QPushButton* ant3 = new QPushButton(QStringLiteral("3"), this);
        ant3->setCheckable(true);
        ant3->setFixedHeight(20);
        NyiOverlay::markNyi(ant3, QStringLiteral("Aries ATU"));
        row->addWidget(ant3);

        root->addLayout(row);
    }

    root->addStretch();
}

void TunerApplet::syncFromModel() { /* NYI */ }

} // namespace NereusSDR
