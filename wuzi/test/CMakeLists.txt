cmake_minimum_required(VERSION 3.5.1)

project(
  wuzi_test 
  VERSION 1.0
  LANGUAGES C)

add_library(main test.c board.h board.c stone.c stone.h win.c win.h)
add_executable(wuzi_test test.c board.c stone.c win.c)
target_link_libraries(wuzi_test PRIVATE)
