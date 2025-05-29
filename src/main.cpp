#include "robot_controller.h"
#include "simulated_motor.h"
#include "image_processor_command_receiver.h"
#include <memory>
#include <iostream>
#include <csignal>

volatile bool running = true;

void signalHandler(int signum) {
    running = false;
}

int main() {
    signal(SIGINT, signalHandler);
    
    try {
        const int camera_index = 0; // Индекс камеры
        
        std::cout << "Starting Robot Control System..." << std::endl;
        std::cout << "Initializing camera with index " << camera_index << std::endl;

        auto motor = std::make_unique<SimulatedMotor>();
        auto receiver = std::make_unique<ImageProcessorCommandReceiver>(camera_index);
        
        RobotController controller(std::move(motor), std::move(receiver));
        
        while (running) {
            controller.run();
            if (cv::waitKey(30) >= 0) break;
        }
        
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Shutting down..." << std::endl;
    return 0;
}