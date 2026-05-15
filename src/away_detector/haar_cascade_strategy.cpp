#include "away_detector/haar_cascade_strategy.hpp"
#include "data/haarcascade_embedded.hpp"
#include <opencv2/core/persistence.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/videoio.hpp>
#include <spdlog/spdlog.h>
#include <string>
#ifdef CAKE_DEBUG_VIZ
#include <filesystem>
#include <fmt/format.h>
#include <opencv2/imgcodecs.hpp>
#endif

namespace cake::away_detector::strategies {

HaarCascadeStrategy::HaarCascadeStrategy(int camera_index, int miss_threshold)
    : m_camera_index{camera_index}, m_miss_threshold{miss_threshold} {
  std::string xml_data(
      reinterpret_cast<const char *>(cake::data::kHaarCascadeData),
      cake::data::kHaarCascadeDataSize);
  cv::FileStorage fs(xml_data, cv::FileStorage::READ | cv::FileStorage::MEMORY);
  if (!fs.isOpened() || !m_cascade.read(fs.getFirstTopLevelNode())) {
    spdlog::error("Failed to load embedded cascade classifier");
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
  m_cascade.detectMultiScale(gray, faces);

#ifdef CAKE_DEBUG_VIZ
  for (const auto &face : faces) {
    cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
  }
  constexpr auto debug_dir = "/tmp/cakeprotocol";
  std::filesystem::create_directories(debug_dir);
  auto path = fmt::format("{}/frame_{:04d}.png", debug_dir, m_frame_counter++);
  cv::imwrite(path, frame);
  spdlog::debug("Debug frame saved to {}", path);
#endif

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
