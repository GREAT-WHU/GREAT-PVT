#Minimum requirement of CMake version : 3.0.0
cmake_minimum_required(VERSION 3.0.0)

#Project name and version number
project(${LibGREAT} VERSION 0.1.0)

# add define
# ==========================================================================================================================================
add_definitions(-D GREAT_LibGREAT_LIBRARY)
# ==========================================================================================================================================

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

# set include path
include_directories(${Third_Eigen_ROOT})
include_directories(${LibGnutSrc})
include_directories(${LibGREATSrc})

#message(STATUS "                                                                     ")
message(STATUS " include path for ${LibGPP} is : ${Third_Eigen_ROOT}")
message(STATUS " include path for ${LibGREAT} is : ${LibGREATSrc}")
#message(STATUS "                                                                     ")

# for all the header files and source_files
file(GLOB_RECURSE header_files ${PROJECT_SOURCE_DIR} *.h *.hpp)
file(GLOB_RECURSE source_files ${PROJECT_SOURCE_DIR} *.cpp)
file(GLOB_RECURSE header_files ${PROJECT_SOURCE_DIR} *.h *.hpp)
file(GLOB_RECURSE source_files ${PROJECT_SOURCE_DIR} *.cpp)

# grouping
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    # add files to project
    source_group("CMake Files"  FILES CMakeLists.txt)
    source_group("Header Files" FILES header_files)
    source_group("Soruce Files" FILES source_files)

    # find files and put them into different groups
    foreach(fold ${file_fold})  
    file(GLOB_RECURSE tmp_header_files 
        ${PROJECT_SOURCE_DIR}/${fold}/*.h 
        ${PROJECT_SOURCE_DIR}/${fold}/*.hpp)
    file(GLOB_RECURSE tmp_source_files 
        ${PROJECT_SOURCE_DIR}/${fold}/*.cpp)
    source_group("Header Files\\${fold}"        FILES ${tmp_header_files})
    source_group("Source Files\\${fold}"        FILES ${tmp_source_files})
endforeach(fold)      
else()
    #message(STATUS "For Linux / unix, there is no need to group so detailed")
endif()

# grouping in VS
# ==========================================================================================================================================
if(${PrintFile})
    message(STATUS " ====================================================================================================================== ")
    message(STATUS " header_files_for_${PROJECT_NAME}" )
    foreach(list ${header_files})   
        message(STATUS " ==> ${list}")
    endforeach(list)   

    message(STATUS " source_files_for_${PROJECT_NAME}" )
    foreach(list ${source_files})   
        message(STATUS " ==> ${list}")
    endforeach(list)   
    message(STATUS " ====================================================================================================================== ")
endif()

# link project
# ==========================================================================================================================================
add_library(${PROJECT_NAME} SHARED ${header_files} ${source_files})
# ==========================================================================================================================================

# link third
# ==========================================================================================================================================
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    link_directories(${BUILD_DIR}/Lib/Debug)
    link_directories(${BUILD_DIR}/Lib/Release)
    link_directories(${BUILD_DIR}/Lib/RelWithDebInfo)
    link_directories(${BUILD_DIR}/Lib/MinSizeRel)
else()
    link_directories(${BUILD_DIR}/Lib)
endif()
# ==========================================================================================================================================
if(NOT Main_USE_AMODE_EXELIB)
    target_link_libraries(${PROJECT_NAME} 
        ${LibGnut})
    add_dependencies(${PROJECT_NAME} 
        ${LibGnut})
endif()

set(pvt     GREAT_PVT)
add_subdirectory(${ROOT}/app/${pvt}    ${BUILD_DIR}/${pvt})
SET_PROPERTY(TARGET ${pvt}     PROPERTY FOLDER "app")
