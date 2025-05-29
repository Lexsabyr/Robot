#include "simulated_motor.h"
#include <chrono>
#include <thread>

void SimulatedMotor::move(double angle, double distance) {
    std::cout << "[SimulatedMotor] Moving at angle " << angle 
              << " degrees for distance " << distance << " pixels" << std::endl;
    
    // Simulate movement time based on distance
    int simulated_time = static_cast<int>(distance * 10);
    std::this_thread::sleep_for(std::chrono::milliseconds(simulated_time));
}

void SimulatedMotor::stop() {
    std::cout << "[SimulatedMotor] Stopping" << std::endl;
}