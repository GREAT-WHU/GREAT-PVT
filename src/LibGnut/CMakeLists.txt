# ============================================================================
# Project: LibGnut – Shared library subproject
# Purpose: Build a shared library (libLibGnut.so / .dylib / .dll) that uses
#          Eigen (header‑only) and may link to other dependencies.
# Dependencies: This CMakeLists.txt is intended to be added via add_subdirectory()
#               from a parent project. It expects the following variables to be
#               defined in the parent scope (or cached):
#                 - ROOT             : Project source root
#                 - BUILD_DIR        : Platform‑specific binary output directory
#                 - Third_Eigen_ROOT : (optional) Path to Eigen; if not set, it
#                                      will be searched or defaulted.
#                 - LibGnutSrc       : (optional) Path to this library's source;
#                                      if not set, the parent may define it.
#                 - file_fold        : List of sub‑folders for source grouping.
#                 - PrintFile        : Boolean flag to print file lists for debug.
# Notes:
#   - The library defines the preprocessor macro GREAT_LibGnut_LIBRARY.
#   - On Windows, source files are grouped into folders in Visual Studio.
#   - On Linux/macOS, the library is linked against -lz (zlib).
# ============================================================================

# ---- 1. Minimum CMake version and project ----
cmake_minimum_required(VERSION 3.5)

# Project name is taken from the variable ${LibGnut} (must be defined earlier
# in the parent project). We also set a version number.
project(${LibGnut} VERSION 0.1.0)

# ---- 2. Preprocessor definitions ----
# Define a macro that indicates this is the LibGnut library build (used to
# conditionally export symbols for Windows DLLs).
# ==========================================================================================================================================
add_definitions(-D GREAT_LibGnut_LIBRARY)
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
# Add Eigen's include directory and this library's own source directory so that
# its internal headers can be found.
include_directories(${Third_Eigen_ROOT})
include_directories(${LibGnutSrc})

# Optional debug output to show the include paths (commented out for normal use)
#message(STATUS "                                                                     ")
message(STATUS " include path for Eigen is : ${Third_Eigen_ROOT}")
message(STATUS " include path for ${LibGnut} is : ${LibGnutSrc}")
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

# ---- 9. Library linking (platform‑specific) ----
# ==========================================================================================================================================
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    # On Windows: add linker search paths for all build configurations.
    # These paths point to where the dependency libraries (like LibGREAT) are built.
    link_directories(${BUILD_DIR}/Lib/Debug)
    link_directories(${BUILD_DIR}/Lib/Release)
    link_directories(${BUILD_DIR}/Lib/RelWithDebInfo)
    link_directories(${BUILD_DIR}/Lib/MinSizeRel)
else()
    # On Linux/macOS: the library search path is simply ${BUILD_DIR}/Lib,
    # and we additionally link against zlib (compression library).
    link_directories(${BUILD_DIR}/Lib)
    target_link_libraries(${PROJECT_NAME} z)
endif()
# ==========================================================================================================================================