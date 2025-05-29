#ifndef ROBOT_CONTROLLER_H
#define ROBOT_CONTROLLER_H

#include "motor.h"
#include "command_receiver.h"
#include <memory>

class RobotController {
public:
    RobotController(std::unique_ptr<Motor> motor, std::unique_ptr<CommandReceiver> receiver);
    void run();

private:
    std::unique_ptr<Motor> motor;
    std::unique_ptr<CommandReceiver> receiver;
};

#endif