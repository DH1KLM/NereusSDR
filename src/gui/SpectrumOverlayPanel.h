#pragma once

#include <QWidget>

class QPushButton;

namespace NereusSDR {

class RadioModel;
class SliceModel;

// Vertical button strip anchored to the top-left of SpectrumWidget.
// Each button opens a flyout sub-panel to the right of the strip.
// Fixed width: kPanelW (76px = 68px button + 8px margin).
//
// Buttons: Collapse, +RX (NYI), +TNF (NYI), BAND, ANT, DSP, Display, DAX (NYI), ATT (NYI), MNF (NYI)
// Active flyouts: BAND, ANT, DSP, Display
//
// Phase 3 UI — AetherSDR-style quick-access overlay panel
class SpectrumOverlayPanel : public QWidget {
    Q_OBJECT

public:
    explicit SpectrumOverlayPanel(RadioModel* model, QWidget* parent = nullptr);

    // Set the active slice for band-change routing.
    void setSlice(SliceModel* slice);

signals:
    void bandSelected(const QString& bandName, double freqHz, const QString& mode);
    void nbToggled(bool enabled);
    void nrToggled(bool enabled);
    void anfToggled(bool enabled);
    void snbToggled(bool enabled);
    void wfColorGainChanged(int gain);
    void wfBlackLevelChanged(int level);
    void colorSchemeChanged(int index);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void buildUI();
    void applyButtonStyle(QPushButton* btn, bool enabled = true);

    void showFlyout(QWidget* flyout, QPushButton* anchor);
    void hideFlyout();

    QWidget* buildBandFlyout();
    QWidget* buildDspFlyout();
    QWidget* buildDisplayFlyout();
    QWidget* buildAntFlyout();

    // Button size constants
    static constexpr int kBtnW = 68;
    static constexpr int kBtnH = 22;
    static constexpr int kPanelW = 76;  // button + 8px right margin

    RadioModel*  m_model{nullptr};
    SliceModel*  m_slice{nullptr};

    // Buttons
    QPushButton* m_collapseBtn{nullptr};
    QPushButton* m_rxBtn{nullptr};
    QPushButton* m_tnfBtn{nullptr};
    QPushButton* m_bandBtn{nullptr};
    QPushButton* m_antBtn{nullptr};
    QPushButton* m_dspBtn{nullptr};
    QPushButton* m_displayBtn{nullptr};
    QPushButton* m_daxBtn{nullptr};
    QPushButton* m_attBtn{nullptr};
    QPushButton* m_mnfBtn{nullptr};

    // Collapsible body widget (all buttons except Collapse)
    QWidget* m_body{nullptr};

    // Active flyout state
    QWidget*     m_activeFlyout{nullptr};
    QPushButton* m_activeBtn{nullptr};
};

} // namespace NereusSDR
