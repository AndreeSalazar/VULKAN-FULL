#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "vulkan_cube.h"

#include <iostream>
#include <stdexcept>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class App {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;
    VulkanCube cube;

    void initWindow() {
        glfwInit();
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Cube", nullptr, nullptr);
        if (!window) {
            throw std::runtime_error("failed to create GLFW window!");
        }
        
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
        // El recreateSwapChain se manejará automáticamente en drawFrame
    }

    void initVulkan() {
        cube.initVulkan(window);
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            cube.drawFrame();
        }
        
        cube.waitDeviceIdle();
    }

    void cleanup() {
        cube.cleanup();
        
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

int main() {
    App app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "ERROR: Excepción desconocida ocurrida" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

