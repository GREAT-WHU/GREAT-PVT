# ============================================================================
# Purpose: Define and build the main executable for this project, including
#          its source files, header files, and linking dependencies.
# Assumptions:
#   - The variable ${pvt} is expected to be defined BEFORE this block (e.g., via
#     the command line -Dpvt=... or set earlier in the top‑level CMakeLists.txt).
#   - Third_Eigen_ROOT, LibGnutSrc, LibGREATSrc, BUILD_DIR, LibGnut, and
#     LibGREAT are already defined (from previous sections).
#   - The two libraries LibGnut and LibGREAT are built as subprojects via
#     add_subdirectory() and their targets exist.
# Structure:
#   1. Set minimum CMake version and project name.
#   2. Collect source and header files using globbing.
#   3. Organise files into IDE source groups (Visual Studio / Xcode).
#   4. Set include directories for Eigen and the two libraries.
#   5. Create the executable target.
#   6. Configure library search paths (platform‑specific).
#   7. Link the executable against the two libraries.
#   8. Declare build‑order dependencies (ensures libraries are built first).
# ============================================================================

# ---- 1. Minimum CMake version and project ----
# Require at least CMake 3.5 (needed for modern features)
cmake_minimum_required(VERSION 3.5)

# Project name is taken from the variable ${pvt} (must be defined earlier)
project(${pvt})

# ---- 2. Collect source and header files ----
# Use GLOB to automatically pick up all .h, .hpp, .cpp files in the current
# directory. Note: GLOB is not recommended for production builds because it
# may miss newly added files; explicitly listing sources is more robust.
file(GLOB header_files     *.h *.hpp)
file(GLOB source_files     *.cpp)

# ---- 3. Organise files in IDE ----
# Group CMakeLists.txt, headers, and sources into separate folders in Visual
# Studio / Xcode for better project navigation.
source_group("CMake Files" FILES CMakeLists.txt)
source_group("Header Files" FILES header_files)
source_group("Soruce Files" FILES source_files)   # Typo: "Soruce" → "Source" (kept as is)

# ---- 4. Include directories ----
# List of directories to search for header files during compilation.
set(include_path
    ${Third_Eigen_ROOT}      # Eigen header‑only library (must be defined)
    ${LibGnutSrc}            # Source directory of LibGnut
    ${LibGREATSrc})          # Source directory of LibGREAT
include_directories(${include_path})

# ---- 5. Create the executable ----
# Build the executable from the collected source and header files.
# Header files are not strictly needed for compilation but help IDEs.
add_executable(${PROJECT_NAME} ${header_files} ${source_files})

# ---- 6. Library search paths (linker) ----
# Tell the linker where to find the libraries during final linking.
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    # On Windows, MSVC creates separate output directories per build config.
    set(link_path 
        ${BUILD_DIR}/Lib/Debug
        ${BUILD_DIR}/Lib/Release
        ${BUILD_DIR}/Lib/RelWithDebInfo
        ${BUILD_DIR}/Lib/MinSizeRel)
    link_directories(${link_path})                 
else()
    # On Linux/macOS, libraries are typically placed in a single Lib folder
    # (or the build type is handled via the library filename suffix).
    set(link_path
        ${BUILD_DIR}/Lib)
    link_directories(${link_path})                 
endif()

# ---- 7. Link libraries to the executable ----
# List of library targets (from the subprojects) to link against.
set(lib_list
    ${LibGnut}
    ${LibGREAT})
target_link_libraries(${PROJECT_NAME} ${lib_list})

# ---- 8. Build order dependencies ----
# Ensure that the libraries are built before the executable (in case of
# parallel builds). This is generally redundant when using target_link_libraries
# because CMake automatically adds dependencies, but it's kept here for clarity.
add_dependencies(${PROJECT_NAME} ${lib_list})