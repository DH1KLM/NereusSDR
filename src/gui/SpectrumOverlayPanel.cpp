#include "SpectrumOverlayPanel.h"
#include "models/RadioModel.h"
#include "models/SliceModel.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSlider>
#include <QComboBox>
#include <QLabel>
#include <QEvent>
#include <QMouseEvent>
#include <QWheelEvent>

namespace NereusSDR {

// ---------------------------------------------------------------------------
// Button stylesheet fragments
// ---------------------------------------------------------------------------

static const char* kBtnStyle =
    "QPushButton {"
    "  background: rgba(20, 30, 45, 240);"
    "  color: #c8d8e8;"
    "  border: 1px solid #304050;"
    "  border-radius: 2px;"
    "  font-size: 10px;"
    "  font-weight: bold;"
    "}"
    "QPushButton:hover { background: rgba(0, 112, 192, 180); }"
    "QPushButton:checked { background: rgba(0, 112, 192, 220); }"
    "QPushButton:disabled {"
    "  background: rgba(15, 20, 30, 200);"
    "  color: #506070;"
    "  border: 1px solid #253040;"
    "}";

static const char* kFlyoutStyle =
    "QWidget {"
    "  background: rgba(15, 15, 26, 220);"
    "  border: 1px solid #304050;"
    "}"
    "QLabel { color: #8090a0; font-size: 9px; border: none; background: transparent; }"
    "QComboBox {"
    "  background: #1a2a3a; color: #c8d8e8;"
    "  border: 1px solid #304050; border-radius: 2px;"
    "  font-size: 10px; padding: 1px 4px;"
    "}"
    "QComboBox::drop-down { border: none; }"
    "QSlider::groove:horizontal { background: #1a2a3a; height: 4px; border-radius: 2px; }"
    "QSlider::handle:horizontal { background: #0070c0; width: 10px; margin: -3px 0; border-radius: 5px; }"
    "QPushButton {"
    "  background: rgba(20, 30, 45, 240);"
    "  color: #c8d8e8;"
    "  border: 1px solid #304050;"
    "  border-radius: 2px;"
    "  font-size: 10px;"
    "  font-weight: bold;"
    "}"
    "QPushButton:hover { background: rgba(0, 112, 192, 180); }"
    "QPushButton:checked { background: rgba(0, 112, 192, 220); }"
    "QPushButton:disabled {"
    "  background: rgba(15, 20, 30, 200);"
    "  color: #506070;"
    "  border: 1px solid #253040;"
    "}";

static const char* kDspCheckedStyle =
    "QPushButton {"
    "  background: rgba(20, 30, 45, 240);"
    "  color: #c8d8e8;"
    "  border: 1px solid #304050;"
    "  border-radius: 2px;"
    "  font-size: 10px;"
    "  font-weight: bold;"
    "}"
    "QPushButton:hover { background: rgba(0, 112, 192, 180); }"
    "QPushButton:checked { background: #006040; color: #00ff88; }"
    "QPushButton:disabled {"
    "  background: rgba(15, 20, 30, 200);"
    "  color: #506070;"
    "  border: 1px solid #253040;"
    "}";

// ---------------------------------------------------------------------------

SpectrumOverlayPanel::SpectrumOverlayPanel(RadioModel* model, QWidget* parent)
    : QWidget(parent)
    , m_model(model)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedWidth(kPanelW);
    buildUI();
    installEventFilter(this);
}

void SpectrumOverlayPanel::setSlice(SliceModel* slice)
{
    m_slice = slice;
}

void SpectrumOverlayPanel::applyButtonStyle(QPushButton* btn, bool enabled)
{
    btn->setStyleSheet(QLatin1String(kBtnStyle));
    btn->setEnabled(enabled);
    btn->setFixedSize(kBtnW, kBtnH);
}

void SpectrumOverlayPanel::buildUI()
{
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(4, 4, 4, 4);
    outerLayout->setSpacing(2);
    outerLayout->setAlignment(Qt::AlignTop);

    // --- Collapse button (always visible) ---
    m_collapseBtn = new QPushButton(QStringLiteral("\u25C0"), this);  // ◀
    m_collapseBtn->setCheckable(true);
    m_collapseBtn->setToolTip(QStringLiteral("Collapse panel buttons"));
    applyButtonStyle(m_collapseBtn, true);
    outerLayout->addWidget(m_collapseBtn);

    // --- Body: all remaining buttons ---
    m_body = new QWidget(this);
    auto* bodyLayout = new QVBoxLayout(m_body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(2);

    auto makeBtn = [&](const QString& label, const QString& tip, bool enabled) -> QPushButton* {
        auto* btn = new QPushButton(label, m_body);
        btn->setCheckable(true);
        btn->setToolTip(tip);
        btn->setStyleSheet(QLatin1String(kBtnStyle));
        btn->setEnabled(enabled);
        btn->setFixedSize(kBtnW, kBtnH);
        bodyLayout->addWidget(btn);
        return btn;
    };

    m_rxBtn      = makeBtn(QStringLiteral("+RX"),      QStringLiteral("Add RX slice (NYI)"), false);
    m_tnfBtn     = makeBtn(QStringLiteral("+TNF"),     QStringLiteral("Add Tunable Notch Filter (NYI)"), false);
    m_bandBtn    = makeBtn(QStringLiteral("BAND"),     QStringLiteral("Band selector"), true);
    m_antBtn     = makeBtn(QStringLiteral("ANT"),      QStringLiteral("Antenna settings"), true);
    m_dspBtn     = makeBtn(QStringLiteral("DSP"),      QStringLiteral("DSP settings"), true);
    m_displayBtn = makeBtn(QStringLiteral("Display"),  QStringLiteral("Display settings"), true);
    m_daxBtn     = makeBtn(QStringLiteral("DAX"),      QStringLiteral("Digital Audio Exchange (NYI)"), false);
    m_attBtn     = makeBtn(QStringLiteral("ATT"),      QStringLiteral("Attenuator (NYI)"), false);
    m_mnfBtn     = makeBtn(QStringLiteral("MNF"),      QStringLiteral("Manual Notch Filter (NYI)"), false);

    outerLayout->addWidget(m_body);
    outerLayout->addStretch(1);

    // Adjust panel height to fit content
    int totalH = kBtnH + 2  // collapse
               + 9 * (kBtnH + 2)  // 9 body buttons
               + 4 + 4;   // top/bottom margins
    setFixedHeight(totalH);

    // --- Collapse toggle ---
    connect(m_collapseBtn, &QPushButton::toggled, this, [this](bool collapsed) {
        m_body->setVisible(!collapsed);
        m_collapseBtn->setText(collapsed ? QStringLiteral("\u25BA") : QStringLiteral("\u25C0"));
        if (collapsed) {
            hideFlyout();
        }
        // Resize to fit
        adjustSize();
    });

    // --- Flyout button connections ---
    connect(m_bandBtn, &QPushButton::clicked, this, [this](bool checked) {
        if (!checked || m_activeFlyout) {
            hideFlyout();
            if (!checked) { return; }
        }
        QWidget* flyout = buildBandFlyout();
        showFlyout(flyout, m_bandBtn);
    });

    connect(m_antBtn, &QPushButton::clicked, this, [this](bool checked) {
        if (!checked || m_activeFlyout) {
            hideFlyout();
            if (!checked) { return; }
        }
        QWidget* flyout = buildAntFlyout();
        showFlyout(flyout, m_antBtn);
    });

    connect(m_dspBtn, &QPushButton::clicked, this, [this](bool checked) {
        if (!checked || m_activeFlyout) {
            hideFlyout();
            if (!checked) { return; }
        }
        QWidget* flyout = buildDspFlyout();
        showFlyout(flyout, m_dspBtn);
    });

    connect(m_displayBtn, &QPushButton::clicked, this, [this](bool checked) {
        if (!checked || m_activeFlyout) {
            hideFlyout();
            if (!checked) { return; }
        }
        QWidget* flyout = buildDisplayFlyout();
        showFlyout(flyout, m_displayBtn);
    });
}

// ---------------------------------------------------------------------------
// Flyout management
// ---------------------------------------------------------------------------

void SpectrumOverlayPanel::showFlyout(QWidget* flyout, QPushButton* anchor)
{
    m_activeFlyout = flyout;
    m_activeBtn    = anchor;

    QWidget* spectrum = parentWidget();
    flyout->setParent(spectrum);
    flyout->setStyleSheet(QLatin1String(kFlyoutStyle));

    // Position: to the right of the button strip, at the same vertical level as the anchor button
    QPoint anchorPosInSpectrum = mapTo(spectrum, anchor->pos());
    int x = kPanelW + 4;
    int y = anchorPosInSpectrum.y();
    // Clamp so flyout stays within parent
    if (y + flyout->sizeHint().height() > spectrum->height()) {
        y = spectrum->height() - flyout->sizeHint().height() - 4;
    }
    if (y < 0) { y = 0; }

    flyout->move(x, y);
    flyout->show();
    flyout->raise();
    flyout->adjustSize();

    // Reposition after adjustSize (size may differ from sizeHint)
    if (y + flyout->height() > spectrum->height()) {
        y = spectrum->height() - flyout->height() - 4;
        if (y < 0) { y = 0; }
        flyout->move(x, y);
    }

    anchor->setChecked(true);
}

void SpectrumOverlayPanel::hideFlyout()
{
    if (m_activeFlyout) {
        m_activeFlyout->deleteLater();
        m_activeFlyout = nullptr;
    }
    if (m_activeBtn) {
        m_activeBtn->setChecked(false);
        m_activeBtn = nullptr;
    }
}

// ---------------------------------------------------------------------------
// BAND flyout
// ---------------------------------------------------------------------------

QWidget* SpectrumOverlayPanel::buildBandFlyout()
{
    auto* w = new QWidget(nullptr);
    auto* grid = new QGridLayout(w);
    grid->setContentsMargins(6, 6, 6, 6);
    grid->setSpacing(3);

    struct Band { const char* name; double freqHz; };
    static constexpr Band kBands[] = {
        { "160",  1.8e6  },
        { "80",   3.5e6  },
        { "60",   5.3e6  },
        { "40",   7.0e6  },
        { "30",   10.1e6 },
        { "20",   14.0e6 },
        { "17",   18.068e6 },
        { "15",   21.0e6 },
        { "12",   24.89e6 },
        { "10",   28.0e6 },
        { "6",    50.0e6 },
        { "WWV",  10.0e6 },
        { "GEN",  5.0e6  },
        { "LF",   0.137e6 },
        { "MW",   0.530e6 },
    };

    static constexpr int kCols = 6;
    int row = 0;
    int col = 0;
    for (const Band& band : kBands) {
        auto* btn = new QPushButton(QString::fromLatin1(band.name), w);
        btn->setFixedSize(36, 20);
        btn->setStyleSheet(QLatin1String(kBtnStyle));
        double freqHz = band.freqHz;
        QString name  = QString::fromLatin1(band.name);
        connect(btn, &QPushButton::clicked, this, [this, name, freqHz]() {
            emit bandSelected(name, freqHz, QStringLiteral("USB"));
            hideFlyout();
        });
        grid->addWidget(btn, row, col);
        ++col;
        if (col >= kCols) {
            col = 0;
            ++row;
        }
    }

    return w;
}

// ---------------------------------------------------------------------------
// DSP flyout
// ---------------------------------------------------------------------------

QWidget* SpectrumOverlayPanel::buildDspFlyout()
{
    auto* w = new QWidget(nullptr);
    w->setFixedWidth(200);
    auto* layout = new QGridLayout(w);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(4);

    struct DspToggle { const char* label; bool nyi; };
    static constexpr DspToggle kToggles[] = {
        { "NR",  false },
        { "NB",  false },
        { "ANF", false },
        { "SNB", false },
        { "BIN", true  },
        { "MNF", true  },
    };

    static constexpr int kCols = 3;
    int row = 0;
    int col = 0;

    for (const DspToggle& t : kToggles) {
        auto* btn = new QPushButton(QString::fromLatin1(t.label), w);
        btn->setCheckable(true);
        btn->setFixedSize(kBtnW / 2, kBtnH);
        btn->setEnabled(!t.nyi);
        btn->setStyleSheet(QLatin1String(kDspCheckedStyle));

        if (!t.nyi) {
            QString lbl = QString::fromLatin1(t.label);
            if (lbl == QLatin1String("NR")) {
                connect(btn, &QPushButton::toggled, this, &SpectrumOverlayPanel::nrToggled);
            } else if (lbl == QLatin1String("NB")) {
                connect(btn, &QPushButton::toggled, this, &SpectrumOverlayPanel::nbToggled);
            } else if (lbl == QLatin1String("ANF")) {
                connect(btn, &QPushButton::toggled, this, &SpectrumOverlayPanel::anfToggled);
            } else if (lbl == QLatin1String("SNB")) {
                connect(btn, &QPushButton::toggled, this, &SpectrumOverlayPanel::snbToggled);
            }
        }

        layout->addWidget(btn, row, col);
        ++col;
        if (col >= kCols) {
            col = 0;
            ++row;
        }
    }

    return w;
}

// ---------------------------------------------------------------------------
// Display flyout
// ---------------------------------------------------------------------------

QWidget* SpectrumOverlayPanel::buildDisplayFlyout()
{
    auto* w = new QWidget(nullptr);
    w->setFixedWidth(220);
    auto* layout = new QGridLayout(w);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(4);
    layout->setColumnStretch(1, 1);

    int row = 0;

    // Scheme
    auto* schemeLabel = new QLabel(QStringLiteral("Scheme"), w);
    auto* schemeCombo = new QComboBox(w);
    schemeCombo->addItems({
        QStringLiteral("Default"),
        QStringLiteral("Enhanced"),
        QStringLiteral("Spectran"),
        QStringLiteral("B&W"),
    });
    layout->addWidget(schemeLabel, row, 0);
    layout->addWidget(schemeCombo, row, 1);
    ++row;

    connect(schemeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SpectrumOverlayPanel::colorSchemeChanged);

    // Gain
    auto* gainLabel  = new QLabel(QStringLiteral("Gain"), w);
    auto* gainSlider = new QSlider(Qt::Horizontal, w);
    gainSlider->setRange(0, 100);
    gainSlider->setValue(50);
    layout->addWidget(gainLabel,  row, 0);
    layout->addWidget(gainSlider, row, 1);
    ++row;

    connect(gainSlider, &QSlider::valueChanged,
            this, &SpectrumOverlayPanel::wfColorGainChanged);

    // Black level
    auto* blackLabel  = new QLabel(QStringLiteral("Black"), w);
    auto* blackSlider = new QSlider(Qt::Horizontal, w);
    blackSlider->setRange(0, 125);
    blackSlider->setValue(40);
    layout->addWidget(blackLabel,  row, 0);
    layout->addWidget(blackSlider, row, 1);
    ++row;

    connect(blackSlider, &QSlider::valueChanged,
            this, &SpectrumOverlayPanel::wfBlackLevelChanged);

    return w;
}

// ---------------------------------------------------------------------------
// ANT flyout
// ---------------------------------------------------------------------------

QWidget* SpectrumOverlayPanel::buildAntFlyout()
{
    auto* w = new QWidget(nullptr);
    w->setFixedWidth(180);
    auto* layout = new QGridLayout(w);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(4);
    layout->setColumnStretch(1, 1);

    int row = 0;

    // RX Ant
    auto* rxLabel = new QLabel(QStringLiteral("RX Ant"), w);
    auto* rxCombo = new QComboBox(w);
    rxCombo->addItems({ QStringLiteral("ANT1"), QStringLiteral("ANT2"), QStringLiteral("ANT3") });
    layout->addWidget(rxLabel, row, 0);
    layout->addWidget(rxCombo, row, 1);
    ++row;

    // TX Ant
    auto* txLabel = new QLabel(QStringLiteral("TX Ant"), w);
    auto* txCombo = new QComboBox(w);
    txCombo->addItems({ QStringLiteral("ANT1"), QStringLiteral("ANT2"), QStringLiteral("ANT3") });
    layout->addWidget(txLabel, row, 0);
    layout->addWidget(txCombo, row, 1);
    ++row;

    // RF Gain
    auto* gainLabel  = new QLabel(QStringLiteral("RF Gain"), w);
    auto* gainSlider = new QSlider(Qt::Horizontal, w);
    gainSlider->setRange(-8, 32);
    gainSlider->setValue(0);
    layout->addWidget(gainLabel,  row, 0);
    layout->addWidget(gainSlider, row, 1);
    ++row;

    return w;
}

// ---------------------------------------------------------------------------
// Event filter — block mouse/wheel from falling through to spectrum
// ---------------------------------------------------------------------------

bool SpectrumOverlayPanel::eventFilter(QObject* obj, QEvent* event)
{
    Q_UNUSED(obj);
    switch (event->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::Wheel:
            return true;  // block event
        default:
            break;
    }
    return false;
}

} // namespace NereusSDR
