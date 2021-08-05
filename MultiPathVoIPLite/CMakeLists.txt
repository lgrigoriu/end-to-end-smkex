cmake_minimum_required(VERSION 3.4)
project("Multi-Path VoIP Lite")

FIND_PACKAGE(OpenSSL REQUIRED)
FIND_PACKAGE(ALSA REQUIRED)   

include (FindPkgConfig)
if (PKG_CONFIG_FOUND) # true if pkg-config works on the system
  pkg_check_modules(PJSIP REQUIRED libpjproject)
endif()



include_directories(${PJSIP_INCLUDE_DIRS} ${ALSA_INCLUDE_DIRS})
link_directories(${PJSIP_LIBRARY_DIRS} ${ALSA_LIBRARIES} /home/liliana/pjproject-2.10/pjlib/bin /home/liliana/pjproject-2.10/pjlib/bin/ /home/liliana/pjproject-2.10/ /home/liliana/pjproject-2.10/pjlib-util/bin /home/liliana/pjproject-2.10/pjmedia/bin /home/liliana/pjproject-2.10/pjnath/bin/)

include_directories(smkex/include auto_resend/include base_service/include buddy/include call_manager/include config/include data_msg/include sip_stack/include tests/include user_account/include utils/include /home/liliana/pjproject-2.10/ /home/liliana/pjproject-2.10/pjsip/include /home/liliana/pjproject-2.10/pjlib/include/ /home/liliana/pjproject-2.10/pjlib/include/  /home/liliana/pjproject-2.10/pjlib-util/include /home/liliana/pjproject-2.10/pjmedia/include/ /home/liliana/pjproject-2.10/pjnath/include/)

file(GLOB_RECURSE sources smkex/src/*.cpp auto_resend/src/*.cpp  base_service/src/*.cpp  buddy/src/*.cpp  call_manager/src/*.cpp  config/src/*.cpp  data_msg/src/*.cpp sip_stack/src/*.cpp  tests/src/*.cpp  user_account/src/*.cpp utils/src/*.cpp)
set(CMAKE_PREFIX_PATH /home/liliana/pjproject-2.10)

set(MP_DEBUG 1)
set(MP_OS_LINUX 1)
set(MP_LOG_TO_CONSOLE 1)
set(CMAKE_PREFIX_PATH "/home/liliana/pjproject-2.10/pjsip-apps/build/")
add_executable(MpVoIPLite ${sources})

enable_testing(TRUE)

add_test(MpVoIPLite ./MpVoIPLite)

SET (CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -O0 -Wall -fprofile-arcs -ftest-coverage")

target_link_libraries(MpVoIPLite ${PJSIP_LIBRARIES})
