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
#include <QPushButton>
#include <QWheelEvent>
#include <memory>
#include "gui/SpectrumOverlayPanel.h"
#include "gui/spectrum/BandCenterView.h"
#include "gui/spectrum/BandCenterController.h"
#include "models/RadioModel.h"
#include "models/SliceModel.h"
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

    void bandAndSegmentZoomNeverEmitReceiverRetunes() {
        BandPlanManager plans;
        plans.loadPlans();
        plans.setActivePlan(QStringLiteral("ARRL (US)"));
        SpectrumWidget pan;
        pan.setBandPlanManager(&plans);
        pan.setSampleRate(384000);
        pan.setDdcCenterFrequency(7'150'000);
        pan.setLocalWidebandAvailable(true);
        pan.setVfoFrequency(7'150'000);
        AppSettings::instance().setValue(QStringLiteral("BandPlanRegion"), QStringLiteral("23"));
        QSignalSpy retunes(&pan, &SpectrumWidget::centerChanged);
        QSignalSpy demand(&pan, &SpectrumWidget::widebandExtensionStateChanged);
        pan.zoomToBand();
        QCOMPARE(pan.centerFrequency(), 7'100'000.0);
        QCOMPARE(pan.bandwidth(), 208'000.0);
        pan.zoomToSegment();
        QCOMPARE(retunes.count(), 0);
        QCOMPARE(demand.count(), 0);
        QCOMPARE(pan.sampleRate(), 384'000.0);
    }

    void bandZoomCannotRequestWidebandOrStretchMissingIq() {
        SpectrumWidget pan;
        pan.setSampleRate(384000);
        pan.setDdcCenterFrequency(3'550'000);
        pan.setVfoFrequency(3'550'000);
        pan.setLocalWidebandAvailable(true);
        AppSettings::instance().setValue(QStringLiteral("BandPlanRegion"), QStringLiteral("8"));
        QSignalSpy retunes(&pan, &SpectrumWidget::centerChanged);
        QSignalSpy demand(&pan, &SpectrumWidget::widebandExtensionStateChanged);
        pan.zoomToBand();
        QCOMPARE(pan.bandwidth(), 384'000.0);
        QCOMPARE(pan.centerFrequency(), 3'550'000.0);
        QCOMPARE(retunes.count(), 0);
        QCOMPARE(demand.count(), 0);
    }

    void centerPlannerUsesSelected384kWithoutMovingReceiveFrequency() {
        const auto view = planBandCenter(14'000'000, 14'350'000, 14'214'200, 384000, 100, 3000);
        QVERIFY(view.has_value());
        QCOMPARE(view->centreHz, 14'175'000.0);
        QCOMPARE(view->spanHz, 357'000.0);
        QVERIFY(view->wholeBand);
        const auto wide = planBandCenter(3'500'000, 4'000'000, 3'550'000, 384000, -3000, -100);
        QVERIFY(wide.has_value());
        QVERIFY(!wide->wholeBand);
        QCOMPARE(wide->spanHz, 384'000.0);
        QVERIFY(3'547'000 >= wide->centreHz - 384000 * 0.46);
        QVERIFY(!planBandCenter(7'200'000, 7'000'000, 7'100'000, 384000, 100, 3000));
        QVERIFY(!planBandCenter(7'000'000, 7'200'000, 7'300'000, 384000, 100, 3000));
    }

    void automaticBandRates_data() {
        QTest::addColumn<double>("low");
        QTest::addColumn<double>("high");
        QTest::addColumn<int>("rate");
        QTest::newRow("160m-operator-choice") << 1'810'000.0 << 2'000'000.0 << 192000;
        QTest::newRow("80m") << 3'500'000.0 << 3'800'000.0 << 384000;
        QTest::newRow("60m") << 5'351'500.0 << 5'366'500.0 << 48000;
        QTest::newRow("40m") << 7'000'000.0 << 7'200'000.0 << 384000;
        QTest::newRow("30m") << 10'100'000.0 << 10'150'000.0 << 96000;
        QTest::newRow("20m") << 14'000'000.0 << 14'350'000.0 << 384000;
        QTest::newRow("17m") << 18'068'000.0 << 18'168'000.0 << 192000;
        QTest::newRow("15m") << 21'000'000.0 << 21'450'000.0 << 768000;
        QTest::newRow("12m") << 24'890'000.0 << 24'990'000.0 << 192000;
        QTest::newRow("10m-operator-choice") << 28'000'000.0 << 29'700'000.0 << 768000;
        QTest::newRow("6m-operator-choice") << 50'000'000.0 << 51'000'000.0 << 768000;
    }

    void automaticBandRates() {
        QFETCH(double, low);
        QFETCH(double, high);
        QFETCH(int, rate);
        const int supported[]{1536000, 192000, 48000, 768000, 96000, 384000};
        const double rx = (low + high) / 2.0;
        QCOMPARE(bandCenterSampleRate(low, high, rx, 192000, 100, 3000, supported), rate);
        const auto view = planBandCenter(low, high, rx, rate, 100, 3000);
        QVERIFY(view.has_value());
        QVERIFY(view->spanHz <= rate);
        if (high - low < rate * 0.9) { QVERIFY(view->wholeBand); }
        const int restricted[]{48000, 96000, 192000};
        QVERIFY(bandCenterSampleRate(low, high, rx, 192000, 100, 3000, restricted) <= 192000);
    }

    void buttonAndWheelZoomKeepOffsetCaptureAndNeverRetune() {
        SpectrumWidget pan;
        pan.resize(800, 400);
        pan.setSampleRate(384000);
        pan.setDdcCenterFrequency(14'200'000);
        pan.setVfoFrequency(14'380'000);
        pan.setFrequencyRange(14'200'000, 384000);
        pan.setLocalWidebandAvailable(true);
        QSignalSpy retunes(&pan, &SpectrumWidget::centerChanged);
        QSignalSpy demand(&pan, &SpectrumWidget::widebandExtensionStateChanged);
        pan.zoomBy(0.8);
        QVERIFY(pan.centerFrequency() + pan.bandwidth() / 2.0 <= 14'392'000);
        QVERIFY(pan.centerFrequency() - pan.bandwidth() / 2.0 >= 14'008'000);
        QWheelEvent wheel(QPointF(100, 50), QPointF(100, 50), QPoint(), QPoint(0, 120),
                          Qt::NoButton, Qt::ControlModifier, Qt::NoScrollPhase, false);
        QApplication::sendEvent(&pan, &wheel);
        QVERIFY(wheel.isAccepted());
        QVERIFY(pan.centerFrequency() + pan.bandwidth() / 2.0 <= 14'392'000);
        QCOMPARE(retunes.count(), 0);
        QCOMPARE(demand.count(), 0);
        QCOMPARE(pan.ddcCenterFrequency(), 14'200'000.0);
    }

    void bandThenMinusCannotReenableWidebandBypassAt192k() {
        SpectrumWidget pan;
        pan.resize(800, 400);
        pan.setSampleRate(192000);
        pan.setDdcCenterFrequency(14'225'000);
        pan.setVfoFrequency(14'225'000);
        pan.setLocalWidebandAvailable(true);
        pan.setFrequencyRange(14'225'000, 675000);
        QVERIFY(pan.extendedMode());
        QSignalSpy retunes(&pan, &SpectrumWidget::centerChanged);
        QSignalSpy demand(&pan, &SpectrumWidget::widebandExtensionStateChanged);
        pan.zoomToBand();
        QCOMPARE(pan.bandwidth(), 192'000.0);
        QVERIFY(!pan.extendedMode());
        QCOMPARE(demand.count(), 1);
        QCOMPARE(demand.first().first().toBool(), false);
        demand.clear();
        pan.zoomBy(1.5);
        QCOMPARE(pan.bandwidth(), 192'000.0);
        QVERIFY(!pan.extendedMode());
        QWheelEvent wheel(QPointF(100, 50), QPointF(100, 50), QPoint(), QPoint(0, -120),
                          Qt::NoButton, Qt::ControlModifier, Qt::NoScrollPhase, false);
        QApplication::sendEvent(&pan, &wheel);
        QCOMPARE(pan.bandwidth(), 192'000.0);
        QCOMPARE(demand.count(), 0);
        QCOMPARE(retunes.count(), 0);
        QCOMPARE(pan.ddcCenterFrequency(), 14'225'000.0);
    }

    void centerForcesBothLinesWithoutChangingSetupPreference() {
        AppSettings::instance().setValue(QStringLiteral("DisplayBandEdges"), QStringLiteral("False"));
        SpectrumWidget pan;
        pan.setBandPlanFontSize(0);
        pan.setFrequencyRange(14'175'000, 500000);
        pan.setGridColor(Qt::transparent);
        pan.setGridFineColor(Qt::transparent);
        pan.setHGridColor(Qt::transparent);
        pan.setBandEdgeColor(Qt::red);
        const auto draw = [&]() {
            QImage image(400, 100, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::black);
            QPainter painter(&image);
            pan.drawFrequencyGridForTest(painter, image.rect());
            return image;
        };
        QCOMPARE(draw().pixelColor(60, 50), QColor(Qt::black));
        pan.setBandCenterActive(true);
        QCOMPARE(draw().pixelColor(60, 50), QColor(Qt::red));
        QCOMPARE(draw().pixelColor(340, 50), QColor(Qt::red));
        QCOMPARE(AppSettings::instance().value(QStringLiteral("DisplayBandEdges")).toString(), QStringLiteral("False"));
        pan.setBandCenterActive(false);
        QCOMPARE(draw().pixelColor(60, 50), QColor(Qt::black));
    }

    void centerToggleRestoresViewRateAndPinWithoutRetuningSlice() {
        RadioModel model;
        model.configureStreamPool(1, 1, 384000);
        model.setConnectionStateForTest(ConnectionState::Connected);
        const int id = model.addSlice();
        auto* slice = model.sliceById(id);
        QVERIFY(slice);
        slice->setFrequency(14'214'200);
        slice->setSampleRateHz(384000);
        QVERIFY(model.requestStreamCentre(id, 14'214'200));
        SpectrumWidget pan;
        pan.setSampleRate(384000);
        pan.setDdcCenterFrequency(14'214'200);
        pan.setVfoFrequency(slice->frequency());
        pan.setFrequencyRange(14'220'000, 120000);
        SpectrumOverlayPanel panel(&pan);
        auto controller = std::make_unique<BandCenterController>(&model, &pan, &panel, [&]() { return slice; });
        auto* button = pan.findChild<QPushButton*>(QStringLiteral("bandCenterButton"));
        QVERIFY(button);
        int bandX = -1, minusX = -1;
        for (auto* sibling : button->parentWidget()->findChildren<QPushButton*>(QString(), Qt::FindDirectChildrenOnly)) {
            if (sibling->text() == QStringLiteral("B")) { bandX = sibling->x(); }
            if (sibling->text() == QStringLiteral("-")) { minusX = sibling->x(); }
        }
        QVERIFY(bandX >= 0 && button->x() > bandX && button->x() < minusX);
        QVERIFY(button->isEnabled());
        const bool pin = model.streamCtunPinned(slice->streamIndex());
        const bool paneCtun = pan.ctunEnabled();
        const int low = slice->filterLow(), high = slice->filterHigh();
        QSignalSpy tuning(slice, &SliceModel::frequencyChanged);
        QSignalSpy displayRetunes(&pan, &SpectrumWidget::centerChanged);
        QSignalSpy txChanges(&model, &RadioModel::transmittingChanged);
        button->click();
        QVERIFY(button->isChecked());
        QVERIFY(pan.bandCenterActive());
        QCOMPARE(pan.centerFrequency(), 14'175'000.0);
        QCOMPARE(model.streamAllocator().streamCentreHz(slice->streamIndex()), 14'175'000.0);
        //DH1KLM: Reconstruct the controller to exercise persisted before/view state, not just an in-memory toggle.
        controller.reset();
        controller = std::make_unique<BandCenterController>(&model, &pan, &panel, [&]() { return slice; });
        QVERIFY(button->isChecked());
        button->click();
        QVERIFY(!button->isChecked());
        QVERIFY(!pan.bandCenterActive());
        QCOMPARE(pan.centerFrequency(), 14'220'000.0);
        QCOMPARE(pan.bandwidth(), 120'000.0);
        QCOMPARE(slice->sampleRateHz(), 384000);
        QCOMPARE(model.streamAllocator().streamCentreHz(slice->streamIndex()), 14'214'200.0);
        QCOMPARE(model.streamCtunPinned(slice->streamIndex()), pin);
        QCOMPARE(pan.ctunEnabled(), paneCtun);
        QCOMPARE(slice->filterLow(), low);
        QCOMPARE(slice->filterHigh(), high);
        QCOMPARE(tuning.count(), 0);
        QCOMPARE(displayRetunes.count(), 0);
        QCOMPARE(txChanges.count(), 0);
        QVERIFY(!model.isTransmitting());

        button->click();
        slice->setFrequency(7'150'000);
        pan.setVfoFrequency(7'150'000);
        pan.setFrequencyRange(7'160'000, 80000);
        QTRY_VERIFY(!button->isChecked());
        button->click();
        QVERIFY(button->isChecked());
        slice->setFrequency(14'214'200);
        pan.setVfoFrequency(14'214'200);
        QTRY_VERIFY(button->isChecked());
        QTRY_COMPARE(pan.centerFrequency(), 14'175'000.0);
        button->click();
        QTRY_VERIFY(!button->isChecked());
        QCOMPARE(pan.centerFrequency(), 14'220'000.0);
        QCOMPARE(pan.bandwidth(), 120'000.0);
    }

    void centerReturnsFromOutsideBand_data() {
        QTest::addColumn<double>("outside");
        QTest::addColumn<bool>("alreadyActive");
        QTest::addColumn<bool>("reload");
        for (bool active : {false, true}) {
            for (bool reload : {false, true}) {
                QTest::newRow(qPrintable(QStringLiteral("below-active%1-reload%2").arg(active).arg(reload))) << 13'990'000.0 << active << reload;
                QTest::newRow(qPrintable(QStringLiteral("above-active%1-reload%2").arg(active).arg(reload))) << 14'360'000.0 << active << reload;
            }
        }
    }

    void centerReturnsFromOutsideBand() {
        QFETCH(double, outside);
        QFETCH(bool, alreadyActive);
        QFETCH(bool, reload);
        RadioModel model;
        model.configureStreamPool(1, 1, 384000);
        model.setConnectionStateForTest(ConnectionState::Connected);
        const int id = model.addSlice();
        auto* slice = model.sliceById(id);
        QVERIFY(slice);
        constexpr double lastFrequency = 14'214'200;
        slice->setFrequency(lastFrequency);
        SpectrumWidget pan;
        pan.setSampleRate(384000);
        pan.setDdcCenterFrequency(lastFrequency);
        pan.setVfoFrequency(lastFrequency);
        pan.setFrequencyRange(lastFrequency, 120000);
        SpectrumOverlayPanel panel(&pan);
        auto controller = std::make_unique<BandCenterController>(&model, &pan, &panel, [&]() { return slice; });
        auto* button = pan.findChild<QPushButton*>(QStringLiteral("bandCenterButton"));
        QVERIFY(button);
        if (alreadyActive) { button->click(); QVERIFY(button->isChecked()); }
        slice->setFrequency(outside);
        pan.setVfoFrequency(outside);
        QCoreApplication::processEvents();
        QCOMPARE(slice->frequency(), outside);
        QVERIFY(button->isEnabled());
        if (reload) {
            controller.reset();
            controller = std::make_unique<BandCenterController>(&model, &pan, &panel, [&]() { return slice; });
        }
        QSignalSpy tuning(slice, &SliceModel::frequencyChanged);
        QSignalSpy displayRetunes(&pan, &SpectrumWidget::centerChanged);
        QSignalSpy txChanges(&model, &RadioModel::transmittingChanged);
        slice->setLocked(true);
        button->click();
        QCOMPARE(slice->frequency(), outside);
        QCOMPARE(tuning.count(), 0);
        slice->setLocked(false);
        button->click();
        QCOMPARE(slice->frequency(), lastFrequency);
        QCOMPARE(pan.regionalBandBounds(), (std::pair<double, double>{14'000'000, 14'350'000}));
        QCOMPARE(pan.centerFrequency(), 14'175'000.0);
        QVERIFY(button->isChecked());
        QCOMPARE(tuning.count(), 1);
        QCOMPARE(displayRetunes.count(), 0);
        QCOMPARE(txChanges.count(), 0);
        button->click();
        QVERIFY(!button->isChecked());
        QCOMPARE(slice->frequency(), lastFrequency);
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
        toggle->setChecked(false);
        pan.setBandCenterActive(true);
        QTRY_VERIFY_WITH_TIMEOUT(!(switched = pan.grabFramebuffer()).isNull() && redAt(switched, 0.55), 10000);
        pan.setBandCenterActive(false);
        QTRY_VERIFY_WITH_TIMEOUT(!(switched = pan.grabFramebuffer()).isNull() && !redAt(switched, 0.55), 10000);
        pan.close();
#endif
    }
};
QTEST_MAIN(TestDisplayBandEdges)
#include "tst_display_band_edges.moc"
