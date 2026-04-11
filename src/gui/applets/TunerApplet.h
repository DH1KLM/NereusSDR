#pragma once
#include "AppletWidget.h"

class QPushButton;
class QLabel;

namespace NereusSDR {

class TunerApplet : public AppletWidget {
    Q_OBJECT
public:
    explicit TunerApplet(RadioModel* model, QWidget* parent = nullptr);
    QString appletId() const override { return QStringLiteral("TUN"); }
    QString appletTitle() const override { return QStringLiteral("ATU Tuner"); }
    void syncFromModel() override;
private:
    void buildUI();
};

} // namespace NereusSDR
