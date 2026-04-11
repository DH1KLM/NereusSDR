#pragma once
#include "AppletWidget.h"

class QPushButton;
class QLabel;

namespace NereusSDR {

class PureSignalApplet : public AppletWidget {
    Q_OBJECT
public:
    explicit PureSignalApplet(RadioModel* model, QWidget* parent = nullptr);
    QString appletId() const override { return QStringLiteral("PS"); }
    QString appletTitle() const override { return QStringLiteral("PureSignal"); }
    void syncFromModel() override;
private:
    void buildUI();
};

} // namespace NereusSDR
