#pragma once
#include "AppletWidget.h"

class QPushButton;
class QLineEdit;
class QLabel;

namespace NereusSDR {

class CatApplet : public AppletWidget {
    Q_OBJECT
public:
    explicit CatApplet(RadioModel* model, QWidget* parent = nullptr);
    QString appletId() const override { return QStringLiteral("CAT"); }
    QString appletTitle() const override { return QStringLiteral("CAT / TCI"); }
    void syncFromModel() override;
private:
    void buildUI();
};

} // namespace NereusSDR
