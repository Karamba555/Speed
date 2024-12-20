#
# mgui (marvell gui) project cmake
###################################

cmake_minimum_required(VERSION 2.6)

PROJECT(MGUI)

ADD_DEFINITIONS(-Os --std=gnu99 -Wmissing-declarations -DMARVELL_EXTENDED)
OPTION(BUILD_EXAMPLES "build examples" OFF)

# define resource files target install
SET(DATADIR ${CMAKE_INSTALL_PREFIX}/share/mgui)
SET(FONTDIR ${DATADIR}/fonts)
SET(IMAGEDIR ${DATADIR}/images)

###SET_DIRECTORY_PROPERTIES(PROPERTIES
###	ADDITIONAL_MAKE_CLEAN_FILES "Makefile;ipkg-install;ipkg-pxa1826;Makefile;CMakeCache.txt;CMakeFiles;cmake_install.cmake;install_manifest.txt;")

IF(OPT_USE_DFB)
# find DirectFB-1.7.0
# directfb.pc defines clags with the necessary include directories
INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(DIRECTFB REQUIRED directfb>=1.7.0)
ADD_DEFINITIONS(${DIRECTFB_CFLAGS})
LIST(APPEND LIBS ${DIRECTFB_LIBRARIES})
ENDIF()

# find necessary libraries
INCLUDE(CheckLibraryExists)

IF(OPT_ENABLE_LIBLOG)
CHECK_LIBRARY_EXISTS(log __android_log_buf_write "" MGUI_LOGCAT)
IF(MGUI_LOGCAT)
	LIST(APPEND LIBS log)
ENDIF(MGUI_LOGCAT)
ENDIF(OPT_ENABLE_LIBLOG)

CHECK_LIBRARY_EXISTS(rilutil rilutil_parseResponse "" MGUI_RILUTIL)
IF(MGUI_RILUTIL)
	LIST(APPEND LIBS rilutil)
ENDIF(MGUI_RILUTIL)

CHECK_LIBRARY_EXISTS(prop2uci property_get "" MGUI_PROP2UCI)
IF(MGUI_PROP2UCI)
	LIST(APPEND LIBS prop2uci)
ENDIF(MGUI_PROP2UCI)

find_library(json NAMES json-c json)
LIST(APPEND LIBS ubox ubus uci ${json} blobmsg_json)

find_library(librt NAMES rt librt)
LIST(APPEND LIBS ${librt})

CONFIGURE_FILE(mgui_config.h.in mgui_config.h @ONLY)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# define sources and linked libraries
SET(SOURCES
	Object.c Image.c Button.c String.c
	TextBox.c mgui_utils.c main.c mgui.c mgui_icons.c mgui_ubus.c
	mgui_ril.c mgui_charger.c mgui_onkey.c mgui_wifi.c mgui_hawk.c 
	mgui_version.c mgui_ubus.c)

#IF(OPT_USE_LVGL)
	add_subdirectory(lvgl)
	LIST(APPEND LIBS lvgl)
	SET(SOURCES ${SOURCES} gpio.c lvgl/asr_lvgl.c)
	INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/lvgl)
#ENDIF()

IF(OPT_USE_MGUI_READ_SMS)
	SET(SOURCES ${SOURCES} mgui_sms.c)
ENDIF()
# create and install executable
ADD_EXECUTABLE(mgui ${SOURCES})
TARGET_LINK_LIBRARIES(mgui ${LIBS})
INSTALL(TARGETS mgui RUNTIME DESTINATION bin)

# install data directories
INSTALL(DIRECTORY data/fonts data/images DESTINATION share/mgui)

ADD_SUBDIRECTORY(examples)
