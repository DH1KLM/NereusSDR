#pragma once
#include "AppletWidget.h"

class QPushButton;
class QSlider;
class QComboBox;
class QLabel;

namespace NereusSDR {

class TxApplet : public AppletWidget {
    Q_OBJECT
public:
    explicit TxApplet(RadioModel* model, QWidget* parent = nullptr);
    QString appletId() const override { return QStringLiteral("TX"); }
    QString appletTitle() const override { return QStringLiteral("TX Controls"); }
    void syncFromModel() override;
private:
    void buildUI();
};

} // namespace NereusSDR
