#pragma once
#include "AppletWidget.h"

class QPushButton;

namespace NereusSDR {

class DvkApplet : public AppletWidget {
    Q_OBJECT
public:
    explicit DvkApplet(RadioModel* model, QWidget* parent = nullptr);
    QString appletId() const override { return QStringLiteral("DVK"); }
    QString appletTitle() const override { return QStringLiteral("Voice Keyer"); }
    void syncFromModel() override;
private:
    void buildUI();
};

} // namespace NereusSDR
