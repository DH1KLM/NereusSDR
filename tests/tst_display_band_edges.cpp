// no-port-check: NereusSDR-original display consumer regressions, GPLv3.
// Modification history (NereusSDR):
// 2026-10-12 J.J. Boyd (KG4VCF), AI-assisted via OpenAI Codex.
#include <QtTest>
#include <QImage>
#include <QPainter>
#include <limits>
#include "core/AppSettings.h"
#include "gui/SpectrumWidget.h"
#include "models/Band.h"
#include "models/BandPlanEdges.h"
#include "models/BandPlanManager.h"
using namespace NereusSDR;

class TestDisplayBandEdges : public QObject {
    Q_OBJECT
private slots:
    void init() { AppSettings::instance().clear(); }
    void cleanup() { AppSettings::instance().clear(); }

    void lookupUsesExistingUnionIdentityBoundaries() {
        QCOMPARE(hamBandEdgesHz(13'900'000, 14'400'000),
                 (QVector<double>{14'000'000, 14'350'000}));
        QCOMPARE(hamBandEdgesHz(6'900'000, 7'400'000),
                 (QVector<double>{7'000'000, 7'300'000}));
        QCOMPARE(hamBandEdgesHz(143'000'000, 149'000'000),
                 (QVector<double>{144'000'000, 148'000'000}));
        QVERIFY(hamBandEdgesHz(14'360'000, 14'400'000).isEmpty());
        QVERIFY(hamBandEdgesHz(5'000'000, 5'010'000).isEmpty()); // WWV is not a ham boundary
        QVERIFY(hamBandEdgesHz(std::numeric_limits<double>::quiet_NaN(), 14'400'000).isEmpty());
        QVERIFY(hamBandEdgesHz(14'400'000, 13'900'000).isEmpty());
    }

    void actualFrequencyGridDrawsBandEdgeWithItsSelectedColor() {
        BandPlanManager plans;
        plans.loadPlans();
        plans.setActivePlan(QStringLiteral("ARRL (US)"));
        SpectrumWidget pan;
        pan.setBandPlanManager(&plans);
        pan.setBandPlanFontSize(0);
        pan.setFrequencyRange(14'175'000, 500'000);
        pan.setGridColor(Qt::transparent);
        pan.setGridFineColor(Qt::transparent);
        pan.setHGridColor(Qt::transparent);
        const QColor chosen(0x9a, 0xe9, 0x45);
        pan.setBandEdgeColor(chosen);
        QImage image(400, 100, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::black);
        {
            QPainter painter(&image);
            pan.drawFrequencyGridForTest(painter, image.rect());
        }
        QCOMPARE(image.pixelColor(60, 50), chosen); // existing 20m low edge
        QCOMPARE(image.pixelColor(340, 50), chosen); // existing 20m high edge
        QCOMPARE(image.pixelColor(200, 50), QColor(Qt::black));
        const QColor next(0x12, 0x34, 0x56);
        pan.setBandEdgeColor(next);
        image.fill(Qt::black);
        {
            QPainter painter(&image);
            pan.drawFrequencyGridForTest(painter, image.rect());
        }
        QCOMPARE(image.pixelColor(60, 50), next);
        QCOMPARE(image.pixelColor(340, 50), next);
    }

    void boundariesAtBothViewportEndsRemainVisible() {
        BandPlanManager plans;
        plans.loadPlans();
        plans.setActivePlan(QStringLiteral("ARRL (US)"));
        SpectrumWidget pan;
        pan.setBandPlanManager(&plans);
        pan.setBandPlanFontSize(0);
        pan.setFrequencyRange(14'175'000, 350'000);
        pan.setGridColor(Qt::transparent);
        pan.setGridFineColor(Qt::transparent);
        pan.setHGridColor(Qt::transparent);
        const QColor chosen(0x9a, 0xe9, 0x45);
        pan.setBandEdgeColor(chosen);
        QImage image(400, 100, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::black);
        QPainter painter(&image);
        pan.drawFrequencyGridForTest(painter, image.rect());
        painter.end();
        QCOMPARE(image.pixelColor(0, 50), chosen);
        QCOMPARE(image.pixelColor(399, 50), chosen);
    }

    void noBoundaryIsInventedOutsideHamCoverage() {
        BandPlanManager plans;
        plans.loadPlans();
        SpectrumWidget pan;
        pan.setBandPlanManager(&plans);
        pan.setFrequencyRange(5'000'000, 10'000);
        pan.setGridColor(Qt::transparent);
        pan.setGridFineColor(Qt::transparent);
        pan.setHGridColor(Qt::transparent);
        pan.setBandEdgeColor(QColor(Qt::red));
        QImage image(400, 100, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::black);
        QPainter painter(&image);
        pan.drawFrequencyGridForTest(painter, image.rect());
        painter.end();
        for (int x = 0; x < image.width(); ++x) {
            QCOMPARE(image.pixelColor(x, 50), QColor(Qt::black));
        }
    }

    void regionalEdgesComeFromTheSelectedPlan() {
        BandPlanManager plans;
        plans.loadPlans();
        plans.setActivePlan(QStringLiteral("IARU Region 1"));
        QCOMPARE(regionalBandEdgesHz(plans.segments(), 3'400'000, 4'100'000),
                 (QVector<double>{3'500'000, 3'800'000}));
        QCOMPARE(regionalBandEdgesHz(plans.segments(), 6'900'000, 7'400'000),
                 (QVector<double>{7'000'000, 7'200'000}));
        QCOMPARE(regionalBandEdgesHz(plans.segments(), 5'300'000, 5'500'000),
                 (QVector<double>{5'351'500, 5'366'500}));
        QVERIFY(regionalBandEdgesHz(plans.segments(), 0, 1'000'000).isEmpty());
        plans.setActivePlan(QStringLiteral("ARRL (US)"));
        QCOMPARE(regionalBandEdgesHz(plans.segments(), 3'400'000, 4'100'000),
                 (QVector<double>{3'500'000, 4'000'000}));
        QCOMPARE(regionalBandEdgesHz(plans.segments(), 100'000, 150'000),
                 (QVector<double>{135'700, 137'800}));
        QVERIFY(regionalBandEdgesHz({}, 0, 30'000'000).isEmpty());
        QVERIFY(regionalBandEdgesHz(plans.segments(), 4'000'000, 3'000'000).isEmpty());
    }

    void regionSwitchMovesThinRedLinesWithBandStripHidden() {
        BandPlanManager plans;
        plans.loadPlans();
        plans.setActivePlan(QStringLiteral("IARU Region 1"));
        SpectrumWidget pan;
        pan.setBandPlanManager(&plans);
        pan.setBandPlanFontSize(0);
        pan.setFrequencyRange(3'750'000, 1'000'000);
        pan.setGridColor(Qt::transparent);
        pan.setGridFineColor(Qt::transparent);
        pan.setHGridColor(Qt::transparent);
        QImage image(400, 100, QImage::Format_ARGB32_Premultiplied);
        for (const bool us : {false, true}) {
            if (us) { plans.setActivePlan(QStringLiteral("ARRL (US)")); }
            image.fill(Qt::black);
            QPainter painter(&image);
            pan.drawFrequencyGridForTest(painter, image.rect());
            painter.end();
            QCOMPARE(image.pixelColor(100, 50), QColor(Qt::red));
            QCOMPARE(image.pixelColor(us ? 300 : 220, 50), QColor(Qt::red));
            QCOMPARE(image.pixelColor(us ? 220 : 300, 50), QColor(Qt::black));
            QCOMPARE(image.pixelColor(99, 50), QColor(Qt::black));
            QCOMPARE(image.pixelColor(101, 50), QColor(Qt::black));
        }
    }
};
QTEST_MAIN(TestDisplayBandEdges)
#include "tst_display_band_edges.moc"
