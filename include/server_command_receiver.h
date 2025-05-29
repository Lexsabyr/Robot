#ifndef SERVER_COMMAND_RECEIVER_H
#define SERVER_COMMAND_RECEIVER_H

#include "command_receiver.h"
#include <mosquitto.h>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <string>
#include <thread>

class ServerCommandReceiver : public CommandReceiver {
public:
    ServerCommandReceiver(const std::string& host, int port, const std::string& topic);
    ~ServerCommandReceiver() noexcept;

    ServerCommandReceiver(const ServerCommandReceiver&) = delete;
    ServerCommandReceiver& operator=(const ServerCommandReceiver&) = delete;

    std::pair<double, double> getCommand() override;
    void notifyCommandReceived(double angle, double distance) override;

private:
    static void on_message(struct mosquitto* mosq, void* obj, 
                         const struct mosquitto_message* msg);
    void connect();
    void disconnect() noexcept;
    void loop();

    struct mosquitto* mosq;
    const std::string host;
    const int port;
    const std::string topic;

    std::mutex mtx;
    std::condition_variable cv;
    double last_angle;
    double last_distance;
    bool new_message;

    std::atomic<bool> running;
    std::thread mqtt_thread;
};

#endif