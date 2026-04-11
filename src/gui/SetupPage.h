#pragma once
#include <QWidget>
class QVBoxLayout;
class QLabel;

namespace NereusSDR {
class RadioModel;

class SetupPage : public QWidget {
    Q_OBJECT
public:
    explicit SetupPage(const QString& title, RadioModel* model, QWidget* parent = nullptr);
    virtual ~SetupPage() = default;
    QString pageTitle() const { return m_title; }
    virtual void syncFromModel();
protected:
    QVBoxLayout* contentLayout() { return m_contentLayout; }
    RadioModel* model() { return m_model; }
    QLabel* addSectionHeader(const QString& name, int wired, int total);
private:
    QString m_title;
    RadioModel* m_model;
    QVBoxLayout* m_contentLayout = nullptr;
};
} // namespace NereusSDR
