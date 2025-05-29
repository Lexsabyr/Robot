#ifndef MOTOR_H
#define MOTOR_H

class Motor {
public:
    virtual ~Motor() = default;
    virtual void move(double angle, double distance) = 0;
    virtual void stop() = 0;
};

#endif