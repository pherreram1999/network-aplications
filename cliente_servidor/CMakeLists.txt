cmake_minimum_required(VERSION 4.0)
project(cliente_servidor C)

set(CMAKE_C_STANDARD 23)

add_executable(server server.c)
add_executable(client client.c)
