//
// Created by wjr on 2026/2/13.
//
#include "image_processor.h"

int main() {
    cv::VideoCapture capture("../images/blue_background_rotate.mp4");
    //cv::Mat3b chessboard_image=vision::read_image_from_source("../images/tic_tac_toe.jpg");
    while (true) {
        cv::Mat3b chessboard_image;
        capture>>chessboard_image;

        cv::Mat hsv_image;
        cv::cvtColor(chessboard_image,hsv_image,cv::COLOR_BGR2HSV);
        std::vector<cv::Mat> hsv_channels;
        cv::split(hsv_image,hsv_channels);

        cv::Mat grey_image;
        cv::cvtColor(chessboard_image,grey_image,cv::COLOR_BGR2GRAY);
        std::vector<cv::Mat> grey_channels;
        cv::split(grey_image,grey_channels);

        // cv::medianBlur(hsv_channels[2],hsv_channels[2],19);
        // cv::GaussianBlur(hsv_channels[2],hsv_channels[2],cv::Size(7,7),0);
        // cv::Mat1b thresh;
        // cv::adaptiveThreshold(hsv_channels[2],thresh,255,cv::ADAPTIVE_THRESH_GAUSSIAN_C,cv::THRESH_BINARY,15,-2);

        cv::medianBlur(hsv_channels[1],hsv_channels[1],19);
        cv::GaussianBlur(hsv_channels[1],hsv_channels[1],cv::Size(7,7),0);
        cv::Mat1b thresh;
        cv::adaptiveThreshold(hsv_channels[1],thresh,255,cv::ADAPTIVE_THRESH_GAUSSIAN_C,cv::THRESH_BINARY,15,-2);

        //---------------在这里使用与运算消除掉中间的框

        cv::Mat kernel=cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(5,5));
        cv::dilate(thresh,thresh,kernel);
        cv::erode(thresh,thresh,kernel);

        cv::Mat kernel1=cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(3,3));
        cv::erode(thresh,thresh,kernel1);
        cv::dilate(thresh,thresh,kernel1);




        cv::Mat1b edge_image;
        cv::Canny(thresh,edge_image,50,150);

        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(edge_image,contours,hierarchy,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_NONE);

        std::vector<std::vector<cv::Point>> filter_contours;
        for (auto &p : contours) {
            std::cout<<cv::contourArea(p)<<std::endl;
            if (cv::contourArea(p) > 4000 && cv::contourArea(p) < 30000 && (4 * M_PI * cv::contourArea(p)) / std::pow(cv::arcLength(p,true),2) > 0.84) {
                filter_contours.push_back(p);
            }
        }

        //cv::Mat contours_image = cv::Mat::zeros(chessboard_image.rows,chessboard_image.cols,CV_8UC3);
        //cv::drawContours(contours_image,filter_contours,-1,cv::Scalar(0,255,0),1);

        std::vector<cv::Mat> piece;
        std::vector<cv::Scalar> mean_vals;
        std::vector<std::vector<cv::Point>> write_piece_contours;
        std::vector<std::vector<cv::Point>> black_piece_contours;

        for (int i=0;i<filter_contours.size();i++) {
            cv::Mat mask = cv::Mat::zeros(chessboard_image.rows,chessboard_image.cols,CV_8UC1);
            cv::drawContours(mask,filter_contours,i,cv::Scalar(255),-1);
            cv::Scalar mean_val = cv::mean(hsv_channels[2],mask);
            mean_vals.push_back(mean_val);
            std::cout<<mean_val<<std::endl;
            //cv::Mat roi;
            //cv::bitwise_and(chessboard_image,chessboard_image,roi,mask);
            //piece.push_back(roi);
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
                black_piece_contours.push_back(filter_contours[i]);
                cv::Point2f circle_center;
                float radius;
                cv::minEnclosingCircle(filter_contours[i],circle_center,radius);
                black_piece_circles_center.push_back(circle_center);
                black_piece_circles_radius.push_back(radius);
            }

            if (mean_vals[i][0] > mean_mean_val+5 && mean_vals[i][0] > 140) {
                write_piece_contours.push_back(filter_contours[i]);
                cv::Point2f circle_center;
                float radius;
                cv::minEnclosingCircle(filter_contours[i],circle_center,radius);
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

        //std::sort(mean_vals.begin(),mean_vals.end(),[](cv::Scalar &a , cv::Scalar &b){return a[0]<b[0];});


        // for (int i=0;i<piece.size();i++) {
        //     cv::namedWindow("piece"+std::to_string(i),cv::WINDOW_NORMAL);
        //     cv::imshow("piece"+std::to_string(i),piece[i]);
        // }

        cv::namedWindow("output",cv::WINDOW_NORMAL);
        cv::imshow("output",chessboard_image);
        cv::namedWindow("output1",cv::WINDOW_NORMAL);
        cv::imshow("output1",thresh);
        cv::waitKey(50);
    }
}