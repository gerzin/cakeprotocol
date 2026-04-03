#include "away_detector/haar_cascade_strategy.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/videoio.hpp>
#include <spdlog/spdlog.h>

namespace cake::away_detector::strategies {

HaarCascadeStrategy::HaarCascadeStrategy(std::string_view cascade_path,
                                         int camera_index, int miss_threshold)
    : m_cascade_path{cascade_path}, m_camera_index{camera_index},
      m_miss_threshold{miss_threshold} {
  if (!m_cascade.load(m_cascade_path)) {
    spdlog::error("Failed to load cascade classifier from '{}'",
                  m_cascade_path);
  }
}

auto HaarCascadeStrategy::is_away() const noexcept -> bool {
  if (m_cascade.empty()) {
    spdlog::warn("Cascade classifier not loaded, assuming user is present");
    return false;
  }

  cv::VideoCapture cap{m_camera_index};
  if (!cap.isOpened()) {
    spdlog::warn("Cannot open camera {}, assuming user is present",
                 m_camera_index);
    return false;
  }

  cv::Mat frame;
  if (!cap.read(frame) || frame.empty()) {
    spdlog::warn("Failed to capture frame, assuming user is present");
    return false;
  }

  cv::Mat gray;
  cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

  std::vector<cv::Rect> faces;
  // TODO: Maybe need to tweak this
  m_cascade.detectMultiScale(gray, faces, 1.1, 3, 0, cv::Size(80, 80));

  if (faces.empty()) {
    ++m_consecutive_misses;
    spdlog::debug("No face detected ({}/{})", m_consecutive_misses,
                  m_miss_threshold);
  } else {
    m_consecutive_misses = 0;
    spdlog::debug("Face detected, resetting miss counter");
  }

  return m_consecutive_misses >= m_miss_threshold;
}

} // namespace cake::away_detector::strategies
