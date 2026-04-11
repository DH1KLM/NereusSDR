#pragma once
#include "AppletWidget.h"

class QPushButton;
class QComboBox;
class QDoubleSpinBox;

namespace NereusSDR {

class FmApplet : public AppletWidget {
    Q_OBJECT
public:
    explicit FmApplet(RadioModel* model, QWidget* parent = nullptr);
    QString appletId() const override { return QStringLiteral("FM"); }
    QString appletTitle() const override { return QStringLiteral("FM Controls"); }
    void syncFromModel() override;
private:
    void buildUI();
};

} // namespace NereusSDR
