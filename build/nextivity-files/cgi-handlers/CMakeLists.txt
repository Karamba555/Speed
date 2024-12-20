
cmake_minimum_required(VERSION 2.8.12)
cmake_policy(SET CMP0048 NEW)

project(power-cycle
        VERSION 0.1
        LANGUAGES C )
set(C_STANDARD 99)

set(CMAKE_C_FLAGS "-Wall -W -Werror -O3 -fPIE -fstack-protector-strong")
link_libraries("-Wl,-znoexecstack -pie")

add_executable(power-cycle power-cycle.c)
target_link_libraries(power-cycle uci)

add_executable(reboot reboot.c)
target_link_libraries(reboot uci)

add_executable(modem-status modem-status.c)
target_link_libraries(modem-status uci)

add_executable(factory.cgi factory.cgi.c)
target_link_libraries(factory.cgi json-c)

add_executable(cgi-userdetails cgi-userdetails.c)
target_link_libraries(cgi-userdetails json-c uci)

add_compile_definitions(AWC_DISPLAY_NAME="${AWC_DISPLAY_NAME}")
