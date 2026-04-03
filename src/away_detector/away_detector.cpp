#include "away_detector/away_detector.hpp"
#include <algorithm>

namespace cake::away_detector {

AwayDetector::AwayDetector(std::vector<StrategyPtr> strategies)
    : m_strategies{std::move(strategies)} {}

auto AwayDetector::is_away() const noexcept -> bool {
  return std::ranges::all_of(m_strategies,
                             [](const auto &s) { return s->is_away(); });
}

} // namespace cake::away_detector
