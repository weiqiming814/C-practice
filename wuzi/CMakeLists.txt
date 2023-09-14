cmake_minimum_required(VERSION 3.5.1)

project(
  wuzi 
  VERSION 1.0
  LANGUAGES C)

add_library(main wuzi.c board.c board.h stone.c stone.h win.c win.h)
add_executable(wuzi wuzi.c board.c stone.c win.c)
target_link_libraries(wuzi PRIVATE main)
