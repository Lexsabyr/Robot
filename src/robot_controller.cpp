#include "robot_controller.h"
#include <iostream>

RobotController::RobotController(std::unique_ptr<Motor> motor,
                               std::unique_ptr<CommandReceiver> receiver)
    : motor(std::move(motor)), receiver(std::move(receiver)) {}

void RobotController::run() {
    try {
        auto command = receiver->getCommand();
        double angle = command.first;
        double distance = command.second;
        
        if (distance <= 0) {
            receiver->notifyCommandReceived(angle, distance);
            motor->stop();
            return;
        }
        
        receiver->notifyCommandReceived(angle, distance);
        motor->move(angle, distance);
    } catch(const std::exception& e) {
        std::cerr << "Control error: " << e.what() << std::endl;
    }
}