#pragma once

#include <expected>
#include <string>
#include <string_view>

#if defined(__linux__)
#define SL_PLATFORM_LINUX
#elif defined(__APPLE__) && defined(__MACH__)
#define SL_PLATFORM_MACOS
#elif defined(_WIN32) || defined(_WIN64)
#define SL_PLATFORM_WINDOWS
#error "ScreenLocker: unsupported platform"
#endif

#if defined(SL_PLATFORM_LINUX) || defined(SL_PLATFORM_MACOS)
#include <array>
#include <cstdlib>
#endif

#ifdef SL_PLATFORM_WINDOWS
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
  ScreenLocker &operator=(const ScreenLocker &) = delete;
  ScreenLocker(ScreenLocker &&) = delete;
  ScreenLocker &operator=(ScreenLocker &&) = delete;
  [[nodiscard]] static LockResult lock() noexcept;
  [[nodiscard]] static consteval std::string_view platform_name() noexcept;

private:
#ifdef SL_PLATFORM_LINUX
  // Tries different screen-locking commands for linux hoping to find one that
  // works.
  [[nodiscard]] static bool try_command(std::string_view cmd) noexcept;
#endif
};

consteval std::string_view ScreenLocker::platform_name() noexcept {
#ifdef SL_PLATFORM_LINUX
  return "Linux";
#elif defined(SL_PLATFORM_MACOS)
  return "macOS";
#else
  return "Windows";
#endif
}

inline LockResult ScreenLocker::lock() noexcept {
#ifdef SL_PLATFORM_WINDOWS
  // LockWorkStation() is the official Win32 API call.
  if (::LockWorkStation() == 0) {
    const DWORD err{::GetLastError()};
    return std::unexpected("LockWorkStation() failed (Win32 error " +
                           std::to_string(err) + ")");
  }
  return {};
#elif defined(SL_PLATFORM_MACOS)
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
#elif defined(SL_PLATFORM_LINUX)
  // On Linux we try a list of common screen-locking commands in order of
  // preference:
  //  loginctl  – systemd (most modern distros, Wayland + X11)
  //  xdg-screensaver – XDG standard (X11)
  //  gnome-screensaver-command – GNOME fallback
  //  dm-tool (LightDM)
  constexpr std::array candidates{
      "loginctl lock-session",
      "xdg-screensaver lock",
      "gnome-screensaver-command --lock",
      "dm-tool lock",
  };
  for (std::string_view cmd : candidates) {
    if (try_command(cmd)) {
      return {};
    }
  }
  return std::unexpected(
      "All screen-lock commands failed. "
      "Ensure one of: loginctl, xdg-screensaver, "
      "gnome-screensaver-command, or dm-tool is installed and accessible.");
#endif
}
#ifdef SL_PLATFORM_LINUX
inline bool ScreenLocker::try_command(std::string_view cmd) noexcept {
  // Redirect stderr to /dev/null so failed attempts are silent.
  const auto full{std::string(cmd) + " 2>/dev/null"};
  return std::system(full.c_str()) == 0;
}
#endif

} // namespace cake::screen_locker