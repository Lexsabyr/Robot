#ifndef IMAGE_PROCESSOR_COMMAND_RECEIVER_H
#define IMAGE_PROCESSOR_COMMAND_RECEIVER_H

#include "command_receiver.h"
#include <opencv2/opencv.hpp>
#include <string>

class ImageProcessorCommandReceiver : public CommandReceiver {
public:
    ImageProcessorCommandReceiver(int cameraIndex = 0);
    ~ImageProcessorCommandReceiver();

    std::pair<double, double> getCommand() override;
    void notifyCommandReceived(double angle, double distance) override;

private:
    cv::Point findColorCenter(const cv::Mat& image, const cv::Scalar& lower, const cv::Scalar& upper);
    cv::Point findRedCenter(const cv::Mat& image); // Новая функция для красного
    double calculateDistance(const cv::Point& p1, const cv::Point& p2);
    
    cv::VideoCapture cap;
    const double STOP_DISTANCE = 20.0;
    int cameraIndex;
};

#endif