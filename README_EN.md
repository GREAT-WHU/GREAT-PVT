# GREAT-PVT: Precision Positioning and Navigation Software by Wuhan University GREAT Group

## Overview

GREAT-PVT is a key module of the **GREAT (GNSS+ Research, Application, and Teaching)** software platform, developed by the School of Geodesy and Geomatics at Wuhan University. It is designed for precise positioning solutions, including **PPP (Precise Point Positioning)** and **RTK (Real-Time Kinematic)** algorithms. The core computational modules are written in **C++** (C++17), while auxiliary scripts are implemented in **Python3** and **C-Shell** for automated data processing. The software uses **CMAKE** for compilation management, supporting mainstream C++ compilers such as **GCC**, **Clang**, and **MSVC**. Currently, GREAT-PVT provides command-line applications for both **Windows** and **Linux** platforms.

### Key Features
- Supports **GPS**, **GLONASS**, **Galileo**, and **BDS-2/3** systems.
- Supports **ionosphere-free combination** and **uncombined PPP observation models**.
- Supports **multi-frequency, multi-system PPP float and fixed solutions**.
- Supports **dual-frequency** and **mixed-frequency RTK solutions**.

The software package also includes **batch processing scripts** and **plotting tools** for analyzing positioning results over multiple days.

## Directory Structure
```
GREAT-PVT_<ver>	
  ./bin	                The executable binary APs for Windows/Linux *
  ./src	                Source programs of GREAT-PVT software *
    ./app                 Main function of GREAT-PVT *
    ./LibGREAT            Source programs of PPP and RTK Processing *
    ./LibGnut             Source programs of the G-Nut library *
    ./third-party         Third-party libraries *
  ./sample_data         Sample data for AP *
    ./PPPFLT_2023305      Sample data for PPP *
    ./RTKFLT_2020351      Sample data for RTK *
  ./util                Utilities *
    ./batch_process       Batch processing python scripts for PPP *
    ./plot                Plotting scripts *
    ./poleut1             Earth Orientation Parameters (EOP) Generation Program *
  ./doc                 Document files *
    GREAT_PPP.xml         Sample XML files for PPP *
    GREAT_RTK.xml         Sample XML files for RTK *
    GREAT-PVT_<ver>.pdf   User manual *
```

## Installation and Usage

For detailed installation and usage instructions, please refer to the **GREAT-PVT_\<ver>.pdf** document included in the `./doc` directory.

## Changelog
### Version 1.1
1. Fixed a crash issue when running the program in Debug mode with Visual Studio on Windows.
2. Resolved an issue where missing PCO and PCV correction information in ATX antenna files caused program exceptions.
3. Corrected description errors in GREAT-PVT_1.0.pdf.
4. Addressed other code formatting issues.

### Version 1.2
1. Added an English version of the documentation and improved related content.
2. Updated the sample data.
3. Fixed other code formatting issues.

## Contributing

### Developers
- **GNSS + REsearch, Application and Teaching (GREAT) Group**, School of Geodesy and Geomatics, Wuhan University.

### Third-Party Libraries
- **G-Nut Library** (http://www.pecny.cz)  
  Copyright (C) 2011-2016 GOP - Geodetic Observatory Pecny, RIGTC.
  
- **pugixml Library** (http://pugixml.org)  
  Copyright (C) 2006-2014 Arseny Kapoulkine.

- **Newmat Library** (http://www.robertnz.net/nm_intro.htm)  
  Copyright (C) 2008: R B Davies.

- **spdlog Library** (https://github.com/gabime/spdlog)  
  Copyright (c) 2015-present, Gabi Melman & spdlog contributors.

- **Eigen Library** (https://eigen.tuxfamily.org)  
  Copyright (C) 2008-2011 Gael Guennebaud.

- **FAST Source Code** (https://github.com/ChangChuntao/FAST)  
  Copyright (C) The GNSS Center, Wuhan University & Chinese Academy of Surveying and Mapping.


## Download

GitHub: https://github.com/GREAT-WHU/GREAT-PVT


