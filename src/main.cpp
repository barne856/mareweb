#include "mareweb_application.hpp"

int main() {
  constexpr uint32_t WINDOW_WIDTH = 512;
  constexpr uint32_t WINDOW_HEIGHT = 512;
  try {
    Application::getInstance().run(WINDOW_WIDTH, WINDOW_HEIGHT);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}