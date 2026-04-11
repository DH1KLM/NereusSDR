#include "CatApplet.h"
#include "NyiOverlay.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

namespace NereusSDR {

CatApplet::CatApplet(RadioModel* model, QWidget* parent)
    : AppletWidget(model, parent)
{
    buildUI();
}

void CatApplet::buildUI()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(4, 2, 4, 2);
    root->setSpacing(2);

    // rigctld row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QPushButton* rigctldBtn = new QPushButton(QStringLiteral("rigctld"), this);
        rigctldBtn->setCheckable(true);
        rigctldBtn->setFixedHeight(20);
        NyiOverlay::markNyi(rigctldBtn, QStringLiteral("Phase 3K"));
        row->addWidget(rigctldBtn);

        QLineEdit* rigctldPort = new QLineEdit(QStringLiteral("4532"), this);
        rigctldPort->setFixedHeight(22);
        rigctldPort->setPlaceholderText(QStringLiteral("Port"));
        NyiOverlay::markNyi(rigctldPort, QStringLiteral("Phase 3K"));
        row->addWidget(rigctldPort, 1);

        root->addLayout(row);
    }

    // TCI row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QPushButton* tciBtn = new QPushButton(QStringLiteral("TCI"), this);
        tciBtn->setCheckable(true);
        tciBtn->setFixedHeight(20);
        NyiOverlay::markNyi(tciBtn, QStringLiteral("Phase 3K"));
        row->addWidget(tciBtn);

        QLineEdit* tciPort = new QLineEdit(QStringLiteral("40001"), this);
        tciPort->setFixedHeight(22);
        tciPort->setPlaceholderText(QStringLiteral("Port"));
        NyiOverlay::markNyi(tciPort, QStringLiteral("Phase 3K"));
        row->addWidget(tciPort, 1);

        root->addLayout(row);
    }

    // TCP CAT row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QPushButton* tcpCatBtn = new QPushButton(QStringLiteral("TCP CAT"), this);
        tcpCatBtn->setCheckable(true);
        tcpCatBtn->setFixedHeight(20);
        NyiOverlay::markNyi(tcpCatBtn, QStringLiteral("Phase 3K"));
        row->addWidget(tcpCatBtn);

        QLineEdit* tcpCatPort = new QLineEdit(QStringLiteral("13013"), this);
        tcpCatPort->setFixedHeight(22);
        tcpCatPort->setPlaceholderText(QStringLiteral("Port"));
        NyiOverlay::markNyi(tcpCatPort, QStringLiteral("Phase 3K"));
        row->addWidget(tcpCatPort, 1);

        root->addLayout(row);
    }

    // Connections label row
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(4);

        QLabel* connLbl = new QLabel(QStringLiteral("Connections:"), this);
        connLbl->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        row->addWidget(connLbl);

        QLabel* connVal = new QLabel(QStringLiteral("0"), this);
        connVal->setStyleSheet(QStringLiteral("color: #708090; font-size: 9px;"));
        NyiOverlay::markNyi(connVal, QStringLiteral("Phase 3K"));
        row->addWidget(connVal, 1);

        root->addLayout(row);
    }

    root->addStretch();
}

void CatApplet::syncFromModel() { /* NYI */ }

} // namespace NereusSDR
