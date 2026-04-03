/**
 * TLDR: cake::screen_locker::ScreenLocker::lock() will lock the screen.
 */

#pragma once

#include <expected>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>

#if defined(__linux__)
#define IS_PLATFORM_LINUX
#elif defined(__APPLE__) && defined(__MACH__)
#define IS_PLATFORM_MACOS
#elif defined(_WIN32) || defined(_WIN64)
#define IS_PLATFORM_WINDOWS
#error "ScreenLocker: unsupported platform"
#endif

#if defined(IS_PLATFORM_LINUX) || defined(IS_PLATFORM_MACOS)
#include <array>
#include <cstdlib>
#endif

#if defined(IS_PLATFORM_WINDOWS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace cake::screen_locker {

using LockResult = std::expected<void, std::string>;

/**
 * Stateless utility class for locking the screen on various platforms. The
 * lock() method attempts to lock the screen using platform-specific commands,
 * and returns an error message if it fails. The platform_name() method provides
 * a human-readable name of the current platform.
 */
class ScreenLocker {
public:
  ScreenLocker() = default;
  ~ScreenLocker() = default;
  ScreenLocker(const ScreenLocker &) = delete;
  auto operator=(const ScreenLocker &) -> ScreenLocker & = delete;
  ScreenLocker(ScreenLocker &&) = delete;
  auto operator=(ScreenLocker &&) -> ScreenLocker & = delete;
  [[nodiscard]] static auto lock() noexcept -> LockResult;
  [[nodiscard]] static consteval auto platform_name() noexcept
      -> std::string_view;

private:
#ifdef IS_PLATFORM_LINUX
  // Tries different screen-locking commands for linux hoping to find one that
  // works.
  [[nodiscard]] static auto try_command(std::string_view cmd) noexcept -> bool;
#endif
};

consteval auto ScreenLocker::platform_name() noexcept -> std::string_view {
#ifdef IS_PLATFORM_LINUX
  return "Linux";
#elif defined(IS_PLATFORM_MACOS)
  return "macOS";
#else
  return "Windows";
#endif
}

inline auto ScreenLocker::lock() noexcept -> LockResult {

#if defined(IS_PLATFORM_LINUX)
  // On Linux we try a list of common screen-locking commands hoping one works.
  constexpr std::array candidates{
      "loginctl lock-session",
      "xdg-screensaver lock",
      "gnome-screensaver-command --lock",
      "dm-tool lock",
  };
  for (std::string_view cmd : candidates) {
    if (try_command(cmd)) {
      spdlog::info("Screen locked using command: {}", cmd);
      return {};
    }
  }
  return std::unexpected(
      "All screen-lock commands failed. "
      "Ensure one of: loginctl, xdg-screensaver, "
      "gnome-screensaver-command, or dm-tool is installed and accessible.");
#elif defined(IS_PLATFORM_WINDOWS)
  // LockWorkStation() is the official Win32 API call.
  if (::LockWorkStation() == 0) {
    const DWORD err{::GetLastError()};
    return std::unexpected("LockWorkStation() failed (Win32 error " +
                           std::to_string(err) + ")");
  }
  return {};
#elif defined(IS_PLATFORM_MACOS)
  // CGSession -suspend locks the screen and returns to the login window.
  constexpr std::string_view cmd {
    R"(/System/Library/CoreServices/Menu\ Extras/User.menu/)"
    R"(Contents/Resources/CGSession -suspend)";
  }
  if (std::system(cmd.data()) != 0) {
    return std::unexpected("CGSession -suspend failed. "
                           "Ensure the binary exists at the expected path.");
  }
  return {};
#endif
}
#if defined(IS_PLATFORM_LINUX)
inline auto ScreenLocker::try_command(std::string_view cmd) noexcept -> bool {
  // Redirect stderr to /dev/null so failed attempts are silent.
  const auto full{std::string(cmd) + " 2>/dev/null"};
  return std::system(full.c_str()) == 0;
}
#endif

} // namespace cake::screen_locker