#include "server_command_receiver.h"
#include <iostream>
#include <stdexcept>
#include <sstream>

ServerCommandReceiver::ServerCommandReceiver(const std::string& host, int port, 
                                          const std::string& topic)
    : host(host), port(port), topic(topic), mosq(nullptr),
      new_message(false), last_angle(0), last_distance(0), running(true) {
    mosquitto_lib_init();
    mosq = mosquitto_new(nullptr, true, this);
    
    if(!mosq) {
        mosquitto_lib_cleanup();
        throw std::runtime_error("Failed to create Mosquitto instance");
    }
    
    mosquitto_message_callback_set(mosq, ServerCommandReceiver::on_message);
    connect();
    mqtt_thread = std::thread(&ServerCommandReceiver::loop, this);
}

ServerCommandReceiver::~ServerCommandReceiver() noexcept {
    try {
        running = false;
        if (mqtt_thread.joinable()) {
            mqtt_thread.join();
        }
        disconnect();
        if(mosq) {
            mosquitto_destroy(mosq);
        }
        mosquitto_lib_cleanup();
    } catch (...) {}
}

void ServerCommandReceiver::loop() {
    while (running) {
        int rc = mosquitto_loop(mosq, 100, 1);
        if (rc != MOSQ_ERR_SUCCESS) {
            std::cerr << "[MQTT] Loop error: " << mosquitto_strerror(rc) << std::endl;
            break;
        }
    }
}

void ServerCommandReceiver::connect() {
    int rc = mosquitto_connect(mosq, host.c_str(), port, 60);
    if(rc != MOSQ_ERR_SUCCESS) {
        throw std::runtime_error("Failed to connect to MQTT broker: " + 
                               std::string(mosquitto_strerror(rc)));
    }
    
    rc = mosquitto_subscribe(mosq, nullptr, topic.c_str(), 0);
    if(rc != MOSQ_ERR_SUCCESS) {
        mosquitto_disconnect(mosq);
        throw std::runtime_error("Failed to subscribe to topic: " + 
                               std::string(mosquitto_strerror(rc)));
    }
    
    std::cout << "[MQTT] Connected to broker at " << host << ":" << port
              << ", subscribed to topic: " << topic << std::endl;
}

void ServerCommandReceiver::disconnect() noexcept {
    if(mosq) {
        mosquitto_unsubscribe(mosq, nullptr, topic.c_str());
        mosquitto_disconnect(mosq);
    }
}

void ServerCommandReceiver::on_message(struct mosquitto* mosq, void* obj,
                                     const struct mosquitto_message* msg) {
    if(!obj || !msg || !msg->payload) return;
    
    ServerCommandReceiver* receiver = static_cast<ServerCommandReceiver*>(obj);
    std::string message(static_cast<char*>(msg->payload), msg->payloadlen);
    
    double angle = 0;
    double distance = 0;
    
    std::istringstream iss(message);
    if (!(iss >> angle >> distance)) {
        std::cerr << "[WARNING] Invalid command format: " << message << std::endl;
        return;
    }
    
    std::lock_guard<std::mutex> lock(receiver->mtx);
    receiver->last_angle = angle;
    receiver->last_distance = distance;
    receiver->new_message = true;
    receiver->cv.notify_one();
}

std::pair<double, double> ServerCommandReceiver::getCommand() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this]{ return new_message; });
    new_message = false;
    return {last_angle, last_distance};
}

void ServerCommandReceiver::notifyCommandReceived(double angle, double distance) {
    std::cout << "[MQTT] Executing command: angle " << angle 
              << ", distance " << distance << std::endl;
}