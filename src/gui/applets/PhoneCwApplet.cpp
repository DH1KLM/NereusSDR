#include "PhoneCwApplet.h"
#include "NyiOverlay.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QStackedWidget>
#include <QWidget>

namespace NereusSDR {

PhoneCwApplet::PhoneCwApplet(RadioModel* model, QWidget* parent)
    : AppletWidget(model, parent)
{
    buildUI();
}

void PhoneCwApplet::buildUI()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(4, 2, 4, 2);
    root->setSpacing(2);

    // Tab button row: Phone / CW
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        m_phoneTab = new QPushButton(QStringLiteral("Phone"), this);
        m_phoneTab->setCheckable(true);
        m_phoneTab->setChecked(true);
        m_phoneTab->setFixedHeight(20);
        row->addWidget(m_phoneTab);

        m_cwTab = new QPushButton(QStringLiteral("CW"), this);
        m_cwTab->setCheckable(true);
        m_cwTab->setFixedHeight(20);
        row->addWidget(m_cwTab);

        root->addLayout(row);
    }

    m_stack = new QStackedWidget(this);
    root->addWidget(m_stack);

    // Phone page
    {
        QWidget* phonePage = new QWidget(m_stack);
        QVBoxLayout* pageLayout = new QVBoxLayout(phonePage);
        pageLayout->setContentsMargins(0, 2, 0, 0);
        pageLayout->setSpacing(2);

        // MIC row
        {
            QHBoxLayout* row = new QHBoxLayout();
            row->setSpacing(4);

            QLabel* lbl = new QLabel(QStringLiteral("MIC"), phonePage);
            lbl->setFixedWidth(36);
            lbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
            row->addWidget(lbl);

            QSlider* micSlider = new QSlider(Qt::Horizontal, phonePage);
            micSlider->setRange(0, 100);
            micSlider->setFixedHeight(18);
            NyiOverlay::markNyi(micSlider, QStringLiteral("Phase 3I-1"));
            row->addWidget(micSlider, 1);

            pageLayout->addLayout(row);
        }

        // PROC / MON / VOX row
        {
            QHBoxLayout* row = new QHBoxLayout();
            row->setSpacing(4);

            QPushButton* proc = new QPushButton(QStringLiteral("PROC"), phonePage);
            proc->setCheckable(true);
            proc->setFixedHeight(20);
            NyiOverlay::markNyi(proc, QStringLiteral("Phase 3I-1"));
            row->addWidget(proc);

            QPushButton* mon = new QPushButton(QStringLiteral("MON"), phonePage);
            mon->setCheckable(true);
            mon->setFixedHeight(20);
            NyiOverlay::markNyi(mon, QStringLiteral("Phase 3I-1"));
            row->addWidget(mon);

            QPushButton* vox = new QPushButton(QStringLiteral("VOX"), phonePage);
            vox->setCheckable(true);
            vox->setFixedHeight(20);
            NyiOverlay::markNyi(vox, QStringLiteral("Phase 3I-1"));
            row->addWidget(vox);

            pageLayout->addLayout(row);
        }

        pageLayout->addStretch();
        m_stack->addWidget(phonePage);
    }

    // CW page
    {
        QWidget* cwPage = new QWidget(m_stack);
        QVBoxLayout* pageLayout = new QVBoxLayout(cwPage);
        pageLayout->setContentsMargins(0, 2, 0, 0);
        pageLayout->setSpacing(2);

        // WPM row
        {
            QHBoxLayout* row = new QHBoxLayout();
            row->setSpacing(4);

            QLabel* lbl = new QLabel(QStringLiteral("WPM"), cwPage);
            lbl->setFixedWidth(36);
            lbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
            row->addWidget(lbl);

            QSlider* wpmSlider = new QSlider(Qt::Horizontal, cwPage);
            wpmSlider->setRange(1, 60);
            wpmSlider->setValue(20);
            wpmSlider->setFixedHeight(18);
            NyiOverlay::markNyi(wpmSlider, QStringLiteral("Phase 3I-2"));
            row->addWidget(wpmSlider, 1);

            pageLayout->addLayout(row);
        }

        // QSK / Iambic row
        {
            QHBoxLayout* row = new QHBoxLayout();
            row->setSpacing(4);

            QPushButton* qsk = new QPushButton(QStringLiteral("QSK"), cwPage);
            qsk->setCheckable(true);
            qsk->setFixedHeight(20);
            NyiOverlay::markNyi(qsk, QStringLiteral("Phase 3I-2"));
            row->addWidget(qsk);

            QPushButton* iambic = new QPushButton(QStringLiteral("Iambic"), cwPage);
            iambic->setCheckable(true);
            iambic->setFixedHeight(20);
            NyiOverlay::markNyi(iambic, QStringLiteral("Phase 3I-2"));
            row->addWidget(iambic);

            pageLayout->addLayout(row);
        }

        pageLayout->addStretch();
        m_stack->addWidget(cwPage);
    }

    // Wire tab buttons to stack
    connect(m_phoneTab, &QPushButton::clicked, this, [this]() {
        m_stack->setCurrentIndex(0);
        m_phoneTab->setChecked(true);
        m_cwTab->setChecked(false);
    });
    connect(m_cwTab, &QPushButton::clicked, this, [this]() {
        m_stack->setCurrentIndex(1);
        m_cwTab->setChecked(true);
        m_phoneTab->setChecked(false);
    });

    root->addStretch();
}

void PhoneCwApplet::syncFromModel() { /* NYI */ }

} // namespace NereusSDR
