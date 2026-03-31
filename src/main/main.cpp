#include "application.hpp"
#include <argparse/argparse.hpp>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <spdlog/spdlog.h>

auto main(int argc, char **argv) -> int {
  argparse::ArgumentParser args{std::filesystem::path{argv[0]}.stem().string()};

  try {
    args.parse_args(argc, argv);
  } catch (const std::exception &err) {
    spdlog::error("Error: {}", err.what());
    return EXIT_FAILURE;
  }

  Application app;
  app.run();

  return EXIT_SUCCESS;
}
