#Minimum requirement of CMake version : 3.5
cmake_minimum_required(VERSION 3.5)

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

# ============================================================================
# Purpose: Detect C++11 and C++0x support in the current C++ compiler.
# Method:  Use CMake's built‑in CheckCXXCompilerFlag module to test the
#          compiler flags -std=c++11 and -std=c++0x.
# Results:
#   - COMPILER_SUPPORTS_CXX11 : boolean (TRUE if C++11 is supported)
#   - COMPILER_SUPPORTS_CXX0X : boolean (TRUE if C++0x is supported)
# Note:    These variables can then be used to conditionally set the C++ standard
#          and compiler flags (e.g., in a subsequent if/elseif block).
# ============================================================================

# Include the module that provides the CHECK_CXX_COMPILER_FLAG macro
include(CheckCXXCompilerFlag)

# Check if the compiler accepts the -std=c++11 flag
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)

# Check if the compiler accepts the -std=c++0x flag (pre‑C++11 draft)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

# ============================================================================
# Purpose: Configure platform‑specific compilation options and set a custom
#          build directory suffix based on the target operating system.
# Supported Platforms: Linux, macOS (Darwin), and Windows (MSVC).
# Notes:
#   - On Linux, we enable high optimisation (-O3), enable warnings (-Wall),
#     then suppress them (-w) – note that -w appears twice (the second
#     one overrides the first, effectively disabling warnings).
#   - On macOS, we only add the pthread flag (usually needed for threading).
#   - On Windows, we enable support for large object files (/bigobj),
#     parallel compilation with 4 processes (/MP4), disable warnings (/w),
#     and enforce UTF‑8 encoding (/utf‑8).
#   - BUILD_DIR is set to a platform‑specific subdirectory under the binary
#     directory, allowing separate output folders per platform.
# ============================================================================

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    # --- Linux specific settings ---
    # High optimisation level for performance
    add_compile_options(-O3)
    # 64‑bit architecture flag
    add_compile_options(-m64)
    # Link against zlib compression library (though -lz is a linker flag,
    # add_compile_options works for linker flags in some generators, but
    # using target_link_libraries is generally preferred; kept as is)
    add_compile_options(-lz)
    # Link against the C++ standard library (GNU libstdc++)
    add_compile_options(-lstdc++)
    # Enable most common warnings
    add_compile_options(-Wall)
    # Suppress all warnings (overrides -Wall)
    add_compile_options(-w)

    # Set a custom build output directory suffixed with _Linux
    SET(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}_Linux)

    # Embed the RPATH into the executable so that the dynamic linker can find
    # shared libraries relative to the executable's location
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    # Set the RPATH to $ORIGIN/../Lib (relative to the executable) for installed
    # binaries
    SET(CMAKE_INSTALL_RPATH "\${ORIGIN}/../Lib")

elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    # --- macOS (Darwin) specific settings ---
    # Set build output directory suffixed with _Darwin
    SET(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}_Darwin)
    # Add pthread flag (required for threading support on macOS)
    add_compile_options(-pthread)

elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    # --- Windows (MSVC) specific settings ---
    # Set build output directory suffixed with _Windows
    SET(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}_Windows)

    # /bigobj: Allow object files to exceed the default 64k sections limit
    add_compile_options(/bigobj)
    # /MP4: Use up to 4 parallel processes for compilation (speeds up builds)
    add_compile_options(/MP4)
    # /w: Disable all compiler warnings
    add_compile_options(/w)
    # /utf‑8: Set source and execution character sets to UTF‑8
    add_compile_options(/utf-8)
endif()


# ============================================================================
# Purpose: Replace the default /W1 warning level with /w (disable all warnings)
#          for the MSVC compiler.
# Method:  Directly substitute "/W1" with "/w" in CMAKE_C_FLAGS and
#          CMAKE_CXX_FLAGS.
# Note:    This script is intended for MSVC only; other compilers will see a
#          warning message.
# ============================================================================

if(MSVC)
    # Replace "/W1" with "/w" in C compilation flags
    string(REPLACE "/W1" "/w" CMAKE_C_FLAGS " ${CMAKE_C_FLAGS} ")
    # Replace "/W1" with "/w" in C++ compilation flags
    string(REPLACE "/W1" "/w" CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} ")
    # No need for extra add_compile_options(/w) because the flag is already in place
else()
    message(WARNING "This script is intended for MSVC compiler.")
endif()


# ============================================================================
# Purpose: Enable C++11 (or fallback to C++0x) support based on compiler
#          capability.
# Assumptions: The variables COMPILER_SUPPORTS_CXX11 and COMPILER_SUPPORTS_CXX0X
#              have been defined earlier (e.g., via check_cxx_compiler_flag or
#              a toolchain-specific check).
# Method:
#   - If C++11 is available, set CMAKE_CXX_STANDARD to 11, make it required,
#     and add the standard flag and -pthread to CMAKE_CXX_FLAGS.
#   - Else if C++0x (the draft version) is available, add the corresponding
#     flags instead.
#   - Otherwise, print a status message suggesting that a different compiler
#     is needed.
# Note: This snippet assumes a GCC/Clang style compiler (uses -std=c++11 and
#       -pthread). For MSVC, other flags would be required.
# ============================================================================

if(COMPILER_SUPPORTS_CXX11)
    # Explicitly set the C++ standard to C++11
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)          # Fail if compiler does not support C++11
    # Append the required compile flags (standard and threading)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
elseif(COMPILER_SUPPORTS_CXX0X)
    # Fallback to the older C++0x standard (pre‑C++11 draft)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -pthread")
else()
    # No suitable C++11/0x support found; inform the user
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# ============================================================================
# Purpose: Set the build postfix (suffix) for generated binaries (executables
#          and libraries) based on the current build configuration.
# Method:  Match CMAKE_BUILD_TYPE against known configurations and assign
#          the corresponding default postfix variable provided by CMake.
#          Supported configurations: Release, Debug, RelWithDebInfo, MinSizeRel.
# Notes:
#   - CMake provides built‑in variables like CMAKE_DEBUG_POSTFIX (default "d"),
#     CMAKE_RELEASE_POSTFIX (default empty), CMAKE_RELWITHDEBINFO_POSTFIX,
#     and CMAKE_MINSIZEREL_POSTFIX. These are used directly.
#   - The result is stored in CMAKE_BUILD_POSTFIX, which can be consumed by
#     add_library() or add_executable() via the ${CMAKE_BUILD_POSTFIX}
#     variable in target names or by setting the target property
#     <CONFIG>_POSTFIX.
#   - If CMAKE_BUILD_TYPE is not set or unrecognised, an empty postfix is used.
# ============================================================================

if(CMAKE_BUILD_TYPE MATCHES "Release")
    # Use the standard postfix for Release builds (usually empty)
    set(CMAKE_BUILD_POSTFIX "${CMAKE_RELEASE_POSTFIX}")
elseif(CMAKE_BUILD_TYPE MATCHES "Debug")
    # Use the standard postfix for Debug builds (typically "d")
    set(CMAKE_BUILD_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
elseif(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    # Use the postfix for Release with Debug Info builds
    set(CMAKE_BUILD_POSTFIX "${CMAKE_RELWITHDEBINFO_POSTFIX}")
elseif(CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
    # Use the postfix for Minimum Size Release builds
    set(CMAKE_BUILD_POSTFIX "${CMAKE_MINSIZEREL_POSTFIX}")
else()
    # Fallback: no recognised build type → no postfix
    set(CMAKE_BUILD_POSTFIX "")
endif()

# ============================================================================
# Purpose: Configure the root source directory, output directories for
#          executables, libraries, and archives, as well as build‑type‑specific
#          postfixes for generated binaries.
# Assumptions:
#   - PROJECT_SOURCE_DIR is the top‑level source directory (set by CMake after
#     the project() command).
#   - BUILD_DIR is a user‑defined variable (e.g., set earlier based on the
#     platform) that points to a platform‑specific build output folder.
# Notes:
#   - EXECUTABLE_OUTPUT_PATH is set as a fallback, but the modern CMake
#     properties (CMAKE_RUNTIME_OUTPUT_DIRECTORY, etc.) are also set for
#     better control.
#   - The postfix variables (CMAKE_*_POSTFIX) are used by CMake to append
#     suffixes to output files when add_executable() or add_library() is called
#     with the corresponding configuration.
# ============================================================================

# ROOT points to the project source root (where the top‑level CMakeLists.txt is)
set(ROOT ${PROJECT_SOURCE_DIR})

# Legacy output path variable (kept for compatibility with older projects)
set(EXECUTABLE_OUTPUT_PATH ${BUILD_DIR}/Bin)

# Modern output directories for runtime executables (binaries)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR}/Bin)

# Output directory for shared libraries (.so, .dylib, .dll)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR}/Lib)

# Output directory for static libraries (.a, .lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR}/Lib)

# Postfix appended to Debug build outputs (typically "d" for debug)
set(CMAKE_DEBUG_POSTFIX "d")

# Postfix for Release builds (usually empty)
set(CMAKE_RELEASE_POSTFIX "")

# Postfix for Release with Debug Info builds (often "rd")
set(CMAKE_RELWITHDEBINFO_POSTFIX "rd")

# Postfix for Minimum Size Release builds (commonly "s")
set(CMAKE_MINSIZEREL_POSTFIX "s")

# ============================================================================
# Purpose: Display important CMake configuration and environment variables
#          to help debug the build process.
# Variables printed:
#   - CMAKE_SYSTEM          : Full system description (e.g., "Linux-5.4.0")
#   - CMAKE_SYSTEM_NAME     : Operating system name (e.g., "Linux", "Windows")
#   - CMAKE_SYSTEM_VERSION  : OS version string
#   - CMAKE_C_COMPILER      : Path to the C compiler
#   - CMAKE_CXX_COMPILER    : Path to the C++ compiler
#   - BUILD_DIR             : User‑defined build output directory (if set)
#   - ROOT                  : User‑defined project source root (if set)
# Usage: Place this block after all relevant variables (BUILD_DIR, ROOT, etc.)
#        have been defined to get accurate outputs.
# Note: These messages appear during CMake configuration (not at build time).
# ============================================================================

# Output messages for debugging CMake configuration
message(STATUS "operation system is : ${CMAKE_SYSTEM}")
message(STATUS "current platform is : ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMake version    is : ${CMAKE_SYSTEM_VERSION}")
message(STATUS "C compiler       is : ${CMAKE_C_COMPILER}")
message(STATUS "C++ compiler     is : ${CMAKE_CXX_COMPILER}")
message(STATUS "Build directory  is : ${BUILD_DIR}")
message(STATUS "The program main directory is : ${ROOT}")


# ============================================================================
# Purpose: Define a list of library names (sub‑projects) to be built,
#          optionally print them, and enable folder grouping in IDEs.
# Details:
#   - file_fold : List of library names (each corresponds to a subdirectory
#                 with its own CMakeLists.txt that defines a library target).
#   - PrintFile : Boolean flag controlling whether to print the list (currently
#                 set to FALSE, so the list is not printed repeatedly).
#   - USE_FOLDERS: Enable source folder grouping in Visual Studio / Xcode
#                  to organise targets into folders.
# Usage: Place this block after project() and after defining the subdirectories
#        (if any), but before adding subdirectories that define these targets.
# ============================================================================

# List of library (sub‑project) names to be built
set(file_fold 
    spdlog gexport gio gutils gcoders gmodels gall gdata gset pugixml gproc gprod gambfix newmat)

# Print the library list (useful for debugging); output appears during configuration
message(${file_fold})

# Flag to enable/disable printing of individual file lists (currently disabled)
set(PrintFile FALSE)

# Enable folder grouping in IDEs that support it (e.g., Visual Studio, Xcode)
# This allows targets to be organised into folders within the IDE's project view
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ============================================================================
# Purpose: Locate the Eigen third‑party library (header‑only) for use in the
#          project.
# Method:
#   - On Windows: Use find_path() to search for the Eigen root directory,
#     trying a user‑provided variable or environment variable.
#   - On non‑Windows (Linux/macOS): Default to a fixed path relative to ROOT
#     (${ROOT}/third-party/Eigen) unless Third_Eigen_ROOT is already defined.
# Notes:
#   - Eigen is a header‑only library, so only the include directory is needed.
#   - The variable Third_Eigen_ROOT is used later (e.g., in include_directories
#     or target_include_directories) to point to the Eigen source folder.
#   - The Windows path uses find_path() to be more flexible; the Linux path
#     assumes a specific directory structure.
# ============================================================================

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    # --- Windows specific: search for Eigen using find_path ---
    # If Third_Eigen_ROOT is not already defined (by user or cache),
    # attempt to locate it automatically.
    if (NOT DEFINED Third_Eigen_ROOT)
        # HINTS: use either the current variable value or the environment
        # variable Third_Eigen_ROOT (if set) as a starting point.
        find_path(Third_Eigen_ROOT       HINTS "${Third_Eigen_ROOT}"        "$ENV{Third_Eigen_ROOT}            ")
    endif()
    # Note: After this, Third_Eigen_ROOT will be set to the found path,
    #       or remain NOTFOUND if not found.

else()
    # --- Linux / macOS (non‑Windows) specific: use a default path ---
    # If Third_Eigen_ROOT is not defined, set it to a fixed location
    # relative to the project source root (ROOT).
    if (NOT DEFINED Third_Eigen_ROOT)
        set(Third_Eigen_ROOT       ${ROOT}/third-party/Eigen)
    endif()
    # This assumes that Eigen is checked out or placed at that exact location.
endif()

# After this block, Third_Eigen_ROOT should be usable for include_directories.

# ============================================================================
# Purpose: Add two internal library subprojects (LibGnut and LibGREAT) to the
#          build and organise them into an IDE folder named "LIB".
# Details:
#   - Each library is located in its own subdirectory under the project root.
#   - The binary output for each subproject is placed under BUILD_DIR/<LibName>.
#   - The targets are assigned to the FOLDER property "LIB" for better
#     organisation in Visual Studio / Xcode (requires USE_FOLDERS ON).
# Dependencies: PROJECT_SOURCE_DIR and BUILD_DIR must be defined beforehand.
# ============================================================================

# Define library names (used as target names and directory names)
set(LibGnut          LibGnut)
set(LibGREAT         LibGREAT)

# Full source paths for each library
set(LibGnutSrc          ${PROJECT_SOURCE_DIR}/${LibGnut})
set(LibGREATSrc         ${PROJECT_SOURCE_DIR}/${LibGREAT})
set(Third_SOFA_ROOT     ${PROJECT_SOURCE_DIR}/third-party/SOFA)

# Add subdirectories; the second argument specifies the binary output directory
# for each subproject (under BUILD_DIR)
add_subdirectory(${Third_SOFA_ROOT}    ${BUILD_DIR}/SOFA)
add_subdirectory(${LibGnutSrc}       ${BUILD_DIR}/${LibGnut})
add_subdirectory(${LibGREATSrc}      ${BUILD_DIR}/${LibGREAT})

# Assign both library targets to the "LIB" folder in the IDE project view
SET_PROPERTY(TARGET LibSOFA        PROPERTY FOLDER "LIB")
SET_PROPERTY(TARGET ${LibGnut}       PROPERTY FOLDER "LIB")
SET_PROPERTY(TARGET ${LibGREAT}      PROPERTY FOLDER "LIB")

# ============================================================================
# Purpose: Display platform‑specific status messages and, on Windows,
#          enable source folder grouping in IDEs (e.g., Visual Studio).
# Details:
#   - On Windows: print a header message and ensure USE_FOLDERS is ON.
#   - On non‑Windows (Linux, macOS, etc.): print a different header message.
# Note: USE_FOLDERS may already have been set earlier in the script; this
#       re‑asserts it for Windows to guarantee IDE folder organisation.
# ============================================================================

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    # --- Windows specific ---
    # Print blank line and a visual separator for CMake output clarity
    message(STATUS "                                                       ")
    message(STATUS "==================    Windows    ======================")
    # Explicitly enable folder grouping in the IDE (Visual Studio / Xcode)
    SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)    
else()
    # --- Non‑Windows platforms (Linux, macOS, etc.) ---
    # Print blank line and a different header to indicate the current platform
    message(STATUS "                                                       ")
    message(STATUS "==================    Not Windows =====================")
endif()
