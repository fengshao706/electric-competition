//
// Created by wjr on 2026/2/10.
//

#ifndef OPENCV_WS_TYPES_H
#define OPENCV_WS_TYPES_H

#include "opencv2/opencv.hpp"

struct ChildToParent {
    std::vector<cv::Point> child;
    std::vector<cv::Point> parent;
    int child_idx;
};

#endif //OPENCV_WS_TYPES_H