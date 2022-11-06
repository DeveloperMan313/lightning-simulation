cmake_minimum_required(VERSION 3.1)

set(TARGET_NAME lightning-simulation)
project(${TARGET_NAME})

set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR "C:/SFML-2.5.1/lib/cmake/SFML")
find_package(SFML 2.5 COMPONENTS graphics REQUIRED)

set(SOURCE_FILES src/main.cpp src/Simulation.cpp)
set(HEADER_FILES include/Simulation.h)

add_executable(${TARGET_NAME} ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(${TARGET_NAME} PUBLIC include)
target_link_libraries(${TARGET_NAME} sfml-graphics)
target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)
