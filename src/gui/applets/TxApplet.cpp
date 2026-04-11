#include "TxApplet.h"
#include "NyiOverlay.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QLabel>

namespace NereusSDR {

TxApplet::TxApplet(RadioModel* model, QWidget* parent)
    : AppletWidget(model, parent)
{
    buildUI();
}

void TxApplet::buildUI()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(4, 2, 4, 2);
    root->setSpacing(2);

    // Title label
    {
        QLabel* title = new QLabel(QStringLiteral("TX Controls"), this);
        title->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        root->addWidget(title);
    }

    // Button row: MOX / TUNE / ATU
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QPushButton* mox = new QPushButton(QStringLiteral("MOX"), this);
        mox->setCheckable(true);
        mox->setFixedHeight(20);
        NyiOverlay::markNyi(mox, QStringLiteral("Phase 3I-1"));
        row->addWidget(mox);

        QPushButton* tune = new QPushButton(QStringLiteral("TUNE"), this);
        tune->setCheckable(true);
        tune->setFixedHeight(20);
        NyiOverlay::markNyi(tune, QStringLiteral("Phase 3I-1"));
        row->addWidget(tune);

        QPushButton* atu = new QPushButton(QStringLiteral("ATU"), this);
        atu->setCheckable(true);
        atu->setFixedHeight(20);
        NyiOverlay::markNyi(atu, QStringLiteral("Phase 3I-1"));
        row->addWidget(atu);

        root->addLayout(row);
    }

    // RF Power row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QLabel* lbl = new QLabel(QStringLiteral("PWR"), this);
        lbl->setFixedWidth(36);
        lbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        row->addWidget(lbl);

        QSlider* rfPwr = new QSlider(Qt::Horizontal, this);
        rfPwr->setRange(0, 100);
        rfPwr->setFixedHeight(18);
        NyiOverlay::markNyi(rfPwr, QStringLiteral("Phase 3I-1"));
        row->addWidget(rfPwr, 1);

        QLabel* pwrVal = new QLabel(QStringLiteral("100"), this);
        pwrVal->setFixedWidth(28);
        pwrVal->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        NyiOverlay::markNyi(pwrVal, QStringLiteral("Phase 3I-1"));
        row->addWidget(pwrVal);

        root->addLayout(row);
    }

    // Tune Power row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QLabel* lbl = new QLabel(QStringLiteral("TUN"), this);
        lbl->setFixedWidth(36);
        lbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        row->addWidget(lbl);

        QSlider* tunPwr = new QSlider(Qt::Horizontal, this);
        tunPwr->setRange(0, 100);
        tunPwr->setFixedHeight(18);
        NyiOverlay::markNyi(tunPwr, QStringLiteral("Phase 3I-1"));
        row->addWidget(tunPwr, 1);

        root->addLayout(row);
    }

    // TX Profile row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QLabel* lbl = new QLabel(QStringLiteral("Profile"), this);
        lbl->setFixedWidth(36);
        lbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        row->addWidget(lbl);

        QComboBox* profileCombo = new QComboBox(this);
        profileCombo->setFixedHeight(22);
        profileCombo->addItem(QStringLiteral("Default"));
        NyiOverlay::markNyi(profileCombo, QStringLiteral("Phase 3I-1"));
        row->addWidget(profileCombo, 1);

        root->addLayout(row);
    }

    root->addStretch();
}

void TxApplet::syncFromModel() { /* NYI */ }

} // namespace NereusSDR
