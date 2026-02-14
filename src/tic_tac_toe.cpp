//
// Created by wjr on 2026/2/11.
//
#include "algorithm.h"
#include "image_processor.h"

int main() {
    cv::VideoCapture capture("../images/chess_play.mp4");
    while (true){
        cv::Mat3b chessboard_image;
        capture>>chessboard_image;
        cv::Mat hsv_image;
        cv::cvtColor(chessboard_image,hsv_image,cv::COLOR_BGR2HSV);
        std::vector<cv::Mat> channels;
        cv::split(chessboard_image,channels);
        cv::Mat1b red_channel=channels[2];
        cv::Mat1b blue_channel=channels[0];

        std::vector<cv::Mat> hsv_channels;
        cv::split(hsv_image,hsv_channels);

        //----------以下是框的算法------------

        cv::Mat1b chessboard_thresh_image;
        cv::medianBlur(red_channel,red_channel,15); //对方框做中值滤波
        cv::GaussianBlur(red_channel,red_channel,cv::Size(9,9),0);

        cv::adaptiveThreshold(channels[2],chessboard_thresh_image,255,cv::ADAPTIVE_THRESH_GAUSSIAN_C,cv::THRESH_BINARY,5,0);

        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;

        cv::findContours(chessboard_thresh_image,contours,hierarchy,cv::RETR_LIST,cv::CHAIN_APPROX_NONE);

        std::vector<std::vector<cv::Point>> filter_contours;
        for (int i=0;i<contours.size();i++) {
            if (cv::contourArea(contours[i])>10000) {
                filter_contours.push_back(contours[i]);
            }
        }
        std::cout<<"filter contours size is : "<<filter_contours.size()<<std::endl;


        std::vector<std::vector<cv::Point>> approx_contours;

        for (int i=0;i<filter_contours.size();i++) {
            std::vector<cv::Point> temp_approx;
            cv::approxPolyDP(filter_contours[i],temp_approx,30,true);
            if (temp_approx.size()==4) {
                approx_contours.push_back(temp_approx);
            }
        }

        std::cout<<"approx contours size is : "<<approx_contours.size()<<std::endl;

        if (approx_contours.empty()) {
            cv::namedWindow("output",cv::WINDOW_NORMAL);
            cv::imshow("output",chessboard_image);
            cv::waitKey(20);
            continue;
        }

        std::vector<cv::Point2f> dst_pts = { //开始执行透视变换
            {0, 0}, {0, static_cast<float>(400)},
            {
                static_cast<float>(400),
                static_cast<float>(400)
            },
            {static_cast<float>(400), 0}
        }; //变换后的四个点坐标

        std::vector<int> polar_ordination_index{0,1,2,3};
        std::vector<cv::Point2f> corners;
        for (int i=0;i<approx_contours[0].size();i++) {
            cv::Point2f temp=cv::Point2f(approx_contours[0][i].x,approx_contours[0][i].y);
            corners.push_back(temp);
        }

        algorithm::four_points_sorting(corners);

        std::vector<cv::Point2f> src_pts = { //变换前的四个点坐标
            {static_cast<float>(corners[0].x), static_cast<float>(corners[0].y)},
            {static_cast<float>(corners[1].x), static_cast<float>(corners[1].y)},
            {static_cast<float>(corners[2].x), static_cast<float>(corners[2].y)},
            {static_cast<float>(corners[3].x), static_cast<float>(corners[3].y)}
        };
        std::cout<<approx_contours[0]<<std::endl;
        cv::Mat H = cv::getPerspectiveTransform(src_pts,dst_pts); //获得透视变换矩阵
        cv::Mat H_inv = H.inv();

        cv::Mat3b output_image;
        cv::warpPerspective(chessboard_image,output_image,H,cv::Size(400,400));

        cv::Point2f center_point=cv::Point(output_image.cols/2,output_image.rows/2);

        std::vector<cv::Point2f> points;

        for (int i=1;i<=5;i+=2) {
            for (int j=1;j<=5;j+=2) {
                points.push_back(cv::Point(output_image.rows/6*i,output_image.cols/6*j));
            }
        }

        std::vector<cv::Point2f> pre_points;

        cv::perspectiveTransform(points,pre_points,H_inv);

        for (int i=0;i<9;i++) {
            std::string text="Point"+std::to_string(i+1);
            cv::putText(chessboard_image,text,pre_points[i],cv::FONT_HERSHEY_COMPLEX,1,cv::Scalar(0,255,255),2);
            cv::circle(chessboard_image,pre_points[i],5,cv::Scalar(0,255,255),-1);
        }

        cv::Mat contours_image=cv::Mat::zeros(red_channel.rows,red_channel.cols,CV_8UC3);
        cv::drawContours(chessboard_image,approx_contours,-1,cv::Scalar(0,255,255),2);

        //---------------以下是找棋子的算法---------------

        cv::medianBlur(hsv_channels[1],hsv_channels[1],19);
        cv::GaussianBlur(hsv_channels[1],hsv_channels[1],cv::Size(7,7),0);
        cv::Mat1b thresh;
        cv::adaptiveThreshold(hsv_channels[1],thresh,255,cv::ADAPTIVE_THRESH_GAUSSIAN_C,cv::THRESH_BINARY,15,-2);

        cv::Mat kernel=cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(5,5));
        cv::dilate(thresh,thresh,kernel);
        cv::erode(thresh,thresh,kernel);

        cv::Mat kernel1=cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(3,3));
        cv::erode(thresh,thresh,kernel1);
        cv::dilate(thresh,thresh,kernel1);

        cv::Mat1b edge_image;
        cv::Canny(thresh,edge_image,50,150);

        std::vector<std::vector<cv::Point>> piece_contours;
        std::vector<cv::Vec4i> piece_hierarchy;
        cv::findContours(edge_image,piece_contours,piece_hierarchy,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_NONE);

        std::vector<std::vector<cv::Point>> piece_filter_contours;
        for (auto &p : piece_contours) {
            std::cout<<cv::contourArea(p)<<std::endl;
            if (cv::contourArea(p) > 4000 && cv::contourArea(p) < 30000 && (4 * M_PI * cv::contourArea(p)) / std::pow(cv::arcLength(p,true),2) > 0.84) {
                piece_filter_contours.push_back(p);
            }
        }

        std::vector<cv::Mat> piece;
        std::vector<cv::Scalar> mean_vals;
        std::vector<std::vector<cv::Point>> write_piece_contours;
        std::vector<std::vector<cv::Point>> black_piece_contours;

        for (int i=0;i<piece_filter_contours.size();i++) {
            cv::Mat mask = cv::Mat::zeros(chessboard_image.rows,chessboard_image.cols,CV_8UC1);
            cv::drawContours(mask,piece_filter_contours,i,cv::Scalar(255),-1);
            cv::Scalar mean_val = cv::mean(hsv_channels[2],mask);
            mean_vals.push_back(mean_val);
            std::cout<<mean_val<<std::endl;
        }
        double mean_mean_val = 0;
        for (auto &p : mean_vals) {
            mean_mean_val+=p[0];
        }
        mean_mean_val /= mean_vals.size();

        std::vector<cv::Point2f> black_piece_circles_center;
        std::vector<cv::Point2f> write_piece_circles_center;
        std::vector<float> black_piece_circles_radius;
        std::vector<float> write_piece_circles_radius;

        for (int i=0;i<mean_vals.size();i++) {
            if (mean_vals[i][0] < mean_mean_val-5 && mean_vals[i][0] < 160) {
                black_piece_contours.push_back(piece_filter_contours[i]);
                cv::Point2f circle_center;
                float radius;
                cv::minEnclosingCircle(piece_filter_contours[i],circle_center,radius);
                black_piece_circles_center.push_back(circle_center);
                black_piece_circles_radius.push_back(radius);
            }

            if (mean_vals[i][0] > mean_mean_val+5 && mean_vals[i][0] > 140) {
                write_piece_contours.push_back(piece_filter_contours[i]);
                cv::Point2f circle_center;
                float radius;
                cv::minEnclosingCircle(piece_filter_contours[i],circle_center,radius);
                write_piece_circles_center.push_back(circle_center);
                write_piece_circles_radius.push_back(radius);
            }
        }

        for (int i=0;i<write_piece_circles_center.size();i++) {
            cv::circle(chessboard_image,write_piece_circles_center[i],write_piece_circles_radius[i],cv::Scalar(0,0,255),2);
            cv::circle(chessboard_image,write_piece_circles_center[i],5,cv::Scalar(0,0,255),-1);
        }

        for (int i=0;i<black_piece_circles_center.size();i++) {
            cv::circle(chessboard_image,black_piece_circles_center[i],black_piece_circles_radius[i],cv::Scalar(0,255,0),2);
            cv::circle(chessboard_image,black_piece_circles_center[i],5,cv::Scalar(0,255,0),-1);
        }

        cv::namedWindow("output",cv::WINDOW_NORMAL);
        cv::imshow("output",thresh);
        cv::namedWindow("output",cv::WINDOW_NORMAL);
        cv::imshow("output",chessboard_image);
        cv::waitKey(20);
    }
}