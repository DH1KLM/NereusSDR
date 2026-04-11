#pragma once

#include <QLabel>

namespace NereusSDR {

// Small badge widget showing "NYI" with a tooltip indicating which phase enables it.
// Usage: auto* badge = new NyiOverlay("Phase 3I-1", parentWidget);
//        badge->attachTo(someControl);  // positions top-right of control
class NyiOverlay : public QLabel {
    Q_OBJECT

public:
    explicit NyiOverlay(const QString& phaseHint, QWidget* parent = nullptr);

    // Position this badge at the top-right corner of the target widget.
    void attachTo(QWidget* target);

    // Convenience: disable a widget and attach an NYI badge to it.
    static NyiOverlay* markNyi(QWidget* target, const QString& phaseHint);
};

} // namespace NereusSDR
