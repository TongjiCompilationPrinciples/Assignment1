cmake_minimum_required(VERSION 3.27)
project(compiling)

set(CMAKE_CXX_STANDARD 17)

add_executable(compiling main.cpp
        LexAna/Ana.h
        LexAna/Ana.cpp
        LexAna/Deceleration.h
        GraAna/GrammaticalAna.h
        GraAna/GrammaticalAna.cpp)
