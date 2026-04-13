cmake_minimum_required(VERSION 3.16)
project(VideoSegmentSelector LANGUAGES CXX)

# Use C++17 (needed for clamp etc.)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT DEFINED CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/6.9.1/gcc_64")
endif()
# Find Qt (Widgets/Core/Gui/Concurrent)
find_package(Qt6 COMPONENTS Widgets Core Gui Concurrent REQUIRED)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Source files
set(SOURCES
    main.cpp
    segment_selector.cpp
    video_player.cpp
    gif.cpp
)

# Enable Qt's automoc for signals/slots
set(CMAKE_AUTOMOC ON)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})
# Include dirs
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${OpenCV_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Widgets
        Qt6::Gui
        Qt6::Core
        Qt6::Concurrent
        ${OpenCV_LIBS}
)

# On Linux, make sure RPATH works
if(UNIX AND NOT APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INSTALL_RPATH "$ORIGIN"
    )
endif()
