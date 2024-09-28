# GREAT-PVT: 武汉大学GREAT团队精密定位导航软件（内测版）

## 概述

&emsp;&emsp;GREAT (GNSS+ REsearch, Application and Teaching) 软件由武汉大学测绘学院设计开发，是一个用于空间大地测量数据处理、精密定位和定轨以及多源融合导航的综合性软件平台。<br />
&emsp;&emsp;GREAT-PVT是GREAT软件中的一个重要模块，主要用于精密定位解算。软件中，核心计算模块使用C++语言(C++17)编写，辅助脚本模块使用Python3和C-Shell语言实现自动化数据处理。GREAT-PVT软件使用CMAKE工具进行编译管理，用户可以灵活选择GCC、Clang、MSVC等主流C++编译器。目前软件提供了Windows和Linux平台的命令行应用程序。<br />
&emsp;&emsp;GREAT-PVT由2个可移植程序库组成，分别是LibGREAT和LibGnut。LibGREAT库主要用于PPP、RTK滤波解算，包括滤波估计中涉及的数据解码、存储以及PPP、PPP-AR、RTK算法的实现，LibGnut库来源于开源GNSS软件G-nut，包括GNSS数据的解码和存储以及基本参数配置模块。<br />
&emsp;&emsp;GREAT-PVT软件主要特点包括：
1. 支持GPS、GLONASS、Galileo、BDS-2/3系统 

2. 支持多频PPP浮点解与固定解

3. 支持无电离层组合、非差非组合PPP观测值组合方式

4. 支持多频率多系统PPP浮点解与固定解
   
5. 支持双频、单双频混合RTK解算
   

&emsp;&emsp;此外，软件包还提供批处理脚本和定位结果绘图脚本，便于用户对多天数据进行解算与结果分析。

## 软件包目录结构
```shell
GREAT-PVT_<ver>	
  ./bin	                Windows和Linux可执行程序 *
  ./src	                源代码 *
    ./app	                GREAT-PVT主程序 *
    ./LibGREAT	            PPP、PPP-AR、RTK解算库 *
    ./LibGnut	            Gnut库 *
    ./third-party	        第三方库 *
  ./sample_data         算例数据 *
    ./PPPFLT_2023304	    PPP算例 *
    ./RTKFLT_2020351	    RTK算例 *
  ./util	              脚本工具 *
    ./batch_process	      PPP批处理脚本 *
    ./plot	              画图脚本 *
    ./poleut1	          地球定向参数生成程序 *
  ./doc	                文档 *
    GREAT_PPP.xml	      PPP配置文件模板 *
    GREAT_RTK.xml	      RTK配置文件模板 *
    GREAT-PVT_1.0.pdf	  GREAT-PVT用户指南 *
```

## 安装和使用

参见GREAT-PVT_1.0.pdf

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

* GREAT-PVT使用spdlog库（https://github.com/gabime/spdlog）
  Copyright(c) 2015-present, Gabi Melman & spdlog contributors.

* GREAT-PVT使用Eigen库（https://eigen.tuxfamily.org）
  Copyright (C) 2008-2011 Gael Guennebaud

* 脚本工具使用部分FAST源码(https://github.com/ChangChuntao/FAST)
Copyright (C) The GNSS Center, Wuhan University & Chinese Academy of Surveying and mapping.

## 下载地址

GitHub：https://github.com/GREAT-WHU/GREAT-PVT

欢迎加入QQ群（1009827379）参与讨论与交流。

