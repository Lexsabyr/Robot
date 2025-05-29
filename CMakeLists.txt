cmake_minimum_required(VERSION 3.10)
project(RobotControl)

set(CMAKE_CXX_STANDARD 14)


add_definitions(-D_CRT_SECURE_NO_WARNINGS)


# Настройка OpenCV (используем ваш проверенный подход)
set(OpenCV_DIR "C:/opencv/build/x64/vc16/lib")  # Тот же путь, что в рабочем проекте
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Указываем путь к заголовочным файлам Mosquitto
include_directories("C:/Program Files/mosquitto")

# Указываем путь к библиотекам Mosquitto
link_directories("C:/Program Files/mosquitto/devel")


# Для Windows - явное указание путей к Mosquitto
if(WIN32)
    # Укажите правильный путь к вашей установке Mosquitto
    set(MOSQUITTO_DIR "C:/Program Files/mosquitto")
    
    # Ищем заголовочные файлы
    find_path(MOSQUITTO_INCLUDE_DIR mosquitto.h
        PATHS ${MOSQUITTO_DIR}
        PATH_SUFFIXES include
    )
    
    # Ищем библиотеки
    find_library(MOSQUITTO_LIBRARY
        NAMES mosquitto mosquitto.lib
        PATHS ${MOSQUITTO_DIR}
        PATH_SUFFIXES lib
    )
    
    include_directories(${MOSQUITTO_INCLUDE_DIR})
    link_directories(${MOSQUITTO_DIR})

endif()

# Исполняемый файл
add_executable(RobotControl
    src/main.cpp
    src/simulated_motor.cpp
    src/server_command_receiver.cpp
    src/robot_controller.cpp
    src/image_processor_command_receiver.cpp
)

target_include_directories(RobotControl PRIVATE 
    include
    ${MOSQUITTO_INCLUDE_DIR}
)

target_link_libraries(RobotControl
    ${MOSQUITTO_LIBRARY}
    ${OpenCV_LIBS}
)