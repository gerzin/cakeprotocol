#pragma once

#include "away_detector/away_detector.hpp"
#include "away_detector/haar_cascade_strategy.hpp"
#include "away_detector/idle_input_strategy.hpp"
#include <atomic>
#include <chrono>

class Application {
public:
  struct Config {
    cake::away_detector::Config detector;
    std::chrono::seconds poll_interval{10};
  };

  explicit Application(Config config);
  auto run() -> void;

  static auto request_stop() -> void;
  [[nodiscard]] static auto stop_requested() -> bool;

private:
  Config m_config;
  cake::away_detector::AwayDetector m_detector;
  static std::atomic<bool> m_stop_requested;
};
