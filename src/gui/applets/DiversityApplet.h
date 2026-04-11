#pragma once
#include "AppletWidget.h"

class QPushButton;
class QComboBox;
class QSlider;

namespace NereusSDR {

class DiversityApplet : public AppletWidget {
    Q_OBJECT
public:
    explicit DiversityApplet(RadioModel* model, QWidget* parent = nullptr);
    QString appletId() const override { return QStringLiteral("DIV"); }
    QString appletTitle() const override { return QStringLiteral("Diversity"); }
    void syncFromModel() override;
private:
    void buildUI();
};

} // namespace NereusSDR
