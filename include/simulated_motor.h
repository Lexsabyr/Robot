#ifndef SIMULATED_MOTOR_H
#define SIMULATED_MOTOR_H

#include "motor.h"
#include <iostream>
#include <chrono>
#include <thread>

class SimulatedMotor : public Motor {
public:
    void move(double angle, double distance) override;
    void stop() override;
};

#endif