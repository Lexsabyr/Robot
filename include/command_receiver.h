#ifndef COMMAND_RECEIVER_H
#define COMMAND_RECEIVER_H

#include <utility> // Добавлено для std::pair

class CommandReceiver {
public:
    virtual ~CommandReceiver() = default;
    virtual std::pair<double, double> getCommand() = 0;
    virtual void notifyCommandReceived(double angle, double distance) = 0;
};

#endif