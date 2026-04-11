#pragma once
#include "AppletWidget.h"

class QPushButton;
class QSlider;
class QComboBox;

namespace NereusSDR {

class EqApplet : public AppletWidget {
    Q_OBJECT
public:
    explicit EqApplet(RadioModel* model, QWidget* parent = nullptr);
    QString appletId() const override { return QStringLiteral("EQ"); }
    QString appletTitle() const override { return QStringLiteral("Equalizer"); }
    void syncFromModel() override;
private:
    void buildUI();
};

} // namespace NereusSDR
