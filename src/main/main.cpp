#include "application.hpp"
#include <argparse/argparse.hpp>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <spdlog/spdlog.h>

namespace {
void signal_handler(int /*signum*/) { Application::request_stop(); }
} // namespace

auto main(int argc, char **argv) -> int {
  argparse::ArgumentParser args{std::filesystem::path{argv[0]}.stem().string()};

  args.add_argument("--cascade-path")
      .default_value(std::string{"data/haarcascades/"
                                 "haarcascade_frontalface_default.xml"})
      .help("path to Haar cascade XML file");

  args.add_argument("--camera-index")
      .default_value(0)
      .scan<'i', int>()
      .help("camera device index");

  args.add_argument("--miss-threshold")
      .default_value(2)
      .scan<'i', int>()
      .help("consecutive no-face frames before locking");

  args.add_argument("--poll-interval")
      .default_value(10)
      .scan<'i', int>()
      .help("seconds between checks");

  try {
    args.parse_args(argc, argv);
  } catch (const std::exception &err) {
    spdlog::error("Error: {}", err.what());
    return EXIT_FAILURE;
  }

  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  Application::Config config{
      .detector =
          {
              .cascade_path = args.get<std::string>("--cascade-path"),
              .camera_index = args.get<int>("--camera-index"),
              .miss_threshold = args.get<int>("--miss-threshold"),
          },
      .poll_interval = std::chrono::seconds{args.get<int>("--poll-interval")},
  };

  Application app{std::move(config)};
  app.run();

  return EXIT_SUCCESS;
}
