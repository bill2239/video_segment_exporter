cmake_minimum_required(VERSION 3.16)
project(VideoSegmentSelector LANGUAGES CXX)

# Use C++17 (needed for clamp etc.)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT DEFINED CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/6.9.1/gcc_64")
endif()
# Find Qt (Widgets and Core)
find_package(Qt6 COMPONENTS Widgets Core Gui Concurrent REQUIRED)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Add include directories early for MOC to find headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    main.cpp
    src/segment_selector.cpp
    src/video_player.cpp
    src/gif.cpp
)

# Qt header files that need MOC processing
set(HEADERS
    include/segment_selector.h
    include/video_player.h
)

# Enable Qt's automoc for signals/slots
set(CMAKE_AUTOMOC ON)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
# Include dirs
target_include_directories(${PROJECT_NAME}
    PRIVATE
        include
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
