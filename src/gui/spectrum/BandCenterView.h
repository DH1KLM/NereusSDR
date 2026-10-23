//DH1KLM: Adapted from AetherSDR src/core/backends/anan/AnanModel.h:74-116 [@ab8c82b1].
//DH1KLM: Copyright (C) 2024-2026 Jeremy (KK7GWY) / AetherSDR contributors; GPL-3.0.
//DH1KLM: https://github.com/ten9876/AetherSDR; upstream has no file header, project LICENSE applies.
//DH1KLM: Modification history (NereusSDR): 2026-10-23 DH1KLM, AI-assisted via OpenAI Codex.
//DH1KLM: Retain the selected DDC rate; Nereus does not crop the ordinary pan by 4%.
#pragma once
#include <algorithm>
#include <cmath>
#include <optional>

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
}
