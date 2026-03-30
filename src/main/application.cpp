#include "application.hpp"
#include "screen_locker/screen_locker.hpp"
#include <spdlog/spdlog.h>

using cake::screen_locker::ScreenLocker;

auto Application::run() -> void {
  spdlog::info("Starting application on {}", ScreenLocker::platform_name());
  auto result = ScreenLocker::lock();
  if (!result) {
    spdlog::error("Failed to lock screen: {}", result.error());
  } else {
    spdlog::info("Screen locked successfully.");
  }
}
