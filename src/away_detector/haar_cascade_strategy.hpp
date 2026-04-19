#pragma once
#include "away_detector/detection_strategies.hpp"
#include <opencv2/objdetect.hpp>

namespace cake::away_detector::strategies {

/**
 * @brief Strategy that uses a Haar Cascade classifier to detect if the user is
 * away.
 *
 */
class HaarCascadeStrategy : public AwayDetectorStrategy {
public:
  explicit HaarCascadeStrategy(int camera_index, int miss_threshold);
  ~HaarCascadeStrategy() override = default;
  HaarCascadeStrategy(const HaarCascadeStrategy &) = delete;
  HaarCascadeStrategy &operator=(const HaarCascadeStrategy &) = delete;
  HaarCascadeStrategy(HaarCascadeStrategy &&) = delete;
  HaarCascadeStrategy &operator=(HaarCascadeStrategy &&) = delete;

  [[nodiscard]] auto is_away() const noexcept -> bool override;

private:
  int m_camera_index;
  int m_miss_threshold;
  mutable cv::CascadeClassifier m_cascade;
  mutable int m_consecutive_misses{0};
#ifdef CAKE_DEBUG_VIZ
  mutable int m_frame_counter{0};
#endif
};
} // namespace cake::away_detector::strategies