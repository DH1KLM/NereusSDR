#include "SetupDialog.h"
#include "SetupPage.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStackedWidget>
#include <QSplitter>
#include <QHBoxLayout>

namespace NereusSDR {

SetupDialog::SetupDialog(RadioModel* model, QWidget* parent)
    : QDialog(parent)
    , m_model(model)
{
    setWindowTitle(QStringLiteral("NereusSDR Settings"));
    setMinimumSize(800, 600);
    resize(900, 650);

    setStyleSheet(QStringLiteral(
        "QDialog {"
        "  background: #0f0f1a;"
        "}"
        "QTreeWidget {"
        "  background: #131326;"
        "  color: #c8d8e8;"
        "  border: none;"
        "  font-size: 12px;"
        "}"
        "QTreeWidget::item {"
        "  padding: 4px 8px;"
        "}"
        "QTreeWidget::item:selected {"
        "  background: #00b4d8;"
        "  color: #0f0f1a;"
        "}"
        "QTreeWidget::item:hover:!selected {"
        "  background: #1a2a3a;"
        "}"
        "QStackedWidget {"
        "  background: #0f0f1a;"
        "}"
    ));

    auto* outerLayout = new QHBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(1);
    splitter->setChildrenCollapsible(false);

    m_tree = new QTreeWidget(splitter);
    m_tree->setHeaderHidden(true);
    m_tree->setFixedWidth(200);
    m_tree->setIndentation(16);

    m_stack = new QStackedWidget(splitter);
    m_stack->setStyleSheet(QStringLiteral("QStackedWidget { background: #0f0f1a; }"));

    splitter->addWidget(m_tree);
    splitter->addWidget(m_stack);
    outerLayout->addWidget(splitter);

    connect(m_tree, &QTreeWidget::currentItemChanged,
            this, [this](QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/) {
        if (!current) { return; }
        const QVariant data = current->data(0, Qt::UserRole);
        if (data.isValid()) {
            m_stack->setCurrentIndex(data.toInt());
        }
    });

    buildTree();
    m_tree->expandAll();

    // Select the first leaf item
    QTreeWidgetItem* root = m_tree->topLevelItem(0);
    if (root && root->childCount() > 0) {
        m_tree->setCurrentItem(root->child(0));
    } else if (root) {
        m_tree->setCurrentItem(root);
    }
}

void SetupDialog::addPage(QTreeWidgetItem* parent, const QString& name, SetupPage* page)
{
    auto* item = new QTreeWidgetItem(parent, QStringList{name});
    const int index = m_stack->addWidget(page);
    item->setData(0, Qt::UserRole, index);
}

void SetupDialog::buildTree()
{
    // ---------------------------------------------------------------
    // General
    // ---------------------------------------------------------------
    auto* general = new QTreeWidgetItem(m_tree, QStringList{QStringLiteral("General")});
    addPage(general, QStringLiteral("Startup & Preferences"),
            new SetupPage(QStringLiteral("Startup & Preferences"), m_model, m_stack));
    addPage(general, QStringLiteral("UI Scale & Theme"),
            new SetupPage(QStringLiteral("UI Scale & Theme"), m_model, m_stack));
    addPage(general, QStringLiteral("Navigation"),
            new SetupPage(QStringLiteral("Navigation"), m_model, m_stack));

    // ---------------------------------------------------------------
    // Hardware
    // ---------------------------------------------------------------
    auto* hardware = new QTreeWidgetItem(m_tree, QStringList{QStringLiteral("Hardware")});
    addPage(hardware, QStringLiteral("Radio Selection"),
            new SetupPage(QStringLiteral("Radio Selection"), m_model, m_stack));
    addPage(hardware, QStringLiteral("ADC / DDC Configuration"),
            new SetupPage(QStringLiteral("ADC / DDC Configuration"), m_model, m_stack));
    addPage(hardware, QStringLiteral("Calibration"),
            new SetupPage(QStringLiteral("Calibration"), m_model, m_stack));
    addPage(hardware, QStringLiteral("Alex Filters"),
            new SetupPage(QStringLiteral("Alex Filters"), m_model, m_stack));
    addPage(hardware, QStringLiteral("Penny / Hermes OC"),
            new SetupPage(QStringLiteral("Penny / Hermes OC"), m_model, m_stack));
    addPage(hardware, QStringLiteral("Firmware"),
            new SetupPage(QStringLiteral("Firmware"), m_model, m_stack));
    addPage(hardware, QStringLiteral("Other H/W"),
            new SetupPage(QStringLiteral("Other H/W"), m_model, m_stack));

    // ---------------------------------------------------------------
    // Audio
    // ---------------------------------------------------------------
    auto* audio = new QTreeWidgetItem(m_tree, QStringList{QStringLiteral("Audio")});
    addPage(audio, QStringLiteral("Device Selection"),
            new SetupPage(QStringLiteral("Device Selection"), m_model, m_stack));
    addPage(audio, QStringLiteral("ASIO Configuration"),
            new SetupPage(QStringLiteral("ASIO Configuration"), m_model, m_stack));
    addPage(audio, QStringLiteral("VAC 1"),
            new SetupPage(QStringLiteral("VAC 1"), m_model, m_stack));
    addPage(audio, QStringLiteral("VAC 2"),
            new SetupPage(QStringLiteral("VAC 2"), m_model, m_stack));
    addPage(audio, QStringLiteral("NereusDAX"),
            new SetupPage(QStringLiteral("NereusDAX"), m_model, m_stack));
    addPage(audio, QStringLiteral("Recording"),
            new SetupPage(QStringLiteral("Recording"), m_model, m_stack));

    // ---------------------------------------------------------------
    // DSP
    // ---------------------------------------------------------------
    auto* dsp = new QTreeWidgetItem(m_tree, QStringList{QStringLiteral("DSP")});
    addPage(dsp, QStringLiteral("AGC / ALC"),
            new SetupPage(QStringLiteral("AGC / ALC"), m_model, m_stack));
    addPage(dsp, QStringLiteral("Noise Reduction (NR / ANF)"),
            new SetupPage(QStringLiteral("Noise Reduction (NR / ANF)"), m_model, m_stack));
    addPage(dsp, QStringLiteral("Noise Blanker (NB / SNB)"),
            new SetupPage(QStringLiteral("Noise Blanker (NB / SNB)"), m_model, m_stack));
    addPage(dsp, QStringLiteral("CW"),
            new SetupPage(QStringLiteral("CW"), m_model, m_stack));
    addPage(dsp, QStringLiteral("AM / SAM"),
            new SetupPage(QStringLiteral("AM / SAM"), m_model, m_stack));
    addPage(dsp, QStringLiteral("FM"),
            new SetupPage(QStringLiteral("FM"), m_model, m_stack));
    addPage(dsp, QStringLiteral("VOX / DEXP"),
            new SetupPage(QStringLiteral("VOX / DEXP"), m_model, m_stack));
    addPage(dsp, QStringLiteral("CFC"),
            new SetupPage(QStringLiteral("CFC"), m_model, m_stack));
    addPage(dsp, QStringLiteral("MNF"),
            new SetupPage(QStringLiteral("MNF"), m_model, m_stack));

    // ---------------------------------------------------------------
    // Display
    // ---------------------------------------------------------------
    auto* display = new QTreeWidgetItem(m_tree, QStringList{QStringLiteral("Display")});
    addPage(display, QStringLiteral("Spectrum Defaults"),
            new SetupPage(QStringLiteral("Spectrum Defaults"), m_model, m_stack));
    addPage(display, QStringLiteral("Waterfall Defaults"),
            new SetupPage(QStringLiteral("Waterfall Defaults"), m_model, m_stack));
    addPage(display, QStringLiteral("Grid & Scales"),
            new SetupPage(QStringLiteral("Grid & Scales"), m_model, m_stack));
    addPage(display, QStringLiteral("RX2 Display"),
            new SetupPage(QStringLiteral("RX2 Display"), m_model, m_stack));
    addPage(display, QStringLiteral("TX Display"),
            new SetupPage(QStringLiteral("TX Display"), m_model, m_stack));

    // ---------------------------------------------------------------
    // Transmit
    // ---------------------------------------------------------------
    auto* transmit = new QTreeWidgetItem(m_tree, QStringList{QStringLiteral("Transmit")});
    addPage(transmit, QStringLiteral("Power & PA"),
            new SetupPage(QStringLiteral("Power & PA"), m_model, m_stack));
    addPage(transmit, QStringLiteral("TX Profiles"),
            new SetupPage(QStringLiteral("TX Profiles"), m_model, m_stack));
    addPage(transmit, QStringLiteral("Speech Processor"),
            new SetupPage(QStringLiteral("Speech Processor"), m_model, m_stack));
    addPage(transmit, QStringLiteral("PureSignal"),
            new SetupPage(QStringLiteral("PureSignal"), m_model, m_stack));

    // ---------------------------------------------------------------
    // Appearance
    // ---------------------------------------------------------------
    auto* appearance = new QTreeWidgetItem(m_tree, QStringList{QStringLiteral("Appearance")});
    addPage(appearance, QStringLiteral("Colors & Theme"),
            new SetupPage(QStringLiteral("Colors & Theme"), m_model, m_stack));
    addPage(appearance, QStringLiteral("Meter Styles"),
            new SetupPage(QStringLiteral("Meter Styles"), m_model, m_stack));
    addPage(appearance, QStringLiteral("Skins"),
            new SetupPage(QStringLiteral("Skins"), m_model, m_stack));

    // ---------------------------------------------------------------
    // CAT & Network
    // ---------------------------------------------------------------
    auto* cat = new QTreeWidgetItem(m_tree, QStringList{QStringLiteral("CAT & Network")});
    addPage(cat, QStringLiteral("Serial Ports"),
            new SetupPage(QStringLiteral("Serial Ports"), m_model, m_stack));
    addPage(cat, QStringLiteral("TCI Server"),
            new SetupPage(QStringLiteral("TCI Server"), m_model, m_stack));
    addPage(cat, QStringLiteral("TCP/IP CAT"),
            new SetupPage(QStringLiteral("TCP/IP CAT"), m_model, m_stack));
    addPage(cat, QStringLiteral("MIDI Control"),
            new SetupPage(QStringLiteral("MIDI Control"), m_model, m_stack));

    // ---------------------------------------------------------------
    // Keyboard
    // ---------------------------------------------------------------
    auto* keyboard = new QTreeWidgetItem(m_tree, QStringList{QStringLiteral("Keyboard")});
    addPage(keyboard, QStringLiteral("Shortcuts"),
            new SetupPage(QStringLiteral("Shortcuts"), m_model, m_stack));

    // ---------------------------------------------------------------
    // Diagnostics
    // ---------------------------------------------------------------
    auto* diagnostics = new QTreeWidgetItem(m_tree, QStringList{QStringLiteral("Diagnostics")});
    addPage(diagnostics, QStringLiteral("Signal Generator"),
            new SetupPage(QStringLiteral("Signal Generator"), m_model, m_stack));
    addPage(diagnostics, QStringLiteral("Hardware Tests"),
            new SetupPage(QStringLiteral("Hardware Tests"), m_model, m_stack));
    addPage(diagnostics, QStringLiteral("Logging"),
            new SetupPage(QStringLiteral("Logging"), m_model, m_stack));
}

} // namespace NereusSDR
