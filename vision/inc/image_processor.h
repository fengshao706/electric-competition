//
// Created by wjr on 2026/2/10.
//

#ifndef OPENCV_WS_IMAGE_PROCESSOR_H
#define OPENCV_WS_IMAGE_PROCESSOR_H

#include "opencv2/opencv.hpp"
#include "types.h"

namespace vision {
    cv::Mat3b read_image_from_source(std::string path);
    cv::Mat3b read_video_from_source(cv::VideoCapture &cap);
    cv::Mat1b image_preprocessing(cv::Mat3b &image,cv::Size image_size,int gaussian_blur_size,int threshold_size,double threshold_c,int closing_size);
    std::vector<std::vector<cv::Point>> get_inner_rectangle(cv::Mat1b thresh_image,int contour_area_threshold,double min_contour_area_ratio,double max_contour_area_ratio);
    std::vector<cv::Vec3f> get_circle(const cv::Mat1b &thresh_image);
    std::vector<cv::Point2f> perspective_transform_get_center_point(cv::Mat pre_image,std::vector<std::vector<cv::Point>> contours,cv::Size after_transform_image_size);
}

#endif //OPENCV_WS_IMAGE_PROCESSOR_H