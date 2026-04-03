#pragma once

#include <opencv2/objdetect.hpp>
#include <string>

namespace cake::away_detector {

struct Config {
  std::string cascade_path;
  int camera_index = 0;
  int miss_threshold = 3;
};

class AwayDetector {
public:
  explicit AwayDetector(Config config);
  ~AwayDetector() = default;
  AwayDetector(const AwayDetector &) = delete;
  AwayDetector &operator=(const AwayDetector &) = delete;
  AwayDetector(AwayDetector &&) = delete;
  AwayDetector &operator=(AwayDetector &&) = delete;

  [[nodiscard]] auto is_away() const noexcept -> bool;

private:
  Config m_config;
  // is_away() is logically const but needs to modify these members
  mutable cv::CascadeClassifier m_cascade;
  mutable int m_consecutive_misses = 0;
};

} // namespace cake::away_detector