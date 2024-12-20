
cmake_minimum_required(VERSION 2.8.12)
cmake_policy(SET CMP0048 NEW)

project(password-megafi
        VERSION 0.1
        LANGUAGES C )
set(C_STANDARD 99)

set(CMAKE_C_FLAGS "-Wall -W -Werror -O3 -fPIE -fstack-protector-strong")
link_libraries("-Wl,-znoexecstack -pie")

file(GLOB SOURCES
	main.c
)

add_executable(password-megafi ${SOURCES})
target_link_libraries(password-megafi)
