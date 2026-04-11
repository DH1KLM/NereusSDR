#pragma once
#include "AppletWidget.h"

class QPushButton;
class QSlider;
class QStackedWidget;

namespace NereusSDR {

class PhoneCwApplet : public AppletWidget {
    Q_OBJECT
public:
    explicit PhoneCwApplet(RadioModel* model, QWidget* parent = nullptr);
    QString appletId() const override { return QStringLiteral("PHCW"); }
    QString appletTitle() const override { return QStringLiteral("Phone / CW"); }
    void syncFromModel() override;
private:
    void buildUI();

    QStackedWidget* m_stack = nullptr;
    QPushButton*    m_phoneTab = nullptr;
    QPushButton*    m_cwTab = nullptr;
};

} // namespace NereusSDR
