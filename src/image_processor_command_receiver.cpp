#include "image_processor_command_receiver.h"
#include <iostream>
#include <cmath>

// HSV-диапазоны для цветов
const cv::Scalar blueLower(100, 150, 50), blueUpper(140, 255, 255);
const cv::Scalar redLower1(0, 120, 70), redUpper1(10, 255, 255);
const cv::Scalar redLower2(160, 120, 70), redUpper2(180, 255, 255);
const cv::Scalar greenLower(40, 50, 50), greenUpper(80, 255, 255);

ImageProcessorCommandReceiver::ImageProcessorCommandReceiver(int cameraIndex)
    : cameraIndex(cameraIndex) {
    cap.open("http://192.168.0.40:8080/video");
    if(!cap.isOpened()) {
        throw std::runtime_error("Failed to open camera with index: " + std::to_string(cameraIndex));
    }
    
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    cap.set(cv::CAP_PROP_FPS, 30);
    
    cv::namedWindow("Color Debug", cv::WINDOW_NORMAL);
}

ImageProcessorCommandReceiver::~ImageProcessorCommandReceiver() {
    if(cap.isOpened()) {
        cap.release();
    }
    cv::destroyAllWindows();
}

std::pair<double, double> ImageProcessorCommandReceiver::getCommand() {
    cv::Mat frame;
    cap >> frame;
    
    if(frame.empty()) {
        throw std::runtime_error("Failed to capture frame from camera");
    }

    cv::Mat blurred;
    cv::GaussianBlur(frame, blurred, cv::Size(11, 11), 2);
    
    cv::Mat hsv;
    cv::cvtColor(blurred, hsv, cv::COLOR_BGR2HSV);
    
    // Поиск цветных меток с разными параметрами для красного
    cv::Point front = findColorCenter(hsv, blueLower, blueUpper);    // Синий
    cv::Point rear = findRedCenter(hsv);                            // Красный (специальная функция)
    cv::Point target = findColorCenter(hsv, greenLower, greenUpper); // Зеленый

    // Отладочное изображение
    cv::Mat debugFrame = frame.clone();
    
    // Рисуем обнаруженные точки
    if(front.x != -1) {
        cv::circle(debugFrame, front, 10, cv::Scalar(255, 0, 0), 2);
        cv::putText(debugFrame, "Front", front + cv::Point(15,5),
                  cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,0,0));
    }
    if(rear.x != -1) {
        cv::circle(debugFrame, rear, 10, cv::Scalar(0, 0, 255), 2);
        cv::putText(debugFrame, "Rear", rear + cv::Point(15,5),
                  cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,255));
    }
    if(target.x != -1) {
        cv::circle(debugFrame, target, 10, cv::Scalar(0, 255, 0), 2);
        cv::putText(debugFrame, "Target", target + cv::Point(15,5),
                  cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,255,0));
    }

    if(front.x == -1 || rear.x == -1 || target.x == -1) {
        cv::putText(debugFrame, "Searching for markers...", cv::Point(10,30),
                  cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255,255,255));
        cv::imshow("Color Debug", debugFrame);
        cv::waitKey(1);
        throw std::runtime_error("Not all markers detected");
    }

    // Расчет угла и расстояния
    cv::Point robotVector = front - rear;
    cv::Point targetVector = target - front;
    
    double robotLength = cv::norm(robotVector);
    double targetLength = cv::norm(targetVector);
    
    if(robotLength < 10 || targetLength < 10) {
        throw std::runtime_error("Markers too close to each other");
    }

    double angle = atan2(robotVector.x*targetVector.y - robotVector.y*targetVector.x,
                        robotVector.x*targetVector.x + robotVector.y*targetVector.y) * 180 / CV_PI;
    
    double distance = cv::norm(targetVector);

    // Визуализация
    cv::line(debugFrame, rear, front, cv::Scalar(255, 255, 0), 2);
    cv::line(debugFrame, front, target, cv::Scalar(0, 255, 255), 2);
    
    std::string angleText = "Angle: " + std::to_string((int)angle) + " deg";
    std::string distanceText = "Distance: " + std::to_string((int)distance) + " px";
    
    cv::putText(debugFrame, angleText, cv::Point(10,30),
               cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255,255,255));
    cv::putText(debugFrame, distanceText, cv::Point(10,60),
               cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255,255,255));
    
    cv::imshow("Color Debug", debugFrame);
    cv::waitKey(1);

    return {angle, distance};
}

// Основная функция поиска цвета
cv::Point ImageProcessorCommandReceiver::findColorCenter(const cv::Mat& hsvImage, 
                                                      const cv::Scalar& lower, 
                                                      const cv::Scalar& upper) {
    cv::Mat mask;
    cv::inRange(hsvImage, lower, upper, mask);

    // Морфологические операции
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5,5));
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

    // Поиск контуров
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    if(contours.empty()) return cv::Point(-1, -1);

    auto largestContour = std::max_element(contours.begin(), contours.end(),
        [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) {
            return cv::contourArea(a) < cv::contourArea(b);
        });

    cv::Moments m = cv::moments(*largestContour);
    return cv::Point((int)(m.m10/m.m00), (int)(m.m01/m.m00));
}

// Специальная функция для поиска красного (использует два диапазона)
cv::Point ImageProcessorCommandReceiver::findRedCenter(const cv::Mat& hsvImage) {
    cv::Mat mask1, mask2, combinedMask;
    
    cv::inRange(hsvImage, redLower1, redUpper1, mask1);
    cv::inRange(hsvImage, redLower2, redUpper2, mask2);
    cv::bitwise_or(mask1, mask2, combinedMask);

    // Морфологические операции
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5,5));
    cv::morphologyEx(combinedMask, combinedMask, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(combinedMask, combinedMask, cv::MORPH_CLOSE, kernel);

    // Поиск контуров
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(combinedMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    if(contours.empty()) return cv::Point(-1, -1);

    auto largestContour = std::max_element(contours.begin(), contours.end(),
        [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) {
            return cv::contourArea(a) < cv::contourArea(b);
        });

    cv::Moments m = cv::moments(*largestContour);
    return cv::Point((int)(m.m10/m.m00), (int)(m.m01/m.m00));
}

void ImageProcessorCommandReceiver::notifyCommandReceived(double angle, double distance) {
    std::cout << "[Camera] Executing command: angle " << angle
              << ", distance " << distance << std::endl;
}