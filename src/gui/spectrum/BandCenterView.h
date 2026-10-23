//DH1KLM: Adapted from AetherSDR src/core/backends/anan/AnanModel.h:74-116 [@ab8c82b1].
//DH1KLM: Copyright (C) 2024-2026 Jeremy (KK7GWY) / AetherSDR contributors; GPL-3.0.
//DH1KLM: https://github.com/ten9876/AetherSDR; upstream has no file header, project LICENSE applies.
//DH1KLM: Modification history (NereusSDR): 2026-10-23 DH1KLM, AI-assisted via OpenAI Codex.
//DH1KLM: Select a supported per-band DDC rate; Nereus does not crop the ordinary pan by 4%.
#pragma once
#include <algorithm>
#include <cmath>
#include <optional>
#include <span>

namespace NereusSDR {
struct BandCenterView {
    double centreHz;
    double spanHz;
    bool wholeBand;
};

inline std::optional<BandCenterView> planBandCenter(double low, double high, double rx,
                                                   int rate, int filterLow, int filterHigh)
{
    if (!std::isfinite(low) || !std::isfinite(high) || !std::isfinite(rx)
        || low <= 0 || high <= low || rx < low || rx > high || rate < 1000
        || filterLow > filterHigh) { return std::nullopt; }
    const double half = rate * 0.46;
    const double minimum = rx + std::max(0, filterHigh) - half;
    const double maximum = rx + std::min(0, filterLow) + half;
    if (minimum > maximum) { return std::nullopt; }
    const double centre = std::round(std::clamp((low + high) / 2.0, minimum, maximum));
    const double span = std::clamp(2.0 * std::max(centre - low, high - centre) * 1.02,
                                   1000.0, static_cast<double>(rate));
    return BandCenterView{centre, span, low > centre - span / 2.0 && high < centre + span / 2.0};
}

inline int bandCenterSampleRate(double low, double high, double rx, int currentRate,
                                int filterLow, int filterHigh, std::span<const int> allowed)
{
    //DH1KLM: Explicit operator choices take precedence, even when only part of the band fits.
    const int preferred = (low == 1'810'000 || low == 1'800'000) ? 192000
        : (low == 28'000'000 || low == 50'000'000) ? 768000 : 0;
    if (preferred) {
        for (int rate : allowed) {
            if (rate == preferred && planBandCenter(low, high, rx, rate, filterLow, filterHigh)) { return rate; }
        }
    }
    //DH1KLM: Prefer the smallest actual device rate that captures both edges with display margin.
    int fitting = 0, widest = 0;
    for (int rate : allowed) {
        const auto view = planBandCenter(low, high, rx, rate, filterLow, filterHigh);
        if (!view) { continue; }
        widest = std::max(widest, rate);
        const double withMargin = 2.0 * std::max(view->centreHz - low, high - view->centreHz) * 1.02;
        if (view->wholeBand && withMargin <= rate && (!fitting || rate < fitting)) { fitting = rate; }
    }
    return fitting ? fitting : (widest ? widest : currentRate);
}
}
