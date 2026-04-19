#include "application.hpp"
#include "screen_locker/screen_locker.hpp"
#include <condition_variable>
#include <memory>
#include <mutex>
#include <spdlog/spdlog.h>
#include <thread>

std::atomic<bool> Application::m_stop_requested{false};

namespace {
std::mutex g_cv_mutex;
std::condition_variable g_cv;
} // namespace

Application::Application(Config config)
    : m_config{std::move(config)}, m_detector{[this] {
        std::vector<cake::away_detector::AwayDetector::StrategyPtr> strategies;
        strategies.push_back(
            std::make_unique<
                cake::away_detector::strategies::HaarCascadeStrategy>(
                m_config.detector.camera_index,
                m_config.detector.miss_threshold));
        strategies.push_back(
            std::make_unique<
                cake::away_detector::strategies::IdleInputStrategy>(
                m_config.poll_interval * m_config.detector.miss_threshold));
        return strategies;
      }()} {}

auto Application::request_stop() -> void {
  m_stop_requested.store(true);
  g_cv.notify_all();
}

auto Application::stop_requested() -> bool { return m_stop_requested.load(); }

auto Application::run() -> void {
  using cake::screen_locker::ScreenLocker;

  spdlog::info("Starting cakeprotocol on {}", ScreenLocker::platform_name());
  spdlog::info("Polling every {}s, lock after {} consecutive misses",
               m_config.poll_interval.count(),
               m_config.detector.miss_threshold);

  while (!m_stop_requested.load()) {
    if (m_detector.is_away()) {
      spdlog::info("User appears away, locking screen");
      auto result = ScreenLocker::lock();
      if (!result) {
        spdlog::error("Failed to lock screen: {}", result.error());
      }
    }
    {
      std::unique_lock lock{g_cv_mutex};
      g_cv.wait_for(lock, m_config.poll_interval,
                    [] { return m_stop_requested.load(); });
    }
  }

  spdlog::info("Shutting down");
}
