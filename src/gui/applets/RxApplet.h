#pragma once

#include "AppletWidget.h"

class QComboBox;
class QSlider;
class QPushButton;
class QLabel;

namespace NereusSDR {

class SliceModel;

class RxApplet : public AppletWidget {
    Q_OBJECT

public:
    explicit RxApplet(RadioModel* model, QWidget* parent = nullptr);

    QString appletId() const override { return QStringLiteral("RX"); }
    QString appletTitle() const override { return QStringLiteral("RX Controls"); }
    void syncFromModel() override;

    void setSlice(SliceModel* slice);

private:
    void buildUI();
    void wireSlice();

    SliceModel* m_slice = nullptr;

    // Tier 1 controls (wired)
    QLabel*       m_sliceBadge = nullptr;
    QPushButton*  m_lockBtn = nullptr;
    QComboBox*    m_modeCombo = nullptr;
    QComboBox*    m_agcCombo = nullptr;
    QSlider*      m_agcThreshold = nullptr;
    QSlider*      m_afGain = nullptr;
    QPushButton*  m_muteBtn = nullptr;
    QLabel*       m_filterLabel = nullptr;

    // Tier 2 controls (NYI — built but disabled)
    QSlider*      m_squelchSlider = nullptr;
    QPushButton*  m_squelchBtn = nullptr;
    QSlider*      m_panSlider = nullptr;
};

} // namespace NereusSDR
