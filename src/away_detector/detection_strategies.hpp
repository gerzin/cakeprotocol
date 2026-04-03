#pragma once

namespace cake::away_detector::strategies {

class AwayDetectorStrategy {
public:
  virtual ~AwayDetectorStrategy() = default;
  [[nodiscard]] virtual bool is_away() const noexcept = 0;
};
} // namespace cake::away_detector::strategies