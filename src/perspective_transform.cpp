//
// Created by wjr on 2026/2/10.
//
#include "image_processor.h"

int main() {
    cv::VideoCapture cap("../images/interference.mp4");

    while (true){
        //cv::Mat3b image=vision::read_image_from_source("../images/side_view.jpg");
        cv::Mat3b image=vision::read_video_from_source(cap);
        cv::Mat1b thresh_image=vision::image_preprocessing(image,cv::Size(1280,720),13,11,3);
        std::vector<std::vector<cv::Point>> contours=vision::get_inner_rectangle(thresh_image,60000,0.7,0.95);
        if (contours.empty()) {
            continue;
        }
        cv::drawContours(image,contours,-1,cv::Scalar(0,255,0),2);

        std::vector<cv::Point2f> dst_pts={{0,0},{0,282},{400,282},{400,0}}; //变换后的四个点坐标

        std::vector<int> position_index {0,1,2,3};
        for (int i=0;i<3;i++) { //将坐标之和从小到大排列
            for (int j=0;j<3;j++) {
                if (contours[0][position_index[j]].x+contours[0][position_index[j]].y > contours[0][position_index[j+1]].x+contours[0][position_index[j+1]].y) {
                    int temp=position_index[j];
                    position_index[j]=position_index[j+1];
                    position_index[j+1]=temp;
                }
            }
        }
        if (contours[0][position_index[1]].x > contours[0][position_index[3]].x) {
            int temp=position_index[1];
            position_index[1]=position_index[3];
            position_index[3]=temp;
        }


        std::vector<cv::Point2f> src_pts = { //变换前的四个点坐标
            {static_cast<float>(contours[0][position_index[0]].x), static_cast<float>(contours[0][position_index[0]].y)},
            {static_cast<float>(contours[0][position_index[1]].x), static_cast<float>(contours[0][position_index[1]].y)},
            {static_cast<float>(contours[0][position_index[3]].x), static_cast<float>(contours[0][position_index[3]].y)},
            {static_cast<float>(contours[0][position_index[2]].x), static_cast<float>(contours[0][position_index[2]].y)}
        };
        std::cout<<contours[0]<<std::endl;
        cv::Mat H = cv::getPerspectiveTransform(src_pts,dst_pts); //获得透视变换矩阵
        cv::Mat H_inv = H.inv();

        cv::Mat3b output_image;
        cv::warpPerspective(image,output_image,H,cv::Size(400,300)); //执行变换

        cv::Point2f center_point=cv::Point(output_image.cols/2,output_image.rows/2);
        std::vector<cv::Point2f> pre_center_points;
        std::vector<cv::Point2f> center_points;
        center_points.push_back(center_point);

        cv::perspectiveTransform(center_points,pre_center_points,H_inv);

        std::cout<<pre_center_points<<std::endl;

        cv::circle(image,pre_center_points[0],5,cv::Scalar(0,255,0),-1);
        cv::circle(output_image,center_point,5,cv::Scalar(0,255,0),-1);
        cv::putText(output_image,"center",center_point,cv::FONT_HERSHEY_PLAIN,2,cv::Scalar(0,255,0),1);
        cv::putText(image,"center",pre_center_points[0],cv::FONT_HERSHEY_PLAIN,2,cv::Scalar(0,255,0),1);

        cv::namedWindow("output",cv::WINDOW_NORMAL);
        cv::namedWindow("perspective",cv::WINDOW_NORMAL);
        cv::imshow("output",image);
        cv::imshow("perspective",output_image);
        cv::waitKey(50);
    }
}