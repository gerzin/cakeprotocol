#pragma once
#include <string>

namespace cake::utils {
class JsonEncoder {
public:
  auto to_json(this auto &&self) -> std::string { return self.to_json_impl(); }
};
} // namespace cake::utils