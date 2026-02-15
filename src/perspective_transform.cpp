//
// Created by wjr on 2026/2/10.
//
#include "image_processor.h"

int main() {
    //cv::Mat3b image=vision::read_image_from_source("../images/side_view.jpg");
    cv::Mat3b image = vision::read_image_from_source("../test_image/big_test3.png");
    cv::Mat1b thresh_image = vision::image_preprocessing(image, cv::Size(1280, 720), 13, 11,1, 3);
    std::vector<std::vector<cv::Point> > contours = vision::get_inner_rectangle(thresh_image, 60000, 0.7, 0.95);
    // if (contours.empty()) {
    //     continue;
    // } //用视频时将这个注释打开
    cv::drawContours(image, contours, -1, cv::Scalar(0, 255, 0), 2);

    std::vector<cv::Point2f> pre_center_points=vision::perspective_transform_get_center_point(image,contours,cv::Size(400,282));

    cv::circle(image, pre_center_points[0], 5, cv::Scalar(0, 255, 0), -1);
    cv::putText(image, "center", pre_center_points[0], cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0, 255, 0), 1);

    cv::namedWindow("output", cv::WINDOW_NORMAL);
    cv::imshow("output", image);
    cv::waitKey(0);
    }