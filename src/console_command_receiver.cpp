#include "console_command_receiver.h"
#include <iostream>

std::pair<double, double> ConsoleCommandReceiver::getCommand() {
    double angle, distance;
    std::cout << "Enter command (angle distance): ";
    std::cin >> angle >> distance;
    return std::make_pair(angle, distance); // Явное создание pair
}

void ConsoleCommandReceiver::notifyCommandReceived(double angle, double distance) {
    std::cout << "[ConsoleReceiver] Command received: angle " << angle 
              << ", distance " << distance << std::endl;
}