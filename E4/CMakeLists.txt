cmake_minimum_required(VERSION 3.10)
project(embeded_network C)

set(CMAKE_C_STANDARD 99)

add_executable(embeded_network server.c client.c)