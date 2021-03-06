cmake_minimum_required(VERSION 3.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(ARK)

#3rd cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

#check cxx flag in linux system
if(UNIX)
	#linux
	include(CheckCXXCompilerFlag)
	CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
	if(COMPILER_SUPPORTS_CXX11)
	    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	else()
		message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
	endif()
endif()

########################################################################

set(ROOT_DIR ${PROJECT_SOURCE_DIR})

include_directories(
    ${ROOT_DIR}
    ${ROOT_DIR}/Dep
    ${ROOT_DIR}/Dep/protobuf/src
    ${ROOT_DIR}/Frame
    ${ROOT_DIR}/Dep/brynet/src
)

if(UNIX)
    #linux
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fPIC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O0 -Wall -g -ggdb -DDEBUG -fPIC")
        set(DEP_LIB_DIR ${ROOT_DIR}/Dep/lib/ ${ROOT_DIR}/Dep/lib/Debug/ ${ROOT_DIR}/Bin/Debug)
    else()
        set(CMAKE_BUILD_TYPE "Release")
        set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fPIC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -Wall -fPIC")
        set(DEP_LIB_DIR ${ROOT_DIR}/Dep/lib/ ${ROOT_DIR}/Dep/lib/Release/ ${ROOT_DIR}/Bin/Release)
    endif()
    set(LDFLAGS "$ENV{LDFLAGS} -stdlib=libc++")
    ADD_DEFINITIONS(-std=c++11)
    set(SDK_OUTPUT_DIR ${ROOT_DIR}/Bin/Comm/${CMAKE_BUILD_TYPE})

    set (CXX_FLAGS
        -g
        -w
        #-Wall
        #-Wno-unused-parameter
        #-Wno-unused-function
        #-Wunused-variable
        #-Wunused-value
        #-Wshadow
        #-Wcast-qual
        #-Wcast-align
        #-Wwrite-strings
        #-Wsign-compare
        #-Winvalid-pch
        -fms-extensions
        #-Wfloat-equal
        #-Wextra
        -fPIC
    )

    SET(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g -ggdb -D_DEBUG")
    SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -g -ggdb -DNDEBUG")
    
    if(ENABLE_COVERAGE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
    endif()
    
else()
    #windows
    include_directories(${ROOT_DIR}/Dep/libevent/WIN32-code/nmake)
    set(CMAKE_CXX_FLAGS "-D_X64 -D_WINDOWS /EHsc")
    set(DEP_LIB_DIR ${ROOT_DIR}/Dep/lib ${ROOT_DIR}/Bin/Comm)
    set(SDK_OUTPUT_DIR ${ROOT_DIR}/Bin/Comm)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

string (REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
link_directories(${DEP_LIB_DIR})

#log
message("ROOT dir=${ROOT_DIR}")
message("SDK Output dir=${SDK_OUTPUT_DIR}")
message("DEP lib dir=${DEP_LIB_DIR}")

#codecov
find_package(codecov)

##################################################################
message("Start to build all...")
add_subdirectory(Frame/SDK)
add_subdirectory(Frame/Server)
add_subdirectory(Plugins)
#add_subdirectory(Frame/Examples)
#add_subdirectory(Frame/Tools)
