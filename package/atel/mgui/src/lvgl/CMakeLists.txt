include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lvgl
    ${CMAKE_CURRENT_SOURCE_DIR}/lv_drivers
    ${CMAKE_CURRENT_SOURCE_DIR}/lv_lib_png
    ${CMAKE_CURRENT_SOURCE_DIR}/lv_lib_qrcode
)
remove_definitions(-Werror)
file(GLOB LVGL_LIB_SRCS "lv_drivers/display/*.c" "lv_drivers/input/*.c" lv_lib_png/*.c lv_lib_qrcode/*.c
 "lvgl/src/lv_core/*.c" "lvgl/src/lv_draw/*.c" "lvgl/src/lv_font/*.c"
 "lvgl/src/lv_hal/*.c" "lvgl/src/lv_misc/*.c" "lvgl/src/lv_themes/*.c" "lvgl/src/lv_widgets/*.c")
add_library(lvgl ${LVGL_LIB_SRCS})
