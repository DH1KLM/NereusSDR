//DH1KLM: NereusSDR-original receive-view transaction and per-band session memory; GPL-3.0.
#pragma once
#include <QObject>
#include <QHash>
#include <QPointer>
#include <QTimer>
#include <functional>
#include "gui/NereusGuiExport.h"

namespace NereusSDR {
class SpectrumWidget;
class SpectrumOverlayPanel;
class RadioModel;
class SliceModel;

class NEREUS_GUI_EXPORT BandCenterController final : public QObject {
public:
    BandCenterController(NereusSDR::RadioModel* model, SpectrumWidget* pan,
                         SpectrumOverlayPanel* panel,
                         std::function<NereusSDR::SliceModel*()> receiver);
private:
    struct View {
        double centre{}, span{}, ddc{};
        int rate{};
        bool ctun{}, extended{}, paneCtun{};
    };
    struct Memory { View before{}, view{}; bool active{}; double lastFrequency{}; };
    View snapshot() const;
    void followBand();
    void toggle(bool active);
    void apply(View target, bool active);
    void finish();
    void publish(const QString& reason = {});
    void observeSlices();
    void saveMemory() const;
    void loadMemory();
    QPointer<NereusSDR::RadioModel> m_model;
    QPointer<SpectrumWidget> m_pan;
    QPointer<SpectrumOverlayPanel> m_panel;
    std::function<NereusSDR::SliceModel*()> m_receiver;
    QHash<QString, Memory> m_memory;
    QString m_band;
    QString m_radio;
    QString m_reason;
    QTimer m_pending;
    View m_target{};
    int m_slice{-1}, m_stream{-1}, m_attempts{};
    bool m_targetActive{}, m_applying{}, m_queued{};
};
}
