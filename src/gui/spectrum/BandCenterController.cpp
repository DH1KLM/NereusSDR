//DH1KLM: NereusSDR-original receive-view transaction; GPL-3.0.
//DH1KLM: Per-band before/view design follows AetherSDR AnanBackend.cpp:642-670,2519-2605 [@ab8c82b1].
#include "BandCenterController.h"
#include "BandCenterView.h"
#include "gui/SpectrumWidget.h"
#include "gui/SpectrumOverlayPanel.h"
#include "models/RadioModel.h"
#include "models/SliceModel.h"
#include "core/AppSettings.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace NereusSDR {
BandCenterController::BandCenterController(RadioModel* model, SpectrumWidget* pan,
    SpectrumOverlayPanel* panel, std::function<SliceModel*()> receiver)
    : QObject(panel), m_model(model), m_pan(pan), m_panel(panel), m_receiver(std::move(receiver))
{
    loadMemory();
    m_pending.setInterval(50);
    connect(&m_pending, &QTimer::timeout, this, &BandCenterController::finish);
    connect(panel, &SpectrumOverlayPanel::bandCenterRequested, this, &BandCenterController::toggle);
    const auto schedule = [this]() {
        if (m_queued) { return; }
        m_queued = true;
        QTimer::singleShot(0, this, [this]() { m_queued = false; followBand(); });
    };
    connect(model, &RadioModel::activeSliceChanged, this, schedule);
    connect(model, &RadioModel::streamBindingsChanged, this, schedule);
    connect(model, &RadioModel::currentRadioChanged, this, schedule);
    connect(model, &RadioModel::connectionStateChanged, this, schedule);
    connect(model, &RadioModel::transmittingChanged, this, [this]() { publish(); });
    connect(model, &RadioModel::sliceAdded, this, [this, schedule]() { observeSlices(); schedule(); });
    connect(model, &RadioModel::streamCentreChanged, this, schedule);
    const auto remember = [this]() {
        if (m_applying || m_pending.isActive() || m_band.isEmpty() || !m_pan) { return; }
        const auto bounds = m_pan->regionalBandBounds();
        if (m_band.endsWith(QStringLiteral("/%1/%2").arg(bounds.first, 0, 'f', 0).arg(bounds.second, 0, 'f', 0))) {
            m_memory[m_band].view = snapshot();
            saveMemory();
        }
    };
    connect(pan, &SpectrumWidget::bandwidthChangeRequested, this, remember);
    connect(pan, &SpectrumWidget::centerChanged, this, remember);
    observeSlices();
    followBand();
}

void BandCenterController::observeSlices()
{
    for (auto* slice : m_model->slices()) {
        const QString key = QStringLiteral("bandCenterObserved_%1").arg(reinterpret_cast<quintptr>(this));
        if (slice->property(key.toUtf8().constData()).toBool()) { continue; }
        slice->setProperty(key.toUtf8().constData(), true);
        connect(slice, &SliceModel::frequencyChanged, this, [this]() {
            QTimer::singleShot(0, this, [this]() { followBand(); });
        });
    }
}

BandCenterController::View BandCenterController::snapshot() const
{
    auto* slice = m_receiver();
    const int stream = slice ? slice->streamIndex() : -1;
    const auto& allocator = m_model->streamAllocator();
    return {m_pan->centerFrequency(), m_pan->bandwidth(),
        stream >= 0 ? allocator.streamCentreHz(stream) : m_pan->ddcCenterFrequency(),
        slice ? slice->sampleRateHz() : 0,
        stream >= 0 && m_model->streamCtunPinned(stream), m_pan->extendedViewAllowed(), m_pan->ctunEnabled()};
}

void BandCenterController::publish(const QString& reason)
{
    if (!m_panel || !m_pan || !m_model) { return; }
    const bool active = !m_band.isEmpty() && m_memory.value(m_band).active;
    m_pan->setBandCenterActive(active);
    const bool enabled = !m_band.isEmpty() && !m_pending.isActive()
        && m_model->ownsLocalDsp() && m_model->isConnected() && !m_model->isTransmitting()
        && m_pan->ctunHeldReason().isEmpty();
    if (!reason.isEmpty()) { m_reason = reason; }
    QString detail = m_reason;
    if (m_pending.isActive()) { detail = QStringLiteral("Waiting for the receive sample rate to settle"); }
    const auto [low, high] = m_pan->regionalBandBounds();
    if (detail.isEmpty() && active && (low <= m_pan->centerFrequency() - m_pan->bandwidth() / 2.0
        || high >= m_pan->centerFrequency() + m_pan->bandwidth() / 2.0)) {
        detail = QStringLiteral("Band exceeds the selected sample rate; only the captured part is shown. Click C to restore.");
    }
    m_panel->setBandCenterState(active, enabled,
        m_model->isTransmitting() ? QStringLiteral("Center view is held during transmit") : detail);
}

void BandCenterController::followBand()
{
    if (!m_model || !m_pan) { return; }
    auto* slice = m_receiver();
    const auto [low, high] = m_pan->regionalBandBounds();
    const QString radio = m_model->currentRadioMac();
    const QString band = slice && high > low && m_model->isConnected()
        ? QStringLiteral("%1/%2/%3/%4").arg(radio).arg(slice->sliceIndex()).arg(low, 0, 'f', 0).arg(high, 0, 'f', 0)
        : QString();
    if (band == m_band) {
        if (slice && !band.isEmpty() && m_memory[band].active && !m_pending.isActive()
            && !m_model->isTransmitting() && m_memory[band].view.rate != slice->sampleRateHz()) {
            const auto plan = planBandCenter(low, high, slice->frequency(), slice->sampleRateHz(), slice->filterLow(), slice->filterHigh());
            if (plan) {
                auto view = snapshot();
                view.centre = view.ddc = plan->centreHz;
                view.span = plan->spanHz;
                view.ctun = true;
                view.paneCtun = true;
                view.extended = false;
                apply(view, true);
            }
        }
        publish(); return;
    }
    m_pending.stop();
    const auto previous = m_memory.value(m_band);
    //DH1KLM: A band jump must never apply the old band's DDC or a stale asynchronous completion.
    if (previous.active && slice && radio == m_radio && !m_model->isTransmitting()) {
        m_model->requestStreamCtunPinned(slice->sliceIndex(), previous.before.ctun);
        m_pan->applyRemoteCtunState(m_pan->ctunAvailable(), previous.before.paneCtun);
        m_pan->setExtendedViewAllowed(previous.before.extended);
    }
    m_radio = radio;
    m_band = band;
    m_reason.clear();
    if (!band.isEmpty()) {
        if (!m_memory.contains(band)) { m_memory.insert(band, {snapshot(), snapshot(), false}); }
        if (m_memory[band].active && !m_model->isTransmitting()) {
            m_memory[band].active = false;
            const auto plan = planBandCenter(low, high, slice->frequency(), slice->sampleRateHz(), slice->filterLow(), slice->filterHigh());
            if (plan) {
                auto view = m_memory[band].view;
                if (view.rate != slice->sampleRateHz()) {
                    view = snapshot();
                    view.centre = view.ddc = plan->centreHz;
                    view.span = plan->spanHz;
                }
                view.ctun = true;
                view.paneCtun = true;
                view.extended = false;
                apply(view, true);
            }
        }
    }
    publish();
}

void BandCenterController::toggle(bool active)
{
    m_reason.clear();
    followBand();
    auto* slice = m_receiver();
    if (!slice || m_band.isEmpty() || m_pending.isActive() || m_model->isTransmitting()
        || !m_model->ownsLocalDsp() || !m_model->isConnected() || !m_pan->ctunHeldReason().isEmpty()) {
        publish(QStringLiteral("Center view is unavailable for this receiver")); return;
    }
    auto& memory = m_memory[m_band];
    if (active == memory.active) { publish(); return; }
    if (!active) { apply(memory.before, false); return; }
    const auto [low, high] = m_pan->regionalBandBounds();
    const auto plan = planBandCenter(low, high, slice->frequency(), slice->sampleRateHz(), slice->filterLow(), slice->filterHigh());
    if (!plan) { publish(QStringLiteral("The receive passband cannot fit the selected sample rate")); return; }
    memory.before = snapshot();
    auto view = memory.before;
    view.centre = view.ddc = plan->centreHz;
    view.span = plan->spanHz;
    view.ctun = true;
    view.paneCtun = true;
    view.extended = false;
    apply(view, true);
}

void BandCenterController::apply(View target, bool active)
{
    auto* slice = m_receiver();
    if (!slice || m_model->isTransmitting()) { publish(); return; }
    //DH1KLM: Do not move another receiver, its filters, or a PureSignal feedback capture.
    for (auto* other : m_model->slices()) {
        if (other != slice && other->streamIndex() == slice->streamIndex()) {
            publish(QStringLiteral("Center view requires an unshared receive stream")); return;
        }
    }
    //DH1KLM: Keep today's tuned signal if tuning changed since the original snapshot.
    const double minimum = slice->frequency() + std::max(0, slice->filterHigh()) - target.rate * 0.46;
    const double maximum = slice->frequency() + std::min(0, slice->filterLow()) + target.rate * 0.46;
    if (minimum > maximum || target.rate < 1000) { publish(QStringLiteral("Previous capture cannot contain this receive filter")); return; }
    const double ddc = std::round(std::clamp(target.ddc, minimum, maximum));
    target.centre += ddc - target.ddc;
    target.ddc = ddc;
    m_target = target;
    m_targetActive = active;
    m_slice = slice->sliceIndex();
    m_stream = slice->streamIndex();
    m_attempts = 0;
    m_pending.start();
    //DH1KLM: C uses the selected rate. Only undo an explicit intervening rate change on toggle-off.
    if (slice->sampleRateHz() != target.rate && !m_model->setStreamSampleRate(m_stream, target.rate)) {
        m_pending.stop(); publish(QStringLiteral("Previous sample rate could not be restored")); return;
    }
    finish();
}

void BandCenterController::finish()
{
    if (!m_pending.isActive() || !m_model || !m_pan) { return; }
    auto* slice = m_receiver();
    if (!slice || slice->sliceIndex() != m_slice || slice->streamIndex() != m_stream) {
        m_pending.stop(); publish(QStringLiteral("Receiver changed; center operation cancelled")); return;
    }
    //DH1KLM: Never key, dekey, reconfigure TX, or change PureSignal; wait until RX before finishing.
    if (m_model->isTransmitting()) { publish(); return; }
    if (slice->sampleRateHz() != m_target.rate) {
        if (++m_attempts > 200) { m_pending.stop(); publish(QStringLiteral("Sample-rate restoration did not complete; previous view retained")); }
        return;
    }
    m_applying = true;
    const bool accepted = m_model->requestStreamCentre(m_slice, m_target.ddc)
        && m_model->requestStreamCtunPinned(m_slice, m_target.ctun);
    if (accepted) {
        m_reason.clear();
        m_pan->applyRemoteCtunState(m_pan->ctunAvailable(), m_target.paneCtun);
        m_pan->setExtendedViewAllowed(m_target.extended);
        m_pan->setDisplayWindowClamped(m_target.centre, m_target.span);
        m_pan->requestAutoZoomReplan();
        m_memory[m_band].active = m_targetActive;
        m_memory[m_band].view = snapshot();
        saveMemory();
    }
    m_applying = false;
    m_pending.stop();
    publish(accepted ? QString() : QStringLiteral("Receiver refused the center change; saved view retained"));
}

void BandCenterController::saveMemory() const
{
    //DH1KLM: Operator-local, separated by radio, pan, receiver and regional band; no TX state is stored.
    QJsonObject records;
    const auto encode = [](const View& view) {
        return QJsonArray{view.centre, view.span, view.ddc, view.rate, view.ctun, view.extended, view.paneCtun};
    };
    for (auto it = m_memory.cbegin(); it != m_memory.cend(); ++it) {
        records.insert(it.key(), QJsonObject{{"before", encode(it->before)}, {"view", encode(it->view)}, {"active", it->active}});
    }
    AppSettings::instance().setValue(QStringLiteral("DisplayBandCenterMemory_%1").arg(m_pan->panIndex()),
        QString::fromUtf8(QJsonDocument(records).toJson(QJsonDocument::Compact)));
}

void BandCenterController::loadMemory()
{
    const auto records = QJsonDocument::fromJson(AppSettings::instance()
        .value(QStringLiteral("DisplayBandCenterMemory_%1").arg(m_pan->panIndex())).toString().toUtf8()).object();
    const auto decode = [](const QJsonArray& data) -> std::optional<View> {
        if (data.size() != 7) { return std::nullopt; }
        View view{data[0].toDouble(-1), data[1].toDouble(-1), data[2].toDouble(-1),
            data[3].toInt(), data[4].toBool(), data[5].toBool(), data[6].toBool()};
        if (!std::isfinite(view.centre) || !std::isfinite(view.ddc) || !std::isfinite(view.span)
            || view.centre <= 0 || view.ddc <= 0 || view.span < 1000 || view.span > 200'000'000
            || view.rate < 48000 || view.rate > 3'072'000) { return std::nullopt; }
        return view;
    };
    if (records.size() > 128) { return; }
    for (auto it = records.begin(); it != records.end(); ++it) {
        const auto record = it.value().toObject();
        const auto before = decode(record.value("before").toArray());
        const auto view = decode(record.value("view").toArray());
        if (before && view) { m_memory.insert(it.key(), {*before, *view, record.value("active").toBool()}); }
    }
}
}
