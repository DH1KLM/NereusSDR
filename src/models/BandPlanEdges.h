//DH1KLM: SPDX-License-Identifier: GPL-3.0-or-later
//DH1KLM: no-port-check: AetherSDR display port registered in aethersdr-reconciliation.md; no Thetis code.
//DH1KLM: Regional band envelopes follow AetherSDR BandPlanManager.cpp:21-34 [@e2e8c50c].
//DH1KLM: Identity windows from AetherSDR src/models/BandDefs.h [@e2e8c50c].
//DH1KLM: AetherSDR (https://github.com/ten9876/AetherSDR), Jeremy Fielder KK7GWY (ten9876),
//DH1KLM: is licensed GPL-3.0-or-later and has no per-file header in these sources.
//DH1KLM: 2026-10-22 DH1KLM, AI-assisted via OpenAI Codex: use NereusSDR plan segments and Hz.
#pragma once

#include "BandPlan.h"
#include <QVector>
#include <algorithm>
#include <cmath>
#include <limits>

namespace NereusSDR {

inline QVector<double> regionalBandEdgesHz(const QVector<BandSegment>& segments,
                                          double minHz, double maxHz)
{
    if (!std::isfinite(minHz) || !std::isfinite(maxHz) || minHz > maxHz) { return {}; }
    //DH1KLM: These identify bands only; actual endpoints come exclusively from the selected plan.
    constexpr double windows[][2] = {
        {0.1357, 0.1378}, {0.472, 0.479}, {1.8, 2.0}, {3.5, 4.0}, {5.3305, 5.4064},
        {7.0, 7.3}, {10.1, 10.15}, {14.0, 14.35}, {18.068, 18.168},
        {21.0, 21.45}, {24.89, 24.99}, {28.0, 29.7}, {50.0, 54.0},
        {70.0, 70.5}, {144.0, 148.0}, {222.0, 225.0}, {420.0, 450.0},
        {902.0, 928.0}, {1240.0, 1300.0}, {2300.0, 2450.0}, {3300.0, 3500.0},
        {5650.0, 5925.0}, {10000.0, 10500.0}
    };
    QVector<double> edges;
    for (const auto& window : windows) {
        double low = std::numeric_limits<double>::infinity();
        double high = -std::numeric_limits<double>::infinity();
        for (const BandSegment& segment : segments) {
            const double midpoint = (segment.lowMhz + segment.highMhz) / 2.0;
            if (midpoint < window[0] || midpoint > window[1]) { continue; }
            low = std::min(low, segment.lowMhz);
            high = std::max(high, segment.highMhz);
        }
        for (const double edge : {low * 1.0e6, high * 1.0e6}) {
            if (edge >= minHz && edge <= maxHz) { edges.append(edge); }
        }
    }
    std::sort(edges.begin(), edges.end());
    edges.erase(std::unique(edges.begin(), edges.end()), edges.end());
    return edges;
}

} //DH1KLM: namespace NereusSDR
