#pragma once
#include "AppletWidget.h"

class QPushButton;
class QComboBox;

namespace NereusSDR {

class DigitalApplet : public AppletWidget {
    Q_OBJECT
public:
    explicit DigitalApplet(RadioModel* model, QWidget* parent = nullptr);
    QString appletId() const override { return QStringLiteral("DIG"); }
    QString appletTitle() const override { return QStringLiteral("Digital / VAC"); }
    void syncFromModel() override;
private:
    void buildUI();
};

} // namespace NereusSDR
