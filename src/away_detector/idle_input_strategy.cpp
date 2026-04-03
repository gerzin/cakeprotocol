#include "away_detector/idle_input_strategy.hpp"
#include <spdlog/spdlog.h>

#if defined(IS_PLATFORM_LINUX)
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <string_view>
#elif defined(IS_PLATFORM_MACOS)
#include <CoreGraphics/CGEventSource.h>
#endif

namespace cake::away_detector::strategies {

IdleInputStrategy::IdleInputStrategy(std::chrono::seconds idle_threshold)
    : m_idle_threshold{idle_threshold} {}

auto IdleInputStrategy::is_away() const noexcept -> bool {
  const auto idle{get_idle_time()};
  if (idle < std::chrono::milliseconds::zero()) {
    spdlog::warn("Could not determine idle time, assuming user is present");
    return false;
  }

  spdlog::debug("User idle for {}ms (threshold: {}s)", idle.count(),
                m_idle_threshold.count());
  return idle >= m_idle_threshold;
}

#if defined(IS_PLATFORM_LINUX)
// Wrapper to avoid -Wignored-attributes with decltype(&pclose).
struct PipeCloser {
  auto operator()(FILE *f) const noexcept -> void {
    if (f) {
      pclose(f);
    }
  }
};

// Try a command that prints idle time in milliseconds to stdout.
static auto try_idle_command(std::string_view cmd) noexcept
    -> std::chrono::milliseconds {
  const auto full{std::string(cmd) + " 2>/dev/null"};
  // NOLINTNEXTLINE(cert-env33-c) - required for querying system idle time
  const auto pipe{std::unique_ptr<FILE, PipeCloser>(popen(full.c_str(), "r"))};
  if (!pipe) {
    return std::chrono::milliseconds{-1};
  }
  std::array<char, 64> buf{};
  if (fgets(buf.data(), buf.size(), pipe.get()) == nullptr) {
    return std::chrono::milliseconds{-1};
  }
  const auto ms{std::strtol(buf.data(), nullptr, 10)};
  return std::chrono::milliseconds{ms};
}
#endif

auto IdleInputStrategy::get_idle_time() noexcept -> std::chrono::milliseconds {

#if defined(IS_PLATFORM_LINUX)
  // Try multiple idle-time tools to support both X11 and Wayland.
  // Each prints idle milliseconds to stdout.
  constexpr std::array candidates{
      "xprintidle", // X11
      "idletime",   // Wayland (via ext-idle-notify-v1)
      "dbus-send --print-reply --dest=org.gnome.Mutter.IdleMonitor "
      "/org/gnome/Mutter/IdleMonitor/Core "
      "org.gnome.Mutter.IdleMonitor.GetIdletime "
      "| awk '/int64/ {print $2}'", // GNOME/Mutter (Wayland & X11)
  };
  for (const auto &cmd : candidates) {
    const auto ms{try_idle_command(cmd)};
    if (ms >= std::chrono::milliseconds::zero()) {
      return ms;
    }
  }
  spdlog::warn("All idle-time commands failed. "
               "Ensure one of: xprintidle, idletime is installed, "
               "or GNOME Mutter D-Bus interface is available.");
  return std::chrono::milliseconds{-1};

#elif defined(IS_PLATFORM_MACOS)
  // CGEventSourceSecondsSinceLastEventType returns seconds as double.
  const double idle_secs = CGEventSourceSecondsSinceLastEventType(
      kCGEventSourceStateCombinedSessionState, kCGAnyInputEventType);
  return std::chrono::milliseconds{static_cast<long>(idle_secs * 1000.0)};

#elif defined(IS_PLATFORM_WINDOWS)
  LASTINPUTINFO lii{};
  lii.cbSize = sizeof(lii);
  if (::GetLastInputInfo(&lii) == 0) {
    spdlog::warn("GetLastInputInfo failed");
    return std::chrono::milliseconds{-1};
  }
  const DWORD idle_ms = ::GetTickCount() - lii.dwTime;
  return std::chrono::milliseconds{idle_ms};

#else
  spdlog::warn("IdleInputStrategy: unsupported platform");
  return std::chrono::milliseconds{-1};
#endif
}

} // namespace cake::away_detector::strategies
