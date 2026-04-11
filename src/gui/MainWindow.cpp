#include "MainWindow.h"
#include "ConnectionPanel.h"
#include "SupportDialog.h"
#include "SpectrumWidget.h"
#include "models/RadioModel.h"
#include "models/SliceModel.h"
#include "widgets/VfoWidget.h"
#include "core/RxChannel.h"
#include "core/ReceiverManager.h"
#include "core/AppSettings.h"
#include "core/RadioDiscovery.h"
#include "core/WdspEngine.h"
#include "core/FFTEngine.h"
#include "core/LogCategories.h"
#include "containers/ContainerManager.h"
#include "containers/ContainerWidget.h"
#include "meters/MeterWidget.h"
#include "meters/MeterItem.h"
#include "meters/ItemGroup.h"
#include "meters/MeterPoller.h"
#include "applets/RxApplet.h"
#include "applets/TxApplet.h"
#include "SpectrumOverlayPanel.h"

#include <cmath>

#include <QApplication>
#include <QSlider>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QSplitter>
#include <QProgressDialog>
#include <QTimer>
#include <QThread>
#include <QDateTime>

#include <cstdlib>

namespace NereusSDR {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_radioModel(new RadioModel(this))
{
    buildUI();
    buildMenuBar();
    buildStatusBar();
    applyDarkTheme();

    // Wire connection state changes to status bar
    connect(m_radioModel, &RadioModel::connectionStateChanged,
            this, &MainWindow::onConnectionStateChanged);

    // WDSP wisdom progress dialog — shown as a modal window during first-run
    // wisdom generation. Pattern from AetherSDR MainWindow::enableNr2WithWisdom().
    connect(m_radioModel->wdspEngine(), &WdspEngine::wisdomProgress,
            this, [this](int percent, const QString& status) {
        // Create dialog on first progress signal
        if (!m_wisdomDialog && percent < 100) {
            m_wisdomDialog = new QProgressDialog(this);
            m_wisdomDialog->setWindowTitle(QStringLiteral("NereusSDR — FFTW Wisdom"));
            m_wisdomDialog->setLabelText(
                QStringLiteral("Optimizing FFT plans for DSP engine...\n\n"
                               "This only happens on first run."));
            m_wisdomDialog->setRange(0, 100);
            m_wisdomDialog->setValue(0);
            m_wisdomDialog->setCancelButton(nullptr);
            m_wisdomDialog->setAutoClose(false);
            m_wisdomDialog->setMinimumWidth(500);
            m_wisdomDialog->setMinimumDuration(0);
            m_wisdomDialog->setWindowModality(Qt::ApplicationModal);
            m_wisdomDialog->setStyleSheet(QStringLiteral(
                "QProgressDialog { background: #0f0f1a; }"
                "QLabel { color: #c8d8e8; font-size: 13px; }"
                "QProgressBar {"
                "  text-align: center; font-size: 13px;"
                "  font-weight: bold; color: #c8d8e8;"
                "  background: #1a2a3a; border: 1px solid #2e4e6e;"
                "  border-radius: 3px; min-height: 24px;"
                "}"
                "QProgressBar::chunk { background: #00b4d8; }"));
            m_wisdomDialog->show();
        }

        if (m_wisdomDialog) {
            m_wisdomDialog->setValue(percent);
            if (!status.isEmpty() && percent < 100) {
                m_wisdomDialog->setLabelText(
                    QStringLiteral("Optimizing FFT plans for DSP engine...\n\n%1").arg(status));
            }
            if (percent >= 100) {
                m_wisdomDialog->setLabelText(QStringLiteral("FFTW planning complete!"));
                m_wisdomDialog->setValue(100);
                // Auto-close after brief delay
                QTimer::singleShot(800, this, [this]() {
                    if (m_wisdomDialog) {
                        m_wisdomDialog->close();
                        m_wisdomDialog->deleteLater();
                        m_wisdomDialog = nullptr;
                    }
                });
            }
        }
    });

    // Start discovery in background so radios are found before the user opens the panel
    m_radioModel->discovery()->startDiscovery();

    // Auto-reconnect to last radio if it appears
    tryAutoReconnect();
}

MainWindow::~MainWindow() = default;

void MainWindow::buildUI()
{
    setWindowTitle(QStringLiteral("NereusSDR %1").arg(NEREUSSDR_VERSION));
    setMinimumSize(800, 600);
    resize(1280, 800);

    // --- Main QSplitter: spectrum (left) + container panel (right) ---
    // AetherSDR pattern: right panel is a proper layout element, not an overlay.
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainSplitter->setChildrenCollapsible(false);
    m_mainSplitter->setHandleWidth(3);
    m_mainSplitter->setStyleSheet(QStringLiteral(
        "QSplitter::handle { background: #203040; }"));

    // Left side: spectrum + zoom bar
    auto* spectrumPane = new QWidget(m_mainSplitter);
    spectrumPane->setMinimumWidth(400);
    auto* layout = new QVBoxLayout(spectrumPane);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_spectrumWidget = new SpectrumWidget(spectrumPane);
    m_spectrumWidget->loadSettings();
    m_spectrumWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(m_spectrumWidget, 1);

    // --- Spectrum overlay panel (Phase 3 UI) ---
    m_overlayPanel = new SpectrumOverlayPanel(m_radioModel, m_spectrumWidget);
    m_overlayPanel->move(4, 4);
    m_overlayPanel->show();
    m_overlayPanel->raise();

    // Zoom slider bar below spectrum
    auto* zoomBar = new QSlider(Qt::Horizontal, spectrumPane);
    zoomBar->setRange(1, 768);
    zoomBar->setValue(768);
    zoomBar->setFixedHeight(20);
    zoomBar->setToolTip(QStringLiteral("Zoom: drag to adjust spectrum bandwidth"));
    zoomBar->setStyleSheet(QStringLiteral(
        "QSlider { background: #0a0a14; }"
        "QSlider::groove:horizontal { background: #1a2a3a; height: 6px; border-radius: 3px; }"
        "QSlider::handle:horizontal { background: #00b4d8; width: 14px; margin: -4px 0; border-radius: 7px; }"));
    layout->addWidget(zoomBar);
    connect(zoomBar, &QSlider::valueChanged, this, [this](int val) {
        double bwHz = val * 1000.0;
        m_spectrumWidget->setFrequencyRange(m_spectrumWidget->centerFrequency(), bwHz);
        emit m_spectrumWidget->bandwidthChangeRequested(bwHz);
    });

    m_mainSplitter->addWidget(spectrumPane);

    // Right side: Container #0 will be added by ContainerManager
    setCentralWidget(m_mainSplitter);

    // --- Container Infrastructure (Phase 3G-1) ---
    m_containerManager = new ContainerManager(spectrumPane, m_mainSplitter, this);
    m_containerManager->restoreState();
    if (m_containerManager->containerCount() == 0) {
        createDefaultContainers();
    }
    m_containerManager->restoreSplitterState();

    // Default splitter sizes on first run: ~80% spectrum, ~20% panel
    if (!AppSettings::instance().contains(QStringLiteral("MainSplitterSizes"))) {
        m_mainSplitter->setSizes({1024, 256});
    }

    // Wire spectrum display to SliceModel (values come from persisted state,
    // no longer hardcoded). Connection is deferred to wireSliceToSpectrum()
    // which runs after RadioModel creates slice 0.
    connect(m_radioModel, &RadioModel::sliceAdded, this, [this](int index) {
        if (index == 0) {
            wireSliceToSpectrum();
        }
    });

    // Create FFTEngine on a worker thread (spectrum thread from architecture)
    m_fftEngine = new FFTEngine(0);  // receiver 0
    m_fftEngine->setSampleRate(768000.0);
    m_fftEngine->setFftSize(4096);
    m_fftEngine->setOutputFps(30);

    m_fftThread = new QThread(this);
    m_fftThread->setObjectName(QStringLiteral("SpectrumThread"));
    m_fftEngine->moveToThread(m_fftThread);

    // Clean up FFTEngine when thread finishes
    connect(m_fftThread, &QThread::finished, m_fftEngine, &QObject::deleteLater);

    // Wire: RadioModel raw I/Q → FFTEngine (auto-queued: main → spectrum thread)
    connect(m_radioModel, &RadioModel::rawIqData,
            m_fftEngine, &FFTEngine::feedIQ);

    // Wire: FFTEngine FFT bins → SpectrumWidget (auto-queued: spectrum → main thread)
    connect(m_fftEngine, &FFTEngine::fftReady,
            m_spectrumWidget, &SpectrumWidget::updateSpectrum);

    // Wire: zoom changes → adjust FFT size for appropriate bin resolution
    // Target: ~500-1000 bins across the visible bandwidth for good detail
    connect(m_spectrumWidget, &SpectrumWidget::bandwidthChangeRequested,
            this, [this](double bwHz) {
        // Pick FFT size so bin_width ≈ bw / 1000 (aim for ~1000 bins across display)
        // bin_width = sampleRate / fftSize → fftSize = sampleRate / bin_width
        double sampleRate = m_spectrumWidget->sampleRate();
        int targetBins = 1000;
        int desiredSize = static_cast<int>(sampleRate * targetBins / bwHz);
        // Round up to next power of 2, clamp to valid range
        int fftSize = 1024;
        while (fftSize < desiredSize && fftSize < 65536) {
            fftSize *= 2;
        }
        fftSize = std::clamp(fftSize, 1024, 65536);
        m_fftEngine->setFftSize(fftSize);
    });

    m_fftThread->start();

    // --- Meter Poller (Phase 3G-2) ---
    m_meterPoller = new MeterPoller(this);
    populateDefaultMeter();

    if (m_meterWidget) {
        m_meterPoller->addTarget(m_meterWidget);
    }
    if (m_txMeterWidget) {
        m_meterPoller->addTarget(m_txMeterWidget);
    }

    // Wire RxChannel to poller when WDSP finishes initializing.
    // RadioModel's initializedChanged handler creates the RxChannel, but
    // it was registered AFTER this connection (RadioModel registers during
    // onConnectionStateChanged, not buildUI). Qt fires in registration order,
    // so we defer by one event loop pass to ensure RxChannel exists.
    connect(m_radioModel->wdspEngine(), &WdspEngine::initializedChanged,
            this, [this](bool ok) {
        if (!ok) { return; }
        QTimer::singleShot(0, this, [this]() {
            RxChannel* rxCh = m_radioModel->wdspEngine()->rxChannel(0);
            if (rxCh) {
                m_meterPoller->setRxChannel(rxCh);
                m_meterPoller->start();
                qCDebug(lcMeter) << "MeterPoller started on RxChannel 0";
            } else {
                qCWarning(lcMeter) << "MeterPoller: RxChannel 0 still null after WDSP init";
            }
        });
    });
}

void MainWindow::createDefaultContainers()
{
    // Container #0: panel-docked right side (AetherSDR pattern).
    // Placeholder content in 3G-1, replaced by AppletPanel in 3G-AP.
    ContainerWidget* c0 = m_containerManager->createContainer(1, DockMode::PanelDocked);
    c0->setNotes(QStringLiteral("Main Panel"));
    c0->setNoControls(false);

    qCDebug(lcContainer) << "Created default Container #0 (panel-docked):" << c0->id();
}

void MainWindow::populateDefaultMeter()
{
    ContainerWidget* c0 = m_containerManager->panelContainer();
    if (!c0) {
        qCWarning(lcContainer) << "No panel container for meter widget";
        return;
    }

    c0->clearContent();

    // --- RX S-Meter ---
    m_meterWidget = new MeterWidget();
    m_meterWidget->setFixedHeight(120);

    // S-Meter: full widget — arc needle bound to SignalAvg
    // From Thetis MeterManager.cs: ANAN needle uses AVG_SIGNAL_STRENGTH
    ItemGroup* smeter = ItemGroup::createSMeterPreset(
        MeterBinding::SignalAvg, QStringLiteral("S-Meter"), m_meterWidget);
    smeter->installInto(m_meterWidget, 0.0f, 0.0f, 1.0f, 1.0f);
    delete smeter;

    c0->addContentWidget(m_meterWidget);

    // --- RX Applet ---
    m_rxApplet = new RxApplet(m_radioModel);
    c0->addContentWidget(m_rxApplet);

    // --- TX Meter (Power/SWR + ALC) ---
    m_txMeterWidget = new MeterWidget();
    m_txMeterWidget->setFixedHeight(80);

    // Power/SWR: upper 65% of TX meter
    ItemGroup* pwrSwr = ItemGroup::createPowerSwrPreset(
        QStringLiteral("Power/SWR"), m_txMeterWidget);
    pwrSwr->installInto(m_txMeterWidget, 0.0f, 0.0f, 1.0f, 0.65f);
    delete pwrSwr;

    // ALC: lower 35% of TX meter
    ItemGroup* alc = ItemGroup::createAlcPreset(m_txMeterWidget);
    alc->installInto(m_txMeterWidget, 0.0f, 0.65f, 1.0f, 0.35f);
    delete alc;

    c0->addContentWidget(m_txMeterWidget);

    // --- TX Applet ---
    m_txApplet = new TxApplet(m_radioModel);
    c0->addContentWidget(m_txApplet);

    qCDebug(lcMeter) << "Installed mixed Container #0: SMeter + RxApplet + Power/SWR/ALC + TxApplet";
}

void MainWindow::buildMenuBar()
{
    // =========================================================
    // File
    // =========================================================
    auto* fileMenu = menuBar()->addMenu(QStringLiteral("&File"));

    fileMenu->addAction(QStringLiteral("&Settings..."), this, []() {
        qCDebug(lcConnection) << "Settings requested (Task 11 NYI)";
    });

    fileMenu->addSeparator();

    fileMenu->addAction(QStringLiteral("&Quit"), QKeySequence::Quit,
                        qApp, &QApplication::quit);

    // =========================================================
    // Radio
    // =========================================================
    auto* radioMenu = menuBar()->addMenu(QStringLiteral("&Radio"));

    radioMenu->addAction(QStringLiteral("&Connect..."), QKeySequence(Qt::CTRL | Qt::Key_K),
                         this, &MainWindow::showConnectionPanel);

    radioMenu->addAction(QStringLiteral("&Disconnect"), this, [this]() {
        m_radioModel->disconnectFromRadio();
    });

    radioMenu->addSeparator();

    radioMenu->addAction(QStringLiteral("&Protocol Info"), this, [this]() {
        if (m_radioModel->isConnected()) {
            RadioInfo info = m_radioModel->connection()->radioInfo();
            QString msg = QStringLiteral("Radio: %1\nProtocol: P%2\nFirmware: %3\nMAC: %4\nIP: %5")
                .arg(info.displayName())
                .arg(static_cast<int>(info.protocol))
                .arg(info.firmwareVersion)
                .arg(info.macAddress, info.address.toString());
            qCDebug(lcConnection) << msg;
        }
    });

    // =========================================================
    // View — all NYI Phase 3F / future
    // =========================================================
    auto* viewMenu = menuBar()->addMenu(QStringLiteral("&View"));

    auto* panMenu = viewMenu->addMenu(QStringLiteral("Pan Layout"));
    for (const QString& label : {
             QStringLiteral("1-up"),
             QStringLiteral("2 Vertical"),
             QStringLiteral("2 Horizontal"),
             QStringLiteral("2x2 Grid"),
             QStringLiteral("1+2 Horizontal")}) {
        QAction* act = panMenu->addAction(label);
        act->setEnabled(false);
    }

    viewMenu->addSeparator();

    auto* bandPlanMenu = viewMenu->addMenu(QStringLiteral("Band Plan"));
    for (const QString& label : {
             QStringLiteral("Off"),
             QStringLiteral("Small"),
             QStringLiteral("Medium"),
             QStringLiteral("Large")}) {
        QAction* act = bandPlanMenu->addAction(label);
        act->setEnabled(false);
    }

    viewMenu->addSeparator();

    auto* uiScaleMenu = viewMenu->addMenu(QStringLiteral("UI Scale"));
    for (const QString& label : {
             QStringLiteral("100%"),
             QStringLiteral("125%"),
             QStringLiteral("150%"),
             QStringLiteral("175%"),
             QStringLiteral("200%")}) {
        QAction* act = uiScaleMenu->addAction(label);
        act->setEnabled(false);
    }

    // =========================================================
    // DSP — checkable toggles wired to RxChannel
    // =========================================================
    auto* dspMenu = menuBar()->addMenu(QStringLiteral("&DSP"));

    QAction* nrAct = dspMenu->addAction(QStringLiteral("NR"));
    nrAct->setCheckable(true);
    connect(nrAct, &QAction::toggled, this, [this](bool on) {
        RxChannel* rxCh = m_radioModel->wdspEngine()->rxChannel(0);
        if (rxCh) { rxCh->setNrEnabled(on); }
    });

    QAction* nbAct = dspMenu->addAction(QStringLiteral("NB"));
    nbAct->setCheckable(true);
    connect(nbAct, &QAction::toggled, this, [this](bool on) {
        RxChannel* rxCh = m_radioModel->wdspEngine()->rxChannel(0);
        if (rxCh) { rxCh->setNb1Enabled(on); }
    });

    QAction* anfAct = dspMenu->addAction(QStringLiteral("ANF"));
    anfAct->setCheckable(true);
    connect(anfAct, &QAction::toggled, this, [this](bool on) {
        RxChannel* rxCh = m_radioModel->wdspEngine()->rxChannel(0);
        if (rxCh) { rxCh->setAnfEnabled(on); }
    });

    dspMenu->addSeparator();

    { QAction* act = dspMenu->addAction(QStringLiteral("Equalizer...")); act->setEnabled(false); }
    { QAction* act = dspMenu->addAction(QStringLiteral("PureSignal...")); act->setEnabled(false); }
    { QAction* act = dspMenu->addAction(QStringLiteral("Diversity...")); act->setEnabled(false); }

    // =========================================================
    // Band
    // =========================================================
    auto* bandMenu = menuBar()->addMenu(QStringLiteral("&Band"));

    auto* hfMenu = bandMenu->addMenu(QStringLiteral("HF"));

    // HF band entries: { label, frequency Hz }
    const struct { const char* label; double freqHz; } kHfBands[] = {
        { "160m (1.8 MHz)",   1.800e6 },
        { "80m (3.5 MHz)",    3.500e6 },
        { "60m (5.3 MHz)",    5.300e6 },
        { "40m (7.0 MHz)",    7.000e6 },
        { "30m (10.1 MHz)",  10.100e6 },
        { "20m (14.0 MHz)",  14.000e6 },
        { "17m (18.068 MHz)",18.068e6 },
        { "15m (21.0 MHz)",  21.000e6 },
        { "12m (24.89 MHz)", 24.890e6 },
        { "10m (28.0 MHz)",  28.000e6 },
        { "6m (50.0 MHz)",   50.000e6 },
    };
    for (const auto& band : kHfBands) {
        double freq = band.freqHz;
        hfMenu->addAction(QString::fromLatin1(band.label), this, [this, freq]() {
            SliceModel* slice = m_radioModel->activeSlice();
            if (slice) { slice->setFrequency(freq); }
        });
    }

    bandMenu->addAction(QStringLiteral("WWV"), this, [this]() {
        SliceModel* slice = m_radioModel->activeSlice();
        if (slice) { slice->setFrequency(10.0e6); }
    });

    bandMenu->addSeparator();

    { QAction* act = bandMenu->addAction(QStringLiteral("Band Stacking...")); act->setEnabled(false); }

    // =========================================================
    // Mode — enum order from WdspTypes.h (LSB=0 … DRM=11)
    // =========================================================
    auto* modeMenu = menuBar()->addMenu(QStringLiteral("&Mode"));

    const struct { const char* label; DSPMode mode; } kModes[] = {
        { "LSB",  DSPMode::LSB  },
        { "USB",  DSPMode::USB  },
        { "DSB",  DSPMode::DSB  },
        { "CWL",  DSPMode::CWL  },
        { "CWU",  DSPMode::CWU  },
        { "FM",   DSPMode::FM   },
        { "AM",   DSPMode::AM   },
        { "DIGU", DSPMode::DIGU },
        { "SPEC", DSPMode::SPEC },
        { "DIGL", DSPMode::DIGL },
        { "SAM",  DSPMode::SAM  },
        { "DRM",  DSPMode::DRM  },
    };
    for (const auto& entry : kModes) {
        DSPMode mode = entry.mode;
        modeMenu->addAction(QString::fromLatin1(entry.label), this, [this, mode]() {
            SliceModel* slice = m_radioModel->activeSlice();
            if (slice) { slice->setDspMode(mode); }
        });
    }

    // =========================================================
    // Containers — NYI
    // =========================================================
    auto* containersMenu = menuBar()->addMenu(QStringLiteral("&Containers"));
    { QAction* act = containersMenu->addAction(QStringLiteral("New Container...")); act->setEnabled(false); }
    { QAction* act = containersMenu->addAction(QStringLiteral("Container Settings...")); act->setEnabled(false); }
    containersMenu->addSeparator();
    { QAction* act = containersMenu->addAction(QStringLiteral("Reset Default Layout")); act->setEnabled(false); }

    // =========================================================
    // Tools
    // =========================================================
    auto* toolsMenu = menuBar()->addMenu(QStringLiteral("&Tools"));
    { QAction* act = toolsMenu->addAction(QStringLiteral("CWX...")); act->setEnabled(false); }
    { QAction* act = toolsMenu->addAction(QStringLiteral("Memory Manager...")); act->setEnabled(false); }
    { QAction* act = toolsMenu->addAction(QStringLiteral("CAT Control...")); act->setEnabled(false); }
    { QAction* act = toolsMenu->addAction(QStringLiteral("TCI Server...")); act->setEnabled(false); }
    { QAction* act = toolsMenu->addAction(QStringLiteral("DAX Audio...")); act->setEnabled(false); }
    toolsMenu->addSeparator();
    { QAction* act = toolsMenu->addAction(QStringLiteral("Network Diagnostics...")); act->setEnabled(false); }
    toolsMenu->addAction(QStringLiteral("&Support..."), this, &MainWindow::showSupportDialog);

    // =========================================================
    // Help
    // =========================================================
    auto* helpMenu = menuBar()->addMenu(QStringLiteral("&Help"));
    { QAction* act = helpMenu->addAction(QStringLiteral("Getting Started...")); act->setEnabled(false); }
    { QAction* act = helpMenu->addAction(QStringLiteral("What's New...")); act->setEnabled(false); }
    helpMenu->addSeparator();
    helpMenu->addAction(QStringLiteral("&About NereusSDR"), this, [this]() {
        Q_UNUSED(this);
    });
}

void MainWindow::buildStatusBar()
{
    // Double-height status bar (AetherSDR pattern)
    statusBar()->setFixedHeight(46);
    statusBar()->setStyleSheet(QStringLiteral(
        "QStatusBar {"
        "  background: #1a2a3a;"
        "  color: #8090a0;"
        "  border-top: 1px solid #203040;"
        "}"
        "QStatusBar::item { border: none; }"));

    // --- Left: connection status indicator ---
    m_connStatusLabel = new QLabel(QStringLiteral(" Disconnected "), this);
    m_connStatusLabel->setStyleSheet(QStringLiteral(
        "QLabel {"
        "  color: #8090a0;"
        "  background: #0f1520;"
        "  border: 1px solid #203040;"
        "  border-radius: 3px;"
        "  padding: 2px 8px;"
        "  font-size: 12px;"
        "}"));
    statusBar()->addWidget(m_connStatusLabel);

    // --- Left: radio info ---
    m_radioInfoLabel = new QLabel(this);
    m_radioInfoLabel->setStyleSheet(QStringLiteral(
        "QLabel { color: #607080; font-size: 12px; }"));
    statusBar()->addWidget(m_radioInfoLabel, 1);  // stretch factor 1

    // --- Center: callsign ---
    m_callsignLabel = new QLabel(this);
    m_callsignLabel->setAlignment(Qt::AlignCenter);
    {
        QString callsign = AppSettings::instance().value(QStringLiteral("StationCallsign")).toString();
        if (!callsign.isEmpty()) {
            m_callsignLabel->setText(QStringLiteral("STATION: %1").arg(callsign));
        }
    }
    m_callsignLabel->setStyleSheet(QStringLiteral(
        "QLabel {"
        "  color: #c8d8e8;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "}"));
    statusBar()->addWidget(m_callsignLabel, 1);  // stretch so it centers

    // --- Right (permanent): UTC time ---
    m_utcTimeLabel = new QLabel(this);
    m_utcTimeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_utcTimeLabel->setStyleSheet(QStringLiteral(
        "QLabel {"
        "  color: #8090a0;"
        "  font-size: 11px;"
        "  padding-right: 8px;"
        "}"));
    statusBar()->addPermanentWidget(m_utcTimeLabel);

    // Clock timer — update UTC label every second
    m_clockTimer = new QTimer(this);
    m_clockTimer->setInterval(1000);
    connect(m_clockTimer, &QTimer::timeout, this, [this]() {
        m_utcTimeLabel->setText(
            QDateTime::currentDateTimeUtc().toString(QStringLiteral("HH:mm:ss UTC  yyyy-MM-dd")));
    });
    m_clockTimer->start();
    // Fire once immediately so the clock shows on startup
    m_utcTimeLabel->setText(
        QDateTime::currentDateTimeUtc().toString(QStringLiteral("HH:mm:ss UTC  yyyy-MM-dd")));

    // Click status label to open connection panel
    m_connStatusLabel->setCursor(Qt::PointingHandCursor);
    m_connStatusLabel->installEventFilter(this);
}

void MainWindow::wireSliceToSpectrum()
{
    SliceModel* slice = m_radioModel->activeSlice();
    if (!slice || !m_spectrumWidget) {
        return;
    }

    // Set initial spectrum display — 48 kHz centered on VFO.
    // With N/2 FFT, only positive frequencies (right half) show real signals.
    double freq = slice->frequency();
    m_spectrumWidget->setFrequencyRange(freq, 768000.0);
    m_spectrumWidget->setDdcCenterFrequency(freq);
    m_spectrumWidget->setSampleRate(768000.0);
    m_spectrumWidget->setVfoFrequency(freq);
    m_spectrumWidget->setFilterOffset(slice->filterLow(), slice->filterHigh());
    m_spectrumWidget->setStepSize(slice->stepHz());

    // --- Create floating VFO flag widget (AetherSDR pattern) ---
    VfoWidget* vfo = m_spectrumWidget->addVfoWidget(0);
    vfo->setFrequency(freq);
    vfo->setMode(slice->dspMode());
    vfo->setFilter(slice->filterLow(), slice->filterHigh());
    vfo->setAgcMode(slice->agcMode());
    vfo->setAfGain(slice->afGain());
    vfo->setRfGain(slice->rfGain());
    vfo->setRxAntenna(slice->rxAntenna());
    vfo->setTxAntenna(slice->txAntenna());
    vfo->setStepHz(slice->stepHz());

    // --- Slice → spectrum display ---

    // VFO frequency change → move VFO marker
    // In CTUN mode (SmartSDR-style): pan stays fixed, VFO moves within it.
    // In traditional mode: pan follows VFO (auto-scroll handled in setVfoFrequency).
    // Band changes (large jumps) always recenter regardless of mode.
    connect(slice, &SliceModel::frequencyChanged, this, [this, vfo, slice](double freq) {
        if (m_handlingBandJump) {
            return;
        }

        double center = m_spectrumWidget->centerFrequency();
        double halfBw = m_spectrumWidget->bandwidth() / 2.0;
        bool offScreen = (freq < center - halfBw) || (freq > center + halfBw);

        if (!m_spectrumWidget->ctunEnabled() || offScreen) {
            m_handlingBandJump = true;

            bool wasCTUN = m_spectrumWidget->ctunEnabled();
            m_radioModel->receiverManager()->setDdcFrequencyLocked(false);

            m_spectrumWidget->setCenterFrequency(freq);

            int rxIdx = slice->receiverIndex();
            if (rxIdx >= 0) {
                m_radioModel->receiverManager()->forceHardwareFrequency(
                    rxIdx, static_cast<quint64>(freq));
            }
            m_spectrumWidget->setDdcCenterFrequency(freq);

            RxChannel* rxCh = m_radioModel->wdspEngine()->rxChannel(0);
            if (rxCh) {
                rxCh->setShiftFrequency(0.0);
            }

            if (wasCTUN) {
                m_radioModel->receiverManager()->setDdcFrequencyLocked(true);
            }

            m_handlingBandJump = false;
        } else {
            // From Thetis radio.rs:1417 — WDSP shift = +(freq - center)
            double shiftHz = freq - center;
            RxChannel* rxCh = m_radioModel->wdspEngine()->rxChannel(0);
            if (rxCh) {
                rxCh->setShiftFrequency(shiftHz);
            }
        }
        m_spectrumWidget->setVfoFrequency(freq);
        vfo->setFrequency(freq);
    });

    connect(slice, &SliceModel::filterChanged, this, [this, vfo](int low, int high) {
        m_spectrumWidget->setFilterOffset(low, high);
        vfo->setFilter(low, high);
    });

    connect(slice, &SliceModel::dspModeChanged, this, [vfo](DSPMode mode) {
        vfo->setMode(mode);
    });

    connect(slice, &SliceModel::agcModeChanged, this, [vfo](AGCMode mode) {
        vfo->setAgcMode(mode);
    });

    connect(slice, &SliceModel::afGainChanged, this, [vfo](int gain) {
        vfo->setAfGain(gain);
    });

    connect(slice, &SliceModel::rfGainChanged, this, [vfo](int gain) {
        vfo->setRfGain(gain);
    });

    connect(slice, &SliceModel::stepHzChanged, this, [this, vfo](int hz) {
        m_spectrumWidget->setStepSize(hz);
        vfo->setStepHz(hz);
    });

    connect(slice, &SliceModel::rxAntennaChanged, this, [vfo](const QString& ant) {
        vfo->setRxAntenna(ant);
    });

    connect(slice, &SliceModel::txAntennaChanged, this, [vfo](const QString& ant) {
        vfo->setTxAntenna(ant);
    });

    // --- VFO flag → slice ---

    connect(vfo, &VfoWidget::frequencyChanged, this, [slice](double hz) {
        slice->setFrequency(hz);
    });

    connect(vfo, &VfoWidget::modeChanged, this, [slice](DSPMode mode) {
        slice->setDspMode(mode);
    });

    connect(vfo, &VfoWidget::filterChanged, this, [slice](int low, int high) {
        slice->setFilter(low, high);
    });

    connect(vfo, &VfoWidget::agcModeChanged, this, [slice](AGCMode mode) {
        slice->setAgcMode(mode);
    });

    connect(vfo, &VfoWidget::afGainChanged, this, [slice](int gain) {
        slice->setAfGain(gain);
    });

    connect(vfo, &VfoWidget::rfGainChanged, this, [slice](int gain) {
        slice->setRfGain(gain);
    });

    connect(vfo, &VfoWidget::rxAntennaChanged, this, [slice](const QString& ant) {
        slice->setRxAntenna(ant);
    });

    connect(vfo, &VfoWidget::txAntennaChanged, this, [slice](const QString& ant) {
        slice->setTxAntenna(ant);
    });

    // NB/NR/ANF → RxChannel directly (not SliceModel properties)
    connect(vfo, &VfoWidget::nb1Changed, this, [this](bool on) {
        RxChannel* rxCh = m_radioModel->wdspEngine()->rxChannel(0);
        if (rxCh) { rxCh->setNb1Enabled(on); }
    });
    connect(vfo, &VfoWidget::nrChanged, this, [this](bool on) {
        RxChannel* rxCh = m_radioModel->wdspEngine()->rxChannel(0);
        if (rxCh) { rxCh->setNrEnabled(on); }
    });
    connect(vfo, &VfoWidget::anfChanged, this, [this](bool on) {
        RxChannel* rxCh = m_radioModel->wdspEngine()->rxChannel(0);
        if (rxCh) { rxCh->setAnfEnabled(on); }
    });

    connect(vfo, &VfoWidget::sliceActivationRequested, this, [this](int index) {
        m_radioModel->setActiveSlice(index);
    });

    // --- Spectrum click-to-tune → slice ---
    connect(m_spectrumWidget, &SpectrumWidget::frequencyClicked,
            this, [slice](double hz) {
        slice->setFrequency(hz);
    });

    // --- Spectrum filter edge drag → slice ---
    connect(m_spectrumWidget, &SpectrumWidget::filterEdgeDragged,
            this, [slice](int low, int high) {
        slice->setFilter(low, high);
    });

    // --- Pan center changed (pan drag) ---
    // CTUN mode (SmartSDR): retune DDC to pan center, offset WDSP to keep
    // demodulating at VFO frequency. This lets the spectrum show real data
    // across the full pan range.
    // Traditional mode: pan drag retunes the VFO (DDC follows VFO naturally).
    connect(m_spectrumWidget, &SpectrumWidget::centerChanged,
            this, [this, slice](double centerHz) {
        if (m_handlingBandJump) {
            return;
        }
        if (!m_spectrumWidget->ctunEnabled()) {
            slice->setFrequency(centerHz);
        } else {
            // CTUN: retune DDC to pan center (bypasses lock) so spectrum shows correct data
            int rxIdx = slice->receiverIndex();
            if (rxIdx >= 0) {
                m_radioModel->receiverManager()->forceHardwareFrequency(
                    rxIdx, static_cast<quint64>(centerHz));
            }
            m_spectrumWidget->setDdcCenterFrequency(centerHz);
            // Offset WDSP shift so audio stays on VFO frequency
            // From Thetis radio.cs:1417 — SetRXAShiftFreq receives +(freq - center)
            double shiftHz = slice->frequency() - centerHz;
            RxChannel* rxCh = m_radioModel->wdspEngine()->rxChannel(0);
            if (rxCh) {
                rxCh->setShiftFrequency(shiftHz);
            }
        }
    });

    // --- CTUN mode toggled → lock/unlock DDC ---
    connect(m_spectrumWidget, &SpectrumWidget::ctunEnabledChanged,
            this, [this](bool enabled) {
        m_radioModel->receiverManager()->setDdcFrequencyLocked(enabled);
        if (!enabled) {
            RxChannel* rxCh = m_radioModel->wdspEngine()->rxChannel(0);
            if (rxCh) {
                rxCh->setShiftFrequency(0.0);
            }
        }
    });

    // Wire RxApplet to the active slice
    if (m_rxApplet) {
        m_rxApplet->setSlice(slice);
    }

    // --- SpectrumOverlayPanel wiring (Phase 3 UI) ---
    if (m_overlayPanel) {
        m_overlayPanel->setSlice(slice);

        connect(m_overlayPanel, &SpectrumOverlayPanel::bandSelected,
                this, [slice](const QString& /*bandName*/, double freqHz, const QString& /*mode*/) {
            slice->setFrequency(freqHz);
        });

        connect(m_overlayPanel, &SpectrumOverlayPanel::nrToggled,
                this, [this](bool on) {
            RxChannel* rxCh = m_radioModel->wdspEngine()->rxChannel(0);
            if (rxCh) { rxCh->setNrEnabled(on); }
        });

        connect(m_overlayPanel, &SpectrumOverlayPanel::nbToggled,
                this, [this](bool on) {
            RxChannel* rxCh = m_radioModel->wdspEngine()->rxChannel(0);
            if (rxCh) { rxCh->setNb1Enabled(on); }
        });

        connect(m_overlayPanel, &SpectrumOverlayPanel::anfToggled,
                this, [this](bool on) {
            RxChannel* rxCh = m_radioModel->wdspEngine()->rxChannel(0);
            if (rxCh) { rxCh->setAnfEnabled(on); }
        });

        connect(m_overlayPanel, &SpectrumOverlayPanel::wfColorGainChanged,
                this, [this](int gain) {
            m_spectrumWidget->setWfColorGain(gain);
        });

        connect(m_overlayPanel, &SpectrumOverlayPanel::wfBlackLevelChanged,
                this, [this](int level) {
            m_spectrumWidget->setWfBlackLevel(level);
        });

        connect(m_overlayPanel, &SpectrumOverlayPanel::colorSchemeChanged,
                this, [this](int index) {
            m_spectrumWidget->setWfColorScheme(static_cast<WfColorScheme>(index));
        });
    }

    // Set initial lock state
    m_radioModel->receiverManager()->setDdcFrequencyLocked(
        m_spectrumWidget->ctunEnabled());

    // Position the VFO flag
    m_spectrumWidget->updateVfoPositions();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);

    // Update axis-lock positions for overlay-docked containers
    if (m_mainSplitter && m_containerManager) {
        // Use the spectrum pane (first splitter child) as reference
        QWidget* spectrumPane = m_mainSplitter->widget(0);
        if (spectrumPane) {
            m_hDelta = spectrumPane->width();
            m_vDelta = spectrumPane->height();
            m_containerManager->updateDockedPositions(m_hDelta, m_vDelta);
        }
    }
}

void MainWindow::applyDarkTheme()
{
    setStyleSheet(QStringLiteral(
        "QMainWindow { background: #0f0f1a; }"
        "QMenuBar {"
        "  background: #1a2a3a;"
        "  color: #c8d8e8;"
        "  border-bottom: 1px solid #203040;"
        "}"
        "QMenuBar::item:selected { background: #00b4d8; }"
        "QMenu {"
        "  background: #1a2a3a;"
        "  color: #c8d8e8;"
        "  border: 1px solid #203040;"
        "}"
        "QMenu::item:selected { background: #00b4d8; }"
        "QLabel { color: #c8d8e8; }"
        "QStatusBar {"
        "  background: #1a2a3a;"
        "  color: #8090a0;"
        "  border-top: 1px solid #203040;"
        "}"));
}

void MainWindow::showConnectionPanel()
{
    if (!m_connectionPanel) {
        m_connectionPanel = new ConnectionPanel(m_radioModel, this);
        m_connectionPanel->setAttribute(Qt::WA_DeleteOnClose);
        connect(m_connectionPanel, &QObject::destroyed, this, [this]() {
            m_connectionPanel = nullptr;
        });
    }
    m_connectionPanel->show();
    m_connectionPanel->raise();
    m_connectionPanel->activateWindow();
}

void MainWindow::showSupportDialog()
{
    if (!m_supportDialog) {
        m_supportDialog = new SupportDialog(m_radioModel, this);
        m_supportDialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(m_supportDialog, &QObject::destroyed, this, [this]() {
            m_supportDialog = nullptr;
        });
    }
    m_supportDialog->show();
    m_supportDialog->raise();
    m_supportDialog->activateWindow();
}

void MainWindow::onConnectionStateChanged()
{
    if (m_radioModel->isConnected()) {
        m_connStatusLabel->setText(QStringLiteral(" Connected "));
        m_connStatusLabel->setStyleSheet(QStringLiteral(
            "QLabel {"
            "  color: #ffffff;"
            "  background: #007a3d;"
            "  border: 1px solid #00a050;"
            "  border-radius: 3px;"
            "  padding: 2px 8px;"
            "  font-size: 12px;"
            "}"));
        m_radioInfoLabel->setText(QStringLiteral("%1  —  FW %2")
            .arg(m_radioModel->name(), m_radioModel->version()));
        m_radioInfoLabel->setStyleSheet(QStringLiteral(
            "QLabel { color: #c8d8e8; font-size: 12px; }"));
    } else {
        m_connStatusLabel->setText(QStringLiteral(" Disconnected "));
        m_connStatusLabel->setStyleSheet(QStringLiteral(
            "QLabel {"
            "  color: #8090a0;"
            "  background: #1a2a3a;"
            "  border: 1px solid #203040;"
            "  border-radius: 3px;"
            "  padding: 2px 8px;"
            "  font-size: 12px;"
            "}"));
        m_radioInfoLabel->setText(QString());
        m_radioInfoLabel->setStyleSheet(QStringLiteral(
            "QLabel { color: #607080; font-size: 12px; }"));
    }
}

void MainWindow::tryAutoReconnect()
{
    QString lastMac = AppSettings::instance()
        .value(QStringLiteral("LastConnectedRadioMac")).toString();
    if (lastMac.isEmpty()) {
        return;
    }

    // When a radio matching the last MAC is discovered, auto-connect
    connect(m_radioModel->discovery(), &RadioDiscovery::radioDiscovered,
            this, [this, lastMac](const RadioInfo& info) {
        if (info.macAddress == lastMac && !m_radioModel->isConnected() && !info.inUse) {
            qCDebug(lcConnection) << "Auto-reconnecting to" << info.displayName();
            m_radioModel->connectToRadio(info);
        }
    });
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // Stop discovery to prevent new signals during shutdown
    m_radioModel->discovery()->stopDiscovery();

    // Stop FFT thread
    if (m_fftThread && m_fftThread->isRunning()) {
        m_fftThread->quit();
        m_fftThread->wait(2000);
    }

    // Save display settings before shutdown
    if (m_spectrumWidget) {
        m_spectrumWidget->saveSettings();
    }

    // Tear down connection (sends stop command, closes sockets, joins thread)
    m_radioModel->disconnectFromRadio();

    // Save container layout
    if (m_containerManager) {
        m_containerManager->saveState();
    }

    AppSettings::instance().save();
    event->accept();

    // Force process exit. Worker threads and active QObjects can prevent
    // the Qt event loop from returning. Settings are saved, sockets closed,
    // and stop command sent to the radio — safe to exit now.
    std::exit(0);
}

} // namespace NereusSDR
