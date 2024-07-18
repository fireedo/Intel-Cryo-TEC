cmake_minimum_required(VERSION 3.10)
project(IntelCryoTEC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the include directory to the include path
include_directories(${CMAKE_SOURCE_DIR}/include)

# Set policy for Boost
cmake_policy(SET CMP0167 NEW)

# Find required packages
find_package(Boost 1.85.0 REQUIRED COMPONENTS system)
find_package(nlohmann_json 3.11.3 REQUIRED)

# Qt5 configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt5 COMPONENTS Widgets PrintSupport REQUIRED)

# Standalone application
add_executable(intel_cryo_tec
    src/standalone/intel_cryo_tec.cpp
)

target_link_libraries(intel_cryo_tec PRIVATE
    Boost::system
    nlohmann_json::nlohmann_json
    pthread
)

# GUI application
set(GUI_SOURCES
    src/gui/main.cpp
    src/gui/intelcryotecapp.cpp
    src/gui/graphdialog.cpp
    src/gui/qcustomplot.cpp
)

set(GUI_HEADERS
    src/gui/intelcryotecapp.h
    src/gui/graphdialog.h
    src/gui/qcustomplot.h
)

# Add resource files
set(RESOURCES
    src/gui/resources.qrc
)

add_executable(intel_cryo_tec_gui
    ${GUI_SOURCES}
    ${GUI_HEADERS}
    ${RESOURCES}
)

target_link_libraries(intel_cryo_tec_gui PRIVATE
    Qt5::Widgets
    Qt5::PrintSupport
    Boost::system
    nlohmann_json::nlohmann_json
    pthread
)

# Installation
install(TARGETS intel_cryo_tec intel_cryo_tec_gui
    RUNTIME DESTINATION bin
)

install(FILES ${CMAKE_SOURCE_DIR}/intel-cryo-tec.service
    DESTINATION /etc/systemd/system
)

install(FILES
    ${CMAKE_SOURCE_DIR}/images/blue.png
    ${CMAKE_SOURCE_DIR}/images/red.png
    DESTINATION share/intel_cryo_tec/images
)

# Note: The desktop file is not in your current directory listing.
# If you have it, uncomment the following lines:
# install(FILES ${CMAKE_SOURCE_DIR}/intel-cryo-tec-monitor.desktop
#     DESTINATION share/applications
# )
