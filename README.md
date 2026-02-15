# Electric Competition (2026)

## Overview

本项目主要用于存放本小组电赛相关的实验算法，主要基于OpenCV (c++) ，ros_control（c++），stm32，msp系列单片机实现。

## Environment

- Vision : OpenCV 4.14.0 / C++17 / CMake 3.6
- Embedded : Keil uVision5 / STM32CubeIDE
- Mechanical : SolidWorks 2024
- ROS : Noetic

## Structure

### Vision :  

#### 概览：

该模块当前已实现两个功能，分别是靶纸的目标识别以及三子棋的目标识别，实现以上物体在动态环境下 （旋转，倾斜，光照波动，画质降低）的稳定定位与分类。

#### 关键细节：

图像预处理 ：根据不同的需求，我们对灰度图，色相图，饱和度图，亮度图，以及不同的RGB通道图应用了自适应阈值化技术，并在此基础上进行了开闭运算以保证轮廓的连续性。

边缘检测：对于大部分需求，我们使用canny算子进行边缘检测，获得了一个低噪声，高鲁棒的边缘。

透视变换：为了应对赛场上的复杂环境，我们对识别出来的方框做了透视变换之后再找对应的分位点，使其得以应对倾斜画面进行稳定定位。

四点排序逻辑：针对直接使用无序轮廓点导致的透视变换失败情况，我们有必要将轮廓点进行排序，我在这里根据几何重心和极坐标角度对`findContours`得到的四个顶点进行重新排序，解决了摄像头旋转导致的坐标映射翻转问题，无论如何摆放，始终确保排序结果为 【左上，左下，右下，右上】。

鲁棒性特征提取：针对于25赛题中的靶纸，我们注意到其为一个嵌套矩形结构，我们通过查找两个矩形，并通过计算某个轮廓的点是否全部位于外部轮廓的内部来确定父子关系，但是同一画面中可能会出现多个具有这种父子关系的轮廓，针对此现象，我们通过判断轮廓的面积以及面积比，对获得的所有嵌套轮廓做进一步的过滤，最终实现准确定位目标。

棋子目标的获取：针对于24赛题，我们提取圆形棋子实际上就是提取圆形物体，为了提升其对于质量较差（没那么圆）的棋子的识别成功率以及降低误识别的概率，我们先将hsv通道中的饱和度通道提取出来，将低饱和度的棋子与高饱和度的背景和棋盘区别出来并获取轮廓，然后使用圆度公式来过滤和筛选轮廓，最后使用圆形拟合轮廓精确地获取棋子的坐标。

棋子类型判断：针对于24赛题，我们对每个获取到的棋子目标计算其灰度的平均值，并计算所有的棋子的灰度的平均值的平均值，获得一个参考阈值`thresh`，利用该阈值确定棋子的类型。

### Embedded :

### Mechanical :

### ROS :



## How To Use

### Vision :

使用`cv::Mat3b read_image_from_source(std::string path)`可方便地从您给定的路径中读取rgb图像，例如：

```cpp
cv::Mat3b image = vision::read_image_from_source("../test_image/big_test3.png");
```

读取视频时使用请手动创建`cv::VideoCapture`对象，并将其传入`cv::Mat3b read_video_from_source(cv::VideoCapture &cap)`中，例如：

```cpp
cv::VideoCapture cap("../images/video.mp4");

    while (true) {
        cv::Mat3b    
        image = vision::read_video_from_source(cap);
        }
```

> [!NOTE]
>
> 当您使用视频时，请务必将其放于循环之中

使用`cv::Mat1b image_preprocessing(cv::Mat3b &image,cv::Size image_size,int gaussian_blur_size,int threshold_size,double threshold_c,int closing_size)` 可方便地对图像做一些前置处理

使用`std::vector<std::vector<cv::Point>> get_inner_rectangle(cv::Mat1b thresh_image,int contour_area_threshold,double min_contour_area_ratio,double max_contour_area_ratio)` 可获取嵌套矩形中的内轮廓

### Embedded :

### Mechanical :

### ROS :



