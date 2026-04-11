#pragma once
#include "AppletWidget.h"

class QPushButton;
class QTextEdit;

namespace NereusSDR {

class CwxApplet : public AppletWidget {
    Q_OBJECT
public:
    explicit CwxApplet(RadioModel* model, QWidget* parent = nullptr);
    QString appletId() const override { return QStringLiteral("CWX"); }
    QString appletTitle() const override { return QStringLiteral("CW Macros"); }
    void syncFromModel() override;
private:
    void buildUI();
};

} // namespace NereusSDR
