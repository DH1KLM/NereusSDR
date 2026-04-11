#include "RxApplet.h"
#include "NyiOverlay.h"
#include "models/RadioModel.h"
#include "models/SliceModel.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

namespace NereusSDR {

RxApplet::RxApplet(RadioModel* model, QWidget* parent)
    : AppletWidget(model, parent)
{
    buildUI();
}

void RxApplet::buildUI()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(4, 2, 4, 2);
    root->setSpacing(2);

    // ---- Row 1: Slice badge | Lock button | Filter label ----
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        m_sliceBadge = new QLabel(QStringLiteral("A"), this);
        m_sliceBadge->setFixedSize(22, 18);
        m_sliceBadge->setAlignment(Qt::AlignCenter);
        m_sliceBadge->setStyleSheet(QStringLiteral(
            "background: #00d4ff; color: black;"
            "font-size: 10px; font-weight: bold;"
            "border-radius: 3px;"));
        row->addWidget(m_sliceBadge);

        m_lockBtn = new QPushButton(QStringLiteral("Lock"), this);
        m_lockBtn->setCheckable(true);
        row->addWidget(m_lockBtn);

        m_filterLabel = new QLabel(QStringLiteral("3.0 kHz"), this);
        m_filterLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_filterLabel->setStyleSheet(QStringLiteral(
            "color: #8090a0; font-size: 10px;"));
        row->addWidget(m_filterLabel, 1);

        root->addLayout(row);
    }

    // ---- Row 2: Mode label + mode combo ----
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QLabel* lbl = new QLabel(QStringLiteral("Mode"), this);
        lbl->setFixedWidth(36);
        lbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        row->addWidget(lbl);

        // Items ordered to match DSPMode enum integer values:
        // LSB=0, USB=1, DSB=2, CWL=3, CWU=4, FM=5, AM=6,
        // DIGU=7, SPEC=8, DIGL=9, SAM=10, DRM=11
        m_modeCombo = new QComboBox(this);
        m_modeCombo->addItems({
            QStringLiteral("LSB"),
            QStringLiteral("USB"),
            QStringLiteral("DSB"),
            QStringLiteral("CWL"),
            QStringLiteral("CWU"),
            QStringLiteral("FM"),
            QStringLiteral("AM"),
            QStringLiteral("DIGU"),
            QStringLiteral("SPEC"),
            QStringLiteral("DIGL"),
            QStringLiteral("SAM"),
            QStringLiteral("DRM")
        });
        row->addWidget(m_modeCombo, 1);

        root->addLayout(row);
    }

    // ---- Row 3: AGC label + AGC combo + AGC threshold slider ----
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QLabel* lbl = new QLabel(QStringLiteral("AGC"), this);
        lbl->setFixedWidth(36);
        lbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        row->addWidget(lbl);

        // Items ordered to match AGCMode enum: Off=0, Long=1, Slow=2, Med=3, Fast=4, Custom=5
        m_agcCombo = new QComboBox(this);
        m_agcCombo->setFixedWidth(60);
        m_agcCombo->addItems({
            QStringLiteral("Off"),
            QStringLiteral("Long"),
            QStringLiteral("Slow"),
            QStringLiteral("Med"),
            QStringLiteral("Fast"),
            QStringLiteral("Custom")
        });
        row->addWidget(m_agcCombo);

        m_agcThreshold = new QSlider(Qt::Horizontal, this);
        m_agcThreshold->setRange(-140, 0);
        m_agcThreshold->setFixedHeight(18);
        row->addWidget(m_agcThreshold, 1);

        root->addLayout(row);
    }

    // ---- Row 4: AF label + AF gain slider + Mute button ----
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QLabel* lbl = new QLabel(QStringLiteral("AF"), this);
        lbl->setFixedWidth(36);
        lbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        row->addWidget(lbl);

        m_afGain = new QSlider(Qt::Horizontal, this);
        m_afGain->setRange(0, 100);
        m_afGain->setFixedHeight(18);
        row->addWidget(m_afGain, 1);

        m_muteBtn = new QPushButton(QStringLiteral("Mute"), this);
        m_muteBtn->setCheckable(true);
        m_muteBtn->setFixedSize(40, 18);
        row->addWidget(m_muteBtn);

        root->addLayout(row);
    }

    // ---- Row 5: SQL button + squelch slider (NYI) ----
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        m_squelchBtn = new QPushButton(QStringLiteral("SQL"), this);
        m_squelchBtn->setCheckable(true);
        m_squelchBtn->setFixedSize(36, 18);
        NyiOverlay::markNyi(m_squelchBtn, QStringLiteral("Phase 3-UI Tier 2"));
        row->addWidget(m_squelchBtn);

        m_squelchSlider = new QSlider(Qt::Horizontal, this);
        m_squelchSlider->setRange(0, 160);
        m_squelchSlider->setFixedHeight(18);
        NyiOverlay::markNyi(m_squelchSlider, QStringLiteral("Phase 3-UI Tier 2"));
        row->addWidget(m_squelchSlider, 1);

        root->addLayout(row);
    }

    // ---- Row 6: Pan label + pan slider (NYI) ----
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QLabel* lbl = new QLabel(QStringLiteral("Pan"), this);
        lbl->setFixedWidth(36);
        lbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        row->addWidget(lbl);

        m_panSlider = new QSlider(Qt::Horizontal, this);
        m_panSlider->setRange(-100, 100);
        m_panSlider->setFixedHeight(18);
        NyiOverlay::markNyi(m_panSlider, QStringLiteral("Phase 3-UI Tier 2"));
        row->addWidget(m_panSlider, 1);

        root->addLayout(row);
    }

    root->addStretch();
}

void RxApplet::setSlice(SliceModel* slice)
{
    if (m_slice) {
        disconnect(m_slice, nullptr, this, nullptr);
    }
    m_slice = slice;
    wireSlice();
    syncFromModel();
}

void RxApplet::wireSlice()
{
    if (!m_slice) {
        return;
    }

    // Model → UI connections
    connect(m_slice, &SliceModel::dspModeChanged, this, [this](DSPMode mode) {
        m_updatingFromModel = true;
        m_modeCombo->setCurrentIndex(static_cast<int>(mode));
        m_updatingFromModel = false;
    });

    connect(m_slice, &SliceModel::agcModeChanged, this, [this](AGCMode mode) {
        m_updatingFromModel = true;
        m_agcCombo->setCurrentIndex(static_cast<int>(mode));
        m_updatingFromModel = false;
    });

    connect(m_slice, &SliceModel::afGainChanged, this, [this](int gain) {
        m_updatingFromModel = true;
        m_afGain->setValue(gain);
        m_updatingFromModel = false;
    });

    connect(m_slice, &SliceModel::rfGainChanged, this, [this](int gain) {
        m_updatingFromModel = true;
        m_agcThreshold->setValue(gain);
        m_updatingFromModel = false;
    });

    connect(m_slice, &SliceModel::filterChanged, this, [this](int low, int high) {
        int width = high - low;
        QString text;
        if (width >= 1000) {
            text = QString::number(width / 1000.0, 'f', 1) + QStringLiteral(" kHz");
        } else {
            text = QString::number(width) + QStringLiteral(" Hz");
        }
        m_filterLabel->setText(text);
    });

    // UI → Model connections
    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
        if (m_updatingFromModel || !m_slice) { return; }
        m_slice->setDspMode(static_cast<DSPMode>(idx));
    });

    connect(m_agcCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
        if (m_updatingFromModel || !m_slice) { return; }
        m_slice->setAgcMode(static_cast<AGCMode>(idx));
    });

    connect(m_afGain, &QSlider::valueChanged, this, [this](int val) {
        if (m_updatingFromModel || !m_slice) { return; }
        m_slice->setAfGain(val);
    });

    connect(m_agcThreshold, &QSlider::valueChanged, this, [this](int val) {
        if (m_updatingFromModel || !m_slice) { return; }
        m_slice->setRfGain(val);
    });
}

void RxApplet::syncFromModel()
{
    if (!m_slice) {
        return;
    }

    m_updatingFromModel = true;

    m_modeCombo->setCurrentIndex(static_cast<int>(m_slice->dspMode()));
    m_agcCombo->setCurrentIndex(static_cast<int>(m_slice->agcMode()));
    m_afGain->setValue(m_slice->afGain());
    m_agcThreshold->setValue(m_slice->rfGain());

    int width = m_slice->filterHigh() - m_slice->filterLow();
    QString filterText;
    if (width >= 1000) {
        filterText = QString::number(width / 1000.0, 'f', 1) + QStringLiteral(" kHz");
    } else {
        filterText = QString::number(width) + QStringLiteral(" Hz");
    }
    m_filterLabel->setText(filterText);

    m_updatingFromModel = false;
}

} // namespace NereusSDR
