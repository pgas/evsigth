cmake_minimum_required(VERSION 3.0.0)
project(evsigth VERSION 0.1.0)

add_subdirectory(libevent)

include(CTest)
enable_testing()

add_executable(evsigth main.cpp EventApplication.cpp FifoControl.cpp)
target_link_libraries(evsigth event event_pthreads)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

