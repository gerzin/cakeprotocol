#pragma once
#include "away_detector/detection_strategies.hpp"
#include <opencv2/objdetect.hpp>
#include <string>
#include <string_view>

namespace cake::away_detector::strategies {

class HaarCascadeStrategy : public AwayDetectorStrategy {
public:
  explicit HaarCascadeStrategy(std::string_view cascade_path, int camera_index,
                               int miss_threshold);
  ~HaarCascadeStrategy() override = default;
  HaarCascadeStrategy(const HaarCascadeStrategy &) = delete;
  HaarCascadeStrategy &operator=(const HaarCascadeStrategy &) = delete;
  HaarCascadeStrategy(HaarCascadeStrategy &&) = delete;
  HaarCascadeStrategy &operator=(HaarCascadeStrategy &&) = delete;

  [[nodiscard]] auto is_away() const noexcept -> bool override;

private:
  std::string m_cascade_path;
  int m_camera_index;
  int m_miss_threshold;
  mutable cv::CascadeClassifier m_cascade;
  mutable int m_consecutive_misses = 0;
};
} // namespace cake::away_detector::strategies