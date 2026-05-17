#pragma once

#include "away_detector/detection_strategies.hpp"
#include "utils/json_encoder.hpp"
#include <format>
#include <memory>
#include <vector>

namespace cake::away_detector {

struct Config : public cake::utils::JsonEncoder {
  friend class cake::utils::JsonEncoder;
  int camera_index = 0;
  int miss_threshold = 3;

private:
  [[nodiscard]] auto to_json_impl() const -> std::string {
    return std::format(R"({{"camera_index": {}, "miss_threshold": {}}})",
                       camera_index, miss_threshold);
  }
};

class AwayDetector {
public:
  using StrategyPtr = std::unique_ptr<strategies::AwayDetectorStrategy>;

  explicit AwayDetector(std::vector<StrategyPtr> strategies);
  ~AwayDetector() = default;
  AwayDetector(const AwayDetector &) = delete;
  AwayDetector &operator=(const AwayDetector &) = delete;
  AwayDetector(AwayDetector &&) = default;
  AwayDetector &operator=(AwayDetector &&) = default;

  [[nodiscard]] auto is_away() const noexcept -> bool;

private:
  std::vector<StrategyPtr> m_strategies;
};

} // namespace cake::away_detector