#pragma once
#include <QDialog>
class QTreeWidget;
class QTreeWidgetItem;
class QStackedWidget;

namespace NereusSDR {
class RadioModel;
class SetupPage;

class SetupDialog : public QDialog {
    Q_OBJECT
public:
    explicit SetupDialog(RadioModel* model, QWidget* parent = nullptr);
private:
    void buildTree();
    void addPage(QTreeWidgetItem* parent, const QString& name, SetupPage* page);
    RadioModel* m_model;
    QTreeWidget* m_tree = nullptr;
    QStackedWidget* m_stack = nullptr;
};
} // namespace NereusSDR
