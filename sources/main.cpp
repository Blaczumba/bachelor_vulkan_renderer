#include <application/double_screenshot_application.h>

int main() {
    DoubleScreenshotApplication& app = DoubleScreenshotApplication::getInstance();
    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}