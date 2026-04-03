#pragma once
#include <concepts>
#include <cstddef>
#include <utility>

namespace cake::away_detector::strategies {

class AwayDetectorStrategy {
public:
  virtual ~AwayDetectorStrategy() = default;
  [[nodiscard]] virtual bool is_away() const noexcept = 0;
};

/**
 * @brief Strategy wrapper that requires N consecutive hits before considering
 * the user away.
 */
template <typename Strategy, std::size_t N>
  requires std::derived_from<Strategy, AwayDetectorStrategy>
class MultiTryStrategy : public AwayDetectorStrategy {
public:
  template <typename... Args>
  explicit MultiTryStrategy(Args &&...args)
      : m_strategy(std::forward<Args>(args)...) {}

  [[nodiscard]] auto is_away() const noexcept -> bool override {
    if (m_strategy.is_away()) {
      ++m_consecutive_hits;
    } else {
      m_consecutive_hits = 0;
    }
    return m_consecutive_hits >= N;
  }

private:
  Strategy m_strategy;
  mutable std::size_t m_consecutive_hits{0};
};

} // namespace cake::away_detector::strategies