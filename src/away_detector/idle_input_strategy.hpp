#pragma once
#include "away_detector/detection_strategies.hpp"
#include <chrono>

#if defined(__linux__)
#define IS_PLATFORM_LINUX
#elif defined(__APPLE__) && defined(__MACH__)
#define IS_PLATFORM_MACOS
#elif defined(_WIN32) || defined(_WIN64)
#define IS_PLATFORM_WINDOWS
#endif

#if defined(IS_PLATFORM_WINDOWS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace cake::away_detector::strategies {

/**
 * @brief Strategy that detects if the user is away based on input idle time.
 *
 * Uses platform-specific APIs to query how long since the last keyboard/mouse
 * event. If idle time exceeds the configured threshold the user is considered
 * away.
 */
class IdleInputStrategy : public AwayDetectorStrategy {
public:
  explicit IdleInputStrategy(std::chrono::seconds idle_threshold);
  ~IdleInputStrategy() override = default;
  IdleInputStrategy(const IdleInputStrategy &) = delete;
  IdleInputStrategy &operator=(const IdleInputStrategy &) = delete;
  IdleInputStrategy(IdleInputStrategy &&) = delete;
  IdleInputStrategy &operator=(IdleInputStrategy &&) = delete;

  [[nodiscard]] auto is_away() const noexcept -> bool override;

private:
  std::chrono::seconds m_idle_threshold;

  [[nodiscard]] static auto get_idle_time() noexcept
      -> std::chrono::milliseconds;
};

} // namespace cake::away_detector::strategies
