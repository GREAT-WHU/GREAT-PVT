# GREAT-PVT

[![GitHub stars](https://img.shields.io/github/stars/GREAT-WHU/GREAT-PVT?style=social)](https://github.com/GREAT-WHU/GREAT-PVT/stargazers)
[![License: GPL-3.0](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)

Precision positioning and navigation software from the Wuhan University GREAT
Group.

GREAT-PVT is a C++ command-line module of the GREAT (GNSS+ Research,
Application, and Teaching) platform. It provides PPP and RTK positioning
solutions for scientific, engineering, and teaching use in geodesy and
navigation.

## Features

- Multi-GNSS processing for GPS, GLONASS, Galileo, and BDS-2/3.
- Ionosphere-free and uncombined PPP observation models.
- Multi-frequency, multi-system PPP float and fixed solutions.
- Dual-frequency and mixed-frequency RTK solutions.
- Batch-processing and plotting utilities for multi-day result analysis.
- Prebuilt command-line binaries for Windows and Linux.

## Quick Start

Use the prebuilt executable if you only want to run GREAT-PVT.

### Windows

```powershell
GREAT_PVT.exe -x GREAT_PPP.xml
```

### Linux

```bash
chmod +x GREAT_PVT
GREAT_PVT -x GREAT_PPP.xml
```

The XML files in `doc/` are templates. Update their input and output paths to
match the extracted files from `sample_data/` or your own GNSS products before
running.

## Build From Source

Requirements:

- CMake 3.5+
- A C++ compiler with C++11 support
- Windows, Linux, or macOS

Build on Linux/macOS:

```bash
cmake -S src -B src/build -DCMAKE_BUILD_TYPE=Release
cmake --build src/build --config Release
```

Build on Windows with Visual Studio and CMake GUI:

1. Open CMake GUI.
2. Set `Where is the source code` to the repository `src` directory.
3. Set `Where to build the binaries` to `src/build`.
4. Click `Configure`, then select your installed Visual Studio generator.
5. Click `Generate`.
6. Click `Open Project`, then build the `Release` configuration in Visual Studio.

Build outputs are written under CMake-generated platform directories such as
`src/build_Linux` or `src/build_Windows`.

## Command Line

```text
GREAT_PVT -x <config.xml>
```

Useful options:

```text
-h, --help    Show command-line help
-x file       Read XML configuration
```

## Repository Layout

```text
bin/                  Prebuilt Windows and Linux executables/libraries
doc/                  Manuals and sample XML configurations
sample_data/          PPP and RTK sample datasets
src/                  C++ source code
  app/GREAT_PVT/      Command-line application
  LibGREAT/           PPP, PPP-AR, and RTK processing library
  LibGnut/            G-Nut-based GNSS data and utility library
  third-party/        Vendored third-party headers/libraries
util/                 Batch processing, plotting, and EOP utilities
```

## Sample Data

The repository includes compressed examples:

- `sample_data/PPPFLT_2023305.zip` for PPP
- `sample_data/RTKFLT_2020351.zip` for RTK

Extract a sample dataset, then adjust `doc/GREAT_PPP.xml` or
`doc/GREAT_RTK.xml` so the paths under `<inputs>` point to the extracted files.

## Documentation

Detailed installation, configuration, input products, and output formats are
covered in:

- `doc/GREAT-PVT_1.0.pdf`
- `doc/GREAT-PVT_manual_1.0.pdf`

## Changelog

### Version 1.3

- Fixed an error in the GLONASS satellite attitude calculation (function t_gattitude_model::_noon_turn_GLOM). Consequently, when GLONASS satellites are included in the positioning solution, the results from version 1.3 may be inconsistent with those from earlier versions.
- More smart spdlog, spdlog version update to spdlog-1.17.0.

### Version 1.2

- Added English documentation and improved related content.
- Updated sample data.
- Fixed code formatting issues.

### Version 1.1

- Fixed a Visual Studio Debug-mode crash on Windows.
- Fixed exceptions caused by missing PCO/PCV correction information in ATX.
  antenna files.
- Corrected description errors in `GREAT-PVT_1.0.pdf`.
- Fixed code formatting issues.

## Acknowledgements

GREAT-PVT is developed by the GNSS+ Research, Application and Teaching (GREAT)
Group, School of Geodesy and Geomatics, Wuhan University.

Third-party components include:

- [G-Nut Library](http://www.pecny.cz)
- [pugixml](http://pugixml.org)
- [Newmat](http://www.robertnz.net/nm_intro.htm)
- [spdlog](https://github.com/gabime/spdlog)
- [Eigen](https://eigen.tuxfamily.org)
- [FAST](https://github.com/ChangChuntao/FAST)

## License

This project is licensed under the [GNU General Public License v3.0](LICENSE).

## Links

- GitHub: <https://github.com/GREAT-WHU/GREAT-PVT>

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
