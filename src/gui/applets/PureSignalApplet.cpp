#include "PureSignalApplet.h"
#include "NyiOverlay.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

namespace NereusSDR {

PureSignalApplet::PureSignalApplet(RadioModel* model, QWidget* parent)
    : AppletWidget(model, parent)
{
    buildUI();
}

void PureSignalApplet::buildUI()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(4, 2, 4, 2);
    root->setSpacing(2);

    // Calibrate button row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QPushButton* calBtn = new QPushButton(QStringLiteral("Calibrate"), this);
        calBtn->setFixedHeight(20);
        NyiOverlay::markNyi(calBtn, QStringLiteral("Phase 3I-4"));
        row->addWidget(calBtn);
        row->addStretch();

        root->addLayout(row);
    }

    // Auto-cal toggle row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QPushButton* autoCalBtn = new QPushButton(QStringLiteral("Auto-cal"), this);
        autoCalBtn->setCheckable(true);
        autoCalBtn->setFixedHeight(20);
        NyiOverlay::markNyi(autoCalBtn, QStringLiteral("Phase 3I-4"));
        row->addWidget(autoCalBtn);
        row->addStretch();

        root->addLayout(row);
    }

    // Feedback level label row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QLabel* lbl = new QLabel(QStringLiteral("Feedback:"), this);
        lbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        row->addWidget(lbl);

        QLabel* fbLevel = new QLabel(QStringLiteral("---"), this);
        fbLevel->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        NyiOverlay::markNyi(fbLevel, QStringLiteral("Phase 3I-4"));
        row->addWidget(fbLevel, 1);

        root->addLayout(row);
    }

    // Status label row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QLabel* statusLbl = new QLabel(QStringLiteral("Status:"), this);
        statusLbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        row->addWidget(statusLbl);

        QLabel* statusVal = new QLabel(QStringLiteral("Idle"), this);
        statusVal->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        NyiOverlay::markNyi(statusVal, QStringLiteral("Phase 3I-4"));
        row->addWidget(statusVal, 1);

        root->addLayout(row);
    }

    // Two-tone test button row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QPushButton* twoToneBtn = new QPushButton(QStringLiteral("Two-Tone Test"), this);
        twoToneBtn->setFixedHeight(20);
        NyiOverlay::markNyi(twoToneBtn, QStringLiteral("Phase 3I-4"));
        row->addWidget(twoToneBtn);
        row->addStretch();

        root->addLayout(row);
    }

    root->addStretch();
}

void PureSignalApplet::syncFromModel() { /* NYI */ }

} // namespace NereusSDR
