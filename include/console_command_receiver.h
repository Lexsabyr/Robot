#ifndef CONSOLE_COMMAND_RECEIVER_H
#define CONSOLE_COMMAND_RECEIVER_H

#include "command_receiver.h"
#include <iostream>

class ConsoleCommandReceiver : public CommandReceiver {
public:
    std::pair<double, double> getCommand() override;
    void notifyCommandReceived(double angle, double distance) override;
};

#endif