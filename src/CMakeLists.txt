#Minimum requirement of CMake version : 3.0.0
cmake_minimum_required(VERSION 3.0.0)

#Project name and version number
project(GREAT-PVT VERSION 0.1.0)

#information message
message(STATUS " ====================================================================================================================== ")

message(STATUS "    .oooooo.    ooooooooo.   oooooooooooo       .o.       ooooooooooooo")
message(STATUS "   d8P'  `Y8b   `888   `Y88. `888'     `8      .888.      8'   888   `8")
message(STATUS "  888            888   .d88'  888             .8\"888.         888     ")
message(STATUS "  888            888ooo88P'   888oooo8       .8' `888.         888     ")
message(STATUS "  888     ooooo  888`88b.     888    \"      .88ooo8888.       888     ")
message(STATUS "  `88.    .88'   888  `88b.   888       o  .8'     `888.       888     ")
message(STATUS "   `Y8bood8P'   o888o  o888o o888ooooood8 o88o     o8888o     o888o    ")

message(STATUS " ====================================================================================================================== ")

message(STATUS " The GREAT (GNSS+ Research, Application and Teaching) software is designed and developed at Wuhan University for        ")
message(STATUS " scientific and engineering application and teaching in geodesy and navigation fields.                                     ")
message(STATUS "                                                                                                                        ")
message(STATUS " It is written in standard C++. GREAT-PVT supports multi-frequency multi-system PPP and RTK filtering solutions.        ")

message(STATUS " ====================================================================================================================== ")

# compiler setting
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

# Compilation options settings
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_compile_options(-O3)
    # add_compile_options(-w)
    add_compile_options(-m64)
    add_compile_options(-lz)
    add_compile_options(-lstdc++)
    add_compile_options(-Wall)
    add_compile_options(-w)
    SET(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}_Linux)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
    SET(CMAKE_INSTALL_RPATH "\${ORIGIN}/../Lib") 
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    SET(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}_Darwin)
    add_compile_options(-pthread)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    SET(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}_Windows)
    add_compile_options(/bigobj)
    add_compile_options(/MP4)
    add_compile_options(/w)
endif()

if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -pthread")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# Choose different compilation configurations according to VS compilation
if(CMAKE_BUILD_TYPE MATCHES "Release")
    set(CMAKE_BUILD_POSTFIX "${CMAKE_RELEASE_POSTFIX}")
elseif(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_BUILD_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
elseif(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    set(CMAKE_BUILD_POSTFIX "${CMAKE_RELWITHDEBINFO_POSTFIX}")
elseif(CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
    set(CMAKE_BUILD_POSTFIX "${CMAKE_MINSIZEREL_POSTFIX}")
else()
    set(CMAKE_BUILD_POSTFIX "")
endif()

# Set the ROOT and subdirectory, you should put the CMakeList.txt in these file directories
set(ROOT ${PROJECT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${BUILD_DIR}/Bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR}/Bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR}/Lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR}/Lib)

set(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_RELEASE_POSTFIX "")
set(CMAKE_RELWITHDEBINFO_POSTFIX "rd")
set(CMAKE_MINSIZEREL_POSTFIX "s")

# Output Messages for debug the Cmake
message(STATUS "operation system is : ${CMAKE_SYSTEM}")
message(STATUS "current platform is : ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMake version    is : ${CMAKE_SYSTEM_VERSION}")
message(STATUS "C compiler       is : ${CMAKE_C_COMPILER}")
message(STATUS "C++ compiler     is : ${CMAKE_CXX_COMPILER}")
message(STATUS "Build directory  is : ${BUILD_DIR}")
message(STATUS "The program main directory is : ${ROOT}")


# set lib name
set(file_fold 
    spdlog gexport gio gutils gcoders gmodels gall gdata gset pugixml gproc gprod gambfix newmat)
message(${file_fold})
set(PrintFile FALSE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# link third party
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    #For windows
    # ==============================================================================
    if (NOT DEFINED Third_Eigen_ROOT)
        find_path(Third_Eigen_ROOT       HINTS "${Third_Eigen_ROOT}"        "$ENV{Third_Eigen_ROOT}            ")
    endif()
    # ==============================================================================
else()
    # For linux
    # write write_your_path_here
    # ==============================================================================
    if (NOT DEFINED Third_Eigen_ROOT)
        set(Third_Eigen_ROOT       ${ROOT}/third-party/Eigen)
    endif()
    # ==============================================================================
endif()

# ========================================================================================================================================
set(LibGnut          LibGnut)
set(LibGREAT         LibGREAT)
set(LibGnutSrc          ${PROJECT_SOURCE_DIR}/${LibGnut})
set(LibGREATSrc         ${PROJECT_SOURCE_DIR}/${LibGREAT})
add_subdirectory(${LibGnutSrc}       ${BUILD_DIR}/${LibGnut})
add_subdirectory(${LibGREATSrc}      ${BUILD_DIR}/${LibGREAT})
SET_PROPERTY(TARGET ${LibGnut}       PROPERTY FOLDER "LIB")
SET_PROPERTY(TARGET ${LibGREAT}      PROPERTY FOLDER "LIB")

# ========================================================================================================================================
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    #For windows
    message(STATUS "                                                       ")
    message(STATUS "==================    Windows    ======================")
    SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)    
else()
    # For linux
    message(STATUS "                                                       ")
    message(STATUS "==================    Not Windows =====================")
endif()
