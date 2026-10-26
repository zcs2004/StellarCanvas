cmake_minimum_required(VERSION 3.21)
project(QtPackageProbe LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Qml Quick QuickWidgets QuickControls2)

foreach(target IN ITEMS Qt6::Qml Qt6::Quick Qt6::QuickWidgets Qt6::QuickControls2)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "Missing imported target: ${target}")
    endif()
endforeach()
