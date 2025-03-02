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

---
---

# GREAT-PVT: 武汉大学GREAT团队精密定位导航软件

## 概述

&emsp;&emsp;GREAT (GNSS+ REsearch, Application and Teaching) 软件由武汉大学测绘学院设计开发，是一个用于空间大地测量数据处理、精密定位和定轨以及多源融合导航的综合性软件平台。<br />
&emsp;&emsp;GREAT-PVT是GREAT软件中的一个重要模块，主要用于精密定位解算。软件中，核心计算模块使用C++语言(C++17)编写，辅助脚本模块使用Python3和C-Shell语言实现自动化数据处理。GREAT-PVT软件使用CMAKE工具进行编译管理，用户可以灵活选择GCC、Clang、MSVC等主流C++编译器。目前软件提供了Windows和Linux平台的命令行应用程序。<br />
&emsp;&emsp;GREAT-PVT由2个可移植程序库组成，分别是LibGREAT和LibGnut。LibGREAT库主要用于PPP、RTK滤波解算，包括滤波估计中涉及的数据解码、存储以及PPP、PPP-AR、RTK算法的实现，LibGnut库来源于开源GNSS软件G-nut，包括GNSS数据的解码和存储以及基本参数配置模块。<br />
&emsp;&emsp;GREAT-PVT软件主要特点包括：
1. 支持GPS、GLONASS、Galileo、BDS-2/3系统 

2. 支持无电离层组合、非差非组合PPP观测值组合方式

3. 支持多频率多系统PPP浮点解与固定解
   
4. 支持双频、混合频率RTK解算
   

&emsp;&emsp;此外，软件包还提供批处理脚本和定位结果绘图脚本，便于用户对多天数据进行解算与结果分析。

## 软件包目录结构
```shell
GREAT-PVT_<ver>	
  ./bin	                Windows和Linux可执行程序 *
  ./src	                源代码 *
    ./app                  GREAT-PVT主程序 *
    ./LibGREAT             PPP、PPP-AR、RTK解算库 *
    ./LibGnut              Gnut库 *
    ./third-party          第三方库 *
  ./sample_data          算例数据 *
    ./PPPFLT_2023305       PPP算例 *
    ./RTKFLT_2020351       RTK算例 *
  ./util                 脚本工具 *
    ./batch_process        PPP批处理脚本 *
    ./plot                 画图脚本 *
    ./poleut1              地球定向参数生成程序 *
  ./doc                  文档 *
    GREAT_PPP.xml          PPP配置文件模板 *
    GREAT_RTK.xml          RTK配置文件模板 *
    GREAT-PVT_<ver>.pdf      GREAT-PVT用户指南 *
```

## 安装和使用

参见GREAT-PVT_1.0.pdf

## 修改记录
### 版本1.1
1. 修复了Windows平台中VS软件Debug模式下程序运行崩溃的问题。
2. 修复了ATX天线文件中PCO、PCV改正数信息缺失导致的程序运行异常。
3. 修改了GREAT-PVT_1.0.pdf中的描述错误。
4. 其他代码格式问题的修改。

### 版本1.2
1. 增加英文版说明文档并完善相关内容。
2. 更新了算例。
3. 其他代码格式问题的修改。

## 参与贡献

开发人员：

武汉大学GREAT团队, Wuhan University.

三方库：

* GREAT-PVT使用G-Nut库(http://www.pecny.cz)
  Copyright (C) 2011-2016 GOP - Geodetic Observatory Pecny, RIGTC.
  
* GREAT-PVT使用pugixml库(http://pugixml.org)
Copyright (C) 2006-2014 Arseny Kapoulkine.

* GREAT-PVT使用Newmat库(http://www.robertnz.net/nm_intro.htm)
Copyright (C) 2008: R B Davies.

* GREAT-PVT使用spdlog库(https://github.com/gabime/spdlog)
  Copyright(c) 2015-present, Gabi Melman & spdlog contributors.

* GREAT-PVT使用Eigen库(https://eigen.tuxfamily.org)
  Copyright (C) 2008-2011 Gael Guennebaud

* 脚本工具使用部分FAST源码(https://github.com/ChangChuntao/FAST)
Copyright (C) The GNSS Center, Wuhan University & Chinese Academy of Surveying and mapping.

## 下载地址

GitHub：https://github.com/GREAT-WHU/GREAT-PVT

欢迎加入QQ群(1009827379)参与讨论与交流。
