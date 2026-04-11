#include "DvkApplet.h"
#include "NyiOverlay.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

namespace NereusSDR {

DvkApplet::DvkApplet(RadioModel* model, QWidget* parent)
    : AppletWidget(model, parent)
{
    buildUI();
}

void DvkApplet::buildUI()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(4, 2, 4, 2);
    root->setSpacing(2);

    // 4 slots, each with Record/Play/Stop buttons
    static const char* kSlotLabels[] = { "Slot 1", "Slot 2", "Slot 3", "Slot 4" };
    for (const char* slotName : kSlotLabels) {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QLabel* lbl = new QLabel(QString::fromLatin1(slotName), this);
        lbl->setFixedWidth(44);
        lbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        row->addWidget(lbl);

        QPushButton* recBtn = new QPushButton(QStringLiteral("Rec"), this);
        recBtn->setFixedHeight(20);
        NyiOverlay::markNyi(recBtn, QStringLiteral("Phase 3I-1"));
        row->addWidget(recBtn);

        QPushButton* playBtn = new QPushButton(QStringLiteral("Play"), this);
        playBtn->setFixedHeight(20);
        NyiOverlay::markNyi(playBtn, QStringLiteral("Phase 3I-1"));
        row->addWidget(playBtn);

        QPushButton* stopBtn = new QPushButton(QStringLiteral("Stop"), this);
        stopBtn->setFixedHeight(20);
        NyiOverlay::markNyi(stopBtn, QStringLiteral("Phase 3I-1"));
        row->addWidget(stopBtn);

        root->addLayout(row);
    }

    // Repeat toggle + WAV import row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QPushButton* repeatBtn = new QPushButton(QStringLiteral("Repeat"), this);
        repeatBtn->setCheckable(true);
        repeatBtn->setFixedHeight(20);
        NyiOverlay::markNyi(repeatBtn, QStringLiteral("Phase 3I-1"));
        row->addWidget(repeatBtn);

        QPushButton* importBtn = new QPushButton(QStringLiteral("Import WAV"), this);
        importBtn->setFixedHeight(20);
        NyiOverlay::markNyi(importBtn, QStringLiteral("Phase 3I-1"));
        row->addWidget(importBtn);

        root->addLayout(row);
    }

    root->addStretch();
}

void DvkApplet::syncFromModel() { /* NYI */ }

} // namespace NereusSDR
