#include "SetupPage.h"

#include <QVBoxLayout>
#include <QLabel>

namespace NereusSDR {

SetupPage::SetupPage(const QString& title, RadioModel* model, QWidget* parent)
    : QWidget(parent)
    , m_title(title)
    , m_model(model)
{
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(12, 8, 12, 8);
    outerLayout->setSpacing(6);

    auto* titleLabel = new QLabel(title, this);
    titleLabel->setStyleSheet(QStringLiteral(
        "QLabel {"
        "  color: #c8d8e8;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "  border-bottom: 1px solid #304050;"
        "  padding-bottom: 4px;"
        "}"
    ));
    outerLayout->addWidget(titleLabel);

    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(6);
    outerLayout->addLayout(m_contentLayout);

    outerLayout->addStretch();
}

void SetupPage::syncFromModel()
{
    // Default: nothing to sync. Subclasses override as needed.
}

QLabel* SetupPage::addSectionHeader(const QString& name, int wired, int total)
{
    auto* label = new QLabel(
        QStringLiteral("%1 (%2/%3 wired)").arg(name).arg(wired).arg(total),
        this);
    label->setStyleSheet(QStringLiteral(
        "QLabel {"
        "  color: #8aa8c0;"
        "  font-size: 12px;"
        "  font-weight: bold;"
        "  margin-top: 8px;"
        "}"
    ));
    m_contentLayout->addWidget(label);
    return label;
}

} // namespace NereusSDR
