#include "application.hpp"
#include "screen_locker/screen_locker.hpp"
#include <memory>
#include <spdlog/spdlog.h>
#include <thread>

std::atomic<bool> Application::m_stop_requested{false};

Application::Application(Config config)
    : m_config{std::move(config)}, m_detector{[this] {
        std::vector<cake::away_detector::AwayDetector::StrategyPtr> strategies;
        strategies.push_back(
            std::make_unique<
                cake::away_detector::strategies::HaarCascadeStrategy>(
                m_config.detector.cascade_path, m_config.detector.camera_index,
                m_config.detector.miss_threshold));
        return strategies;
      }()} {}

auto Application::request_stop() -> void { m_stop_requested.store(true); }

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
    std::this_thread::sleep_for(m_config.poll_interval);
  }

  spdlog::info("Shutting down");
}
