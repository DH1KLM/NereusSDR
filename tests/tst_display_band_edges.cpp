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
#include "models/BandPlanManager.h"
#include "gui/setup/GeneralOptionsPage.h"
#include <QCheckBox>
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

    void regionSettingOverridesTheIndependentOverlayPlan() {
        BandPlanManager plans;
        plans.loadPlans();
        plans.setActivePlan(QStringLiteral("ARRL (US)"));
        SpectrumWidget pan;
        pan.setBandPlanManager(&plans);
        pan.setBandPlanFontSize(0);
        pan.setGridColor(Qt::transparent);
        pan.setGridFineColor(Qt::transparent);
        pan.setHGridColor(Qt::transparent);
        AppSettings::instance().setValue(QStringLiteral("BandPlanRegion"), QStringLiteral("23"));
        pan.setFrequencyRange(7'100'000, 400'000);
        QImage image(400, 100, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::black);
        QPainter painter(&image);
        pan.drawFrequencyGridForTest(painter, image.rect());
        painter.end();
        QCOMPARE(image.pixelColor(100, 50), QColor(Qt::red));
        QCOMPARE(image.pixelColor(300, 50), QColor(Qt::red));
        QCOMPARE(image.pixelColor(399, 50), QColor(Qt::black));
        plans.setActivePlan(QStringLiteral("IARU Region 3"));
        QImage changed = image;
        changed.fill(Qt::black);
        QPainter second(&changed);
        pan.drawFrequencyGridForTest(second, changed.rect());
        second.end();
        QCOMPARE(changed, image);
    }

    void zoomAndPanKeepTheSameAbsoluteBoundary() {
        SpectrumWidget pan;
        pan.setBandPlanFontSize(0);
        pan.setGridColor(Qt::transparent);
        pan.setGridFineColor(Qt::transparent);
        pan.setHGridColor(Qt::transparent);
        AppSettings::instance().setValue(QStringLiteral("BandPlanRegion"), QStringLiteral("23"));
        for (const double bandwidth : {400'000.0, 200'000.0, 100'000.0, 40'000.0}) {
            const double centre = 7'200'000.0 - bandwidth / 4.0;
            pan.setFrequencyRange(centre, bandwidth);
            QImage image(400, 100, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::black);
            QPainter painter(&image);
            pan.drawFrequencyGridForTest(painter, image.rect());
            painter.end();
            QCOMPARE(image.pixelColor(300, 50), QColor(Qt::red));
            QCOMPARE(image.pixelColor(299, 50), QColor(Qt::black));
            QCOMPARE(image.pixelColor(301, 50), QColor(Qt::black));
            pan.setFrequencyRange(centre - bandwidth, bandwidth);
            image.fill(Qt::black);
            QPainter outside(&image);
            pan.drawFrequencyGridForTest(outside, image.rect());
            outside.end();
            QCOMPARE(image.pixelColor(399, 50), QColor(Qt::black));
        }
    }

    void regionBoundaries_data() {
        QTest::addColumn<int>("region");
        QTest::addColumn<double>("low");
        QTest::addColumn<double>("high");
        QTest::newRow("Germany-80m") << 23 << 3'500'000.0 << 3'800'000.0;
        QTest::newRow("US-80m") << 8 << 3'500'000.0 << 4'000'000.0;
        QTest::newRow("Region1-40m") << 20 << 7'000'000.0 << 7'200'000.0;
        QTest::newRow("Region2-40m") << 21 << 7'000'000.0 << 7'300'000.0;
        QTest::newRow("Region3-80m") << 22 << 3'500'000.0 << 3'900'000.0;
        QTest::newRow("Germany-60m") << 23 << 5'351'500.0 << 5'366'500.0;
        QTest::newRow("US-60m") << 8 << 5'330'500.0 << 5'406'400.0;
        QTest::newRow("Germany-6m") << 23 << 50'000'000.0 << 51'000'000.0;
        QTest::newRow("Spain-6m") << 6 << 50'000'000.0 << 52'000'000.0;
        QTest::newRow("Germany-2m") << 23 << 144'000'000.0 << 146'000'000.0;
        QTest::newRow("US-2m") << 8 << 144'000'000.0 << 148'000'000.0;
    }

    void regionBoundaries() {
        QFETCH(int, region);
        QFETCH(double, low);
        QFETCH(double, high);
        AppSettings::instance().setValue(QStringLiteral("BandPlanRegion"), QString::number(region));
        SpectrumWidget pan;
        pan.setBandPlanFontSize(0);
        pan.setGridColor(Qt::transparent);
        pan.setGridFineColor(Qt::transparent);
        pan.setHGridColor(Qt::transparent);
        pan.setFrequencyRange((low + high) / 2.0, (high - low) * 2.0);
        QImage image(400, 100, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::black);
        QPainter painter(&image);
        pan.drawFrequencyGridForTest(painter, image.rect());
        painter.end();
        QCOMPARE(image.pixelColor(100, 50), QColor(Qt::red));
        QCOMPARE(image.pixelColor(300, 50), QColor(Qt::red));
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
            AppSettings::instance().setValue(QStringLiteral("BandPlanRegion"), us ? QStringLiteral("8") : QStringLiteral("23"));
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

    void switchBesideRegionPersistsAndControlsLines() {
        GeneralOptionsPage page(nullptr);
        auto* toggle = page.findChild<QCheckBox*>(QStringLiteral("chkDisplayBandEdges"));
        auto* region = page.findChild<QWidget*>(QStringLiteral("comboFRSRegion"));
        QVERIFY(toggle);
        QVERIFY(region);
        QCOMPARE(toggle->parentWidget(), region->parentWidget());
        QVERIFY(toggle->isEnabled());
        QVERIFY(toggle->isChecked());
        page.resize(700, 950);
        page.show();
        QCoreApplication::processEvents();
        QVERIFY(toggle->geometry().left() > region->geometry().right());
        QCOMPARE(toggle->geometry().center().y(), region->geometry().center().y());
        SpectrumWidget pan;
        pan.setBandPlanFontSize(0);
        pan.setGridColor(Qt::transparent);
        pan.setGridFineColor(Qt::transparent);
        pan.setHGridColor(Qt::transparent);
        pan.setFrequencyRange(3'750'000, 1'000'000);
        AppSettings::instance().setValue(QStringLiteral("BandPlanRegion"), QStringLiteral("23"));
        for (const bool enabled : {false, true}) {
            toggle->setChecked(enabled);
            QCOMPARE(AppSettings::instance().value(QStringLiteral("DisplayBandEdges")).toString(),
                     enabled ? QStringLiteral("True") : QStringLiteral("False"));
            QImage image(400, 100, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::black);
            QPainter painter(&image);
            pan.drawFrequencyGridForTest(painter, image.rect());
            painter.end();
            QCOMPARE(image.pixelColor(100, 50), QColor(enabled ? Qt::red : Qt::black));
            QCOMPARE(image.pixelColor(220, 50), QColor(enabled ? Qt::red : Qt::black));
        }
        toggle->setChecked(false);
        GeneralOptionsPage reopened(nullptr);
        QVERIFY(!reopened.findChild<QCheckBox*>(QStringLiteral("chkDisplayBandEdges"))->isChecked());
    }

    void nativeGpuRefreshesRegionWithoutZoom() {
#ifdef NEREUS_GPU_SPECTRUM
        if (!qEnvironmentVariableIsSet("NEREUS_TEST_NATIVE_BAND_EDGES")) {
            QSKIP("Native GPU check requires an available Windows display.");
        }
        SpectrumWidget pan;
        pan.resize(400, 250);
        pan.setDbmScaleVisible(false);
        pan.setBandPlanFontSize(0);
        pan.setGridColor(Qt::transparent);
        pan.setGridFineColor(Qt::transparent);
        pan.setHGridColor(Qt::transparent);
        pan.setFrequencyRange(3'750'000, 1'000'000);
        AppSettings::instance().setValue(QStringLiteral("BandPlanRegion"), QStringLiteral("23"));
        pan.show();
        QVERIFY(QTest::qWaitForWindowExposed(&pan));
        QImage german;
        QTRY_VERIFY_WITH_TIMEOUT(!(german = pan.grabFramebuffer()).isNull(), 10000);
        const int y = qRound(pan.spectrumRectForTest().height() * german.height() / (2.0 * pan.height()));
        const auto redAt = [y](const QImage& image, double fraction) {
            const int x = qRound(image.width() * fraction);
            for (int dx = -1; dx <= 1; ++dx) {
                const QColor color = image.pixelColor(x + dx, y);
                if (color.red() > 200 && color.green() < 60 && color.blue() < 60) { return true; }
            }
            return false;
        };
        QVERIFY(redAt(german, 0.55));
        AppSettings::instance().setValue(QStringLiteral("BandPlanRegion"), QStringLiteral("8"));
        pan.update();
        QImage american;
        QTRY_VERIFY_WITH_TIMEOUT(!(american = pan.grabFramebuffer()).isNull() && redAt(american, 0.75), 10000);
        QVERIFY(!redAt(american, 0.55));
        AppSettings::instance().setValue(QStringLiteral("BandPlanRegion"), QStringLiteral("23"));
        pan.setFrequencyRange(7'210'000, 40'000);
        QImage zoomed;
        QTRY_VERIFY_WITH_TIMEOUT(!(zoomed = pan.grabFramebuffer()).isNull() && redAt(zoomed, 0.25), 10000);
        QVERIFY(!redAt(zoomed, 0.75));
        pan.setFrequencyRange(7'100'000, 40'000);
        QImage inside;
        QTRY_VERIFY_WITH_TIMEOUT(!(inside = pan.grabFramebuffer()).isNull() && !redAt(inside, 0.25), 10000);
        QVERIFY(!redAt(inside, 0.75));
        pan.setFrequencyRange(3'750'000, 1'000'000);
        GeneralOptionsPage page(nullptr);
        auto* toggle = page.findChild<QCheckBox*>(QStringLiteral("chkDisplayBandEdges"));
        QVERIFY(toggle);
        QImage switched;
        QTRY_VERIFY_WITH_TIMEOUT(!(switched = pan.grabFramebuffer()).isNull() && redAt(switched, 0.55), 10000);
        toggle->setChecked(false);
        QTRY_VERIFY_WITH_TIMEOUT(!(switched = pan.grabFramebuffer()).isNull() && !redAt(switched, 0.55), 10000);
        toggle->setChecked(true);
        QTRY_VERIFY_WITH_TIMEOUT(!(switched = pan.grabFramebuffer()).isNull() && redAt(switched, 0.55), 10000);
        pan.close();
#endif
    }
};
QTEST_MAIN(TestDisplayBandEdges)
#include "tst_display_band_edges.moc"
