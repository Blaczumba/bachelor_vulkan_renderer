// #include <application/offscreen_rendering_application.h>
#include <application/double_screenshot_application.h>

int main() {
    try {
        SingleApp& app = SingleApp::getInstance();
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}