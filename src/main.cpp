//
// Created by wjr on 2026/2/10.
//
#include "image_processor.h"

int main() {
    cv::VideoCapture cap("../images/video.mp4");

    while (true) {
        cv::Mat3b image=vision::read_video_from_source(cap);
        cv::Mat1b thresh_image=vision::image_preprocessing(image,cv::Size(1280,720),13,11,1,3);

        std::vector<std::vector<cv::Point>> inner_rectangle_contour=vision::get_inner_rectangle(thresh_image,60000,0.7,0.95);

        std::vector<cv::Vec3f> circle=vision::get_circle(thresh_image);

        cv::drawContours(image,inner_rectangle_contour,-1,cv::Scalar(0,255,0),2);
        cv::namedWindow("output",cv::WINDOW_GUI_NORMAL);
        cv::imshow("output",image);
        cv::waitKey(20);
    }
}