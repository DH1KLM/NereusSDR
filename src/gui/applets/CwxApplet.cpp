#include "CwxApplet.h"
#include "NyiOverlay.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QFontMetrics>

namespace NereusSDR {

CwxApplet::CwxApplet(RadioModel* model, QWidget* parent)
    : AppletWidget(model, parent)
{
    buildUI();
}

void CwxApplet::buildUI()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(4, 2, 4, 2);
    root->setSpacing(2);

    // Text input (3 lines high)
    {
        QTextEdit* textEdit = new QTextEdit(this);
        QFontMetrics fm(textEdit->font());
        textEdit->setFixedHeight(fm.lineSpacing() * 3 + 8);
        NyiOverlay::markNyi(textEdit, QStringLiteral("Phase 3I-2"));
        root->addWidget(textEdit);
    }

    // Send button row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QPushButton* sendBtn = new QPushButton(QStringLiteral("Send"), this);
        sendBtn->setFixedHeight(20);
        NyiOverlay::markNyi(sendBtn, QStringLiteral("Phase 3I-2"));
        row->addWidget(sendBtn);
        row->addStretch();

        root->addLayout(row);
    }

    // 6 memory buttons in a 2x3 grid
    {
        QGridLayout* grid = new QGridLayout();
        grid->setSpacing(4);

        static const char* kMemLabels[] = { "M1", "M2", "M3", "M4", "M5", "M6" };
        for (int i = 0; i < 6; ++i) {
            QPushButton* memBtn = new QPushButton(QString::fromLatin1(kMemLabels[i]), this);
            memBtn->setFixedHeight(20);
            NyiOverlay::markNyi(memBtn, QStringLiteral("Phase 3I-2"));
            grid->addWidget(memBtn, i / 3, i % 3);
        }

        root->addLayout(grid);
    }

    // Repeat toggle row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QPushButton* repeatBtn = new QPushButton(QStringLiteral("Repeat"), this);
        repeatBtn->setCheckable(true);
        repeatBtn->setFixedHeight(20);
        NyiOverlay::markNyi(repeatBtn, QStringLiteral("Phase 3I-2"));
        row->addWidget(repeatBtn);
        row->addStretch();

        root->addLayout(row);
    }

    root->addStretch();
}

void CwxApplet::syncFromModel() { /* NYI */ }

} // namespace NereusSDR
