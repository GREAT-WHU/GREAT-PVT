# ============================================================================
# Project: LibGREAT – Shared library subproject
# Purpose: Build a shared library (libLibGREAT.so / .dylib / .dll) that depends
#          on LibGnut and Eigen. Optionally, if Main_USE_AMODE_EXELIB is NOT set,
#          it links against LibGnut; otherwise, it skips that dependency.
#          It also adds a subproject (GREAT_PVT) as an executable/app.
# Dependencies: This CMakeLists.txt is intended to be added via add_subdirectory()
#               from a parent project. It expects the following variables to be
#               defined in the parent scope (or cached):
#                 - ROOT             : Project source root
#                 - BUILD_DIR        : Platform‑specific binary output directory
#                 - Third_Eigen_ROOT : (optional) Path to Eigen; if not set, it
#                                      will be searched or defaulted.
#                 - LibGnutSrc       : Source directory of LibGnut (for includes)
#                 - LibGREATSrc      : Source directory of this library itself
#                 - file_fold        : List of sub‑folders for source grouping.
#                 - PrintFile        : Boolean flag to print file lists for debug.
#                 - Main_USE_AMODE_EXELIB : If defined and TRUE, skip linking
#                                           against LibGnut (used to avoid
#                                           circular dependencies).
# Notes:
#   - The library defines the preprocessor macro GREAT_LibGREAT_LIBRARY.
#   - On Windows, source files are grouped into folders in Visual Studio.
#   - On Linux/macOS, no extra system libraries are linked (unlike LibGnut).
#   - This script also adds an executable target called ${pvt} (GREAT_PVT)
#     from the subdirectory ROOT/app/${pvt}.
# ============================================================================

# ---- 1. Minimum CMake version and project ----
cmake_minimum_required(VERSION 3.5)

# Project name is taken from the variable ${LibGREAT} (must be defined earlier
# in the parent project). We also set a version number.
project(${LibGREAT} VERSION 0.1.0)

# ---- 2. Preprocessor definitions ----
# Define a macro that indicates this is the LibGREAT library build (used to
# conditionally export symbols for Windows DLLs).
# ==========================================================================================================================================
add_definitions(-D GREAT_LibGREAT_LIBRARY)
# ==========================================================================================================================================

# ---- 3. Locate the Eigen third‑party library ----
# Eigen is header‑only; we only need its include directory.
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    # On Windows: use find_path to search, honouring user‑set variables or env vars.
    # ==============================================================================
    if (NOT DEFINED Third_Eigen_ROOT)
        find_path(Third_Eigen_ROOT       HINTS "${Third_Eigen_ROOT}"        "$ENV{Third_Eigen_ROOT}            ")
    endif()
    # ==============================================================================
else()
    # On Linux / macOS: assume Eigen is placed in a fixed location under ROOT.
    # ==============================================================================
    if (NOT DEFINED Third_Eigen_ROOT)
        set(Third_Eigen_ROOT       ${ROOT}/third-party/Eigen)
    endif()
    # ==============================================================================
endif()

# ---- 4. Include directories ----
# Add Eigen's include directory, LibGnut's source directory (to include its
# public headers), and this library's own source directory.
include_directories(${Third_Eigen_ROOT})
include_directories(${LibGnutSrc})
include_directories(${LibGREATSrc})

# Optional debug output to show the include paths (commented out for normal use)
#message(STATUS "                                                                     ")
message(STATUS " include path for Eigen is : ${Third_Eigen_ROOT}")   # Note: LibGPP is likely a typo; kept as is.
message(STATUS " include path for ${LibGREAT} is : ${LibGREATSrc}")
#message(STATUS "                                                                     ")

# ---- 5. Collect source and header files ----
# Use GLOB_RECURSE to recursively find all .h, .hpp, .cpp files in the project
# source directory. Note: GLOB_RECURSE can miss newly added files; explicit
# listing is more reliable for production.
file(GLOB_RECURSE header_files ${PROJECT_SOURCE_DIR} *.h *.hpp)
file(GLOB_RECURSE source_files ${PROJECT_SOURCE_DIR} *.cpp)
# These two lines are repeated (probably a copy‑paste error); kept as in the original.
file(GLOB_RECURSE header_files ${PROJECT_SOURCE_DIR} *.h *.hpp)
file(GLOB_RECURSE source_files ${PROJECT_SOURCE_DIR} *.cpp)

# ---- 6. Source grouping for Visual Studio (Windows only) ----
# On Windows, we organise the files into virtual folders in the IDE.
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    # Group the top‑level CMakeLists.txt, headers, and sources.
    source_group("CMake Files"  FILES CMakeLists.txt)
    source_group("Header Files" FILES header_files)
    source_group("Soruce Files" FILES source_files)   # Typo: "Soruce" kept as original.

    # For each sub‑folder listed in ${file_fold}, gather its headers and sources
    # and put them into a dedicated subgroup under "Header Files" and "Source Files".
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
    # On non‑Windows platforms, we skip detailed grouping (not needed).
    #message(STATUS "For Linux / unix, there is no need to group so detailed")
endif()

# ---- 7. Optional debug output: print all collected files ----
# If PrintFile is TRUE, list all headers and sources to the CMake output.
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

# ---- 8. Create the shared library target ----
# Build a shared library from the collected source and header files.
# ==========================================================================================================================================
add_library(${PROJECT_NAME} SHARED ${header_files} ${source_files})
# ==========================================================================================================================================

# ---- 9. Library search paths (linker) ----
# ==========================================================================================================================================
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    # On Windows: add linker search paths for all build configurations.
    # These paths point to where the dependency libraries (like LibGnut) are built.
    link_directories(${BUILD_DIR}/Lib/Debug)
    link_directories(${BUILD_DIR}/Lib/Release)
    link_directories(${BUILD_DIR}/Lib/RelWithDebInfo)
    link_directories(${BUILD_DIR}/Lib/MinSizeRel)
else()
    # On Linux/macOS: the library search path is simply ${BUILD_DIR}/Lib.
    link_directories(${BUILD_DIR}/Lib)
endif()
# ==========================================================================================================================================

# ---- 10. Link against LibGnut (conditional) ----
# If Main_USE_AMODE_EXELIB is not defined (or set to FALSE), link this library
# against LibGnut. This is useful to avoid circular dependencies in certain
# build configurations.
if(NOT Main_USE_AMODE_EXELIB)
    target_link_libraries(${PROJECT_NAME} 
        ${LibGnut})
    # Ensure that LibGnut is built before LibGREAT (parallel build ordering)
    add_dependencies(${PROJECT_NAME} 
        ${LibGnut})
endif()

# ---- 11. Add the application subproject ----
# The variable pvt is set to "GREAT_PVT", and we add its subdirectory located
# at ${ROOT}/app/${pvt}. The binary output goes to ${BUILD_DIR}/${pvt}.
set(pvt     GREAT_PVT)
add_subdirectory(${ROOT}/app/${pvt}    ${BUILD_DIR}/${pvt})
# Place the executable target (named GREAT_PVT) into the "app" folder in the IDE
SET_PROPERTY(TARGET ${pvt}     PROPERTY FOLDER "app")