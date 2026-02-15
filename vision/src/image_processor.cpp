//
// Created by wjr on 2026/2/10.
//
#include "image_processor.h"
#include "algorithm.h"

namespace vision {
    cv::Mat3b read_image_from_source(const std::string path) {
        cv::Mat image;
        try {
            image=cv::imread(path);
        }catch (cv::Exception &e) {
            std::cerr<<"Your input path is wrong !!!"<<std::endl;
        }
        return image;
    }

    cv::Mat3b read_video_from_source(cv::VideoCapture &cap) {
        cv::Mat image;
        cap >> image;
        if (image.empty()) {
            std::cout<<"video_end"<<std::endl;
        }
        return image;
    }

    cv::Mat1b image_preprocessing(cv::Mat3b &image,cv::Size image_size,int gaussian_blur_size,int threshold_size,double threshold_c,int closing_size) {
        cv::resize(image,image,image_size);
        cv::Mat gray_image;
        cv::cvtColor(image,gray_image,cv::COLOR_BGR2GRAY);
        cv::GaussianBlur(gray_image,gray_image,cv::Size(gaussian_blur_size,gaussian_blur_size),0);

        cv::Mat thresh_image;
        cv::adaptiveThreshold(gray_image,thresh_image,255,cv::ADAPTIVE_THRESH_GAUSSIAN_C,cv::THRESH_BINARY_INV,threshold_size,threshold_c);

        cv::Mat kernel=cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(closing_size,closing_size));
        cv::erode(thresh_image,thresh_image,kernel);
        cv::dilate(thresh_image,thresh_image,kernel);
        return thresh_image;
    }

    std::vector<std::vector<cv::Point>> get_inner_rectangle(cv::Mat1b thresh_image,int contour_area_threshold,double min_contour_area_ratio,double max_contour_area_ratio) {
        cv::Mat edge_image;
        cv::Canny(thresh_image,edge_image,50,150);

        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(edge_image,contours,hierarchy,cv::RETR_LIST,cv::CHAIN_APPROX_NONE);

        std::vector<std::vector<cv::Point>> filter_contours;
        std::vector<cv::Rect> rectangle;
        std::vector<std::vector<cv::Point>> polygons;
        for (int i=0;i<contours.size();i++) {
            if (cv::contourArea(contours[i])>contour_area_threshold) {
                filter_contours.push_back(contours[i]);
                rectangle.push_back(cv::boundingRect(contours[i]));
                std::vector<cv::Point> temp_polygon;
                cv::approxPolyDP(contours[i],temp_polygon,30,true); //使用多边形拟合轮廓
                if (temp_polygon.size()==4) {
                    polygons.push_back(temp_polygon);
                }
            }
        }

        if (polygons.empty()) {
            std::cout<<"polygon_empty"<<std::endl;
            std::vector<std::vector<cv::Point>> empty;
            return empty;
        }

        for (int i=0;i<polygons.size();i++) { //按照面积升序排列图形
            for (int j=0;j<polygons.size()-1;j++) {
                if (cv::contourArea(polygons[j]) > cv::contourArea(polygons[j+1])) {
                    std::vector<cv::Point> temp_polygon;
                    temp_polygon=polygons[j];
                    polygons[j]=polygons[j+1];
                    polygons[j+1]=temp_polygon;
                }
            }
        }

        std::vector<ChildToParent> child_to_parents;
        int result_idx=-1;
        if (polygons.size()>=2) {
            for (int i=0;i<polygons.size()-1;i++) {
                short count = 0;
                for (int j=0;j<4;j++) {
                    if (cv::pointPolygonTest(polygons[i+1],polygons[i][j],false)==1) {
                        count++;
                    }
                }
                if (count==4) {
                    ChildToParent child_to_parent;
                    child_to_parent.child=polygons[i];
                    child_to_parent.parent=polygons[i+1];
                    child_to_parent.child_idx=i;
                    child_to_parents.push_back(child_to_parent);
                }
            }
        }else {
            result_idx=0;
        }

        for (int i = 0; i < child_to_parents.size(); i++) {
            std::cout<<cv::contourArea(child_to_parents[i].child) / cv::contourArea(child_to_parents[i].parent)<<std::endl;
            if (cv::contourArea(child_to_parents[i].child) / cv::contourArea(child_to_parents[i].parent) > min_contour_area_ratio
                && cv::contourArea(child_to_parents[i].child) / cv::contourArea(child_to_parents[i].parent) < max_contour_area_ratio
            ) {
                result_idx=child_to_parents[i].child_idx;
                break;
            }
        }

        std::cout<<result_idx<<std::endl;

        std::vector<std::vector<cv::Point>> result_contour;
        if (result_idx!=-1) {
            result_contour.push_back(polygons[result_idx]);
        }
        return result_contour;
    }

    std::vector<cv::Vec3f> get_circle(const cv::Mat1b &thresh_image) {
        std::vector<cv::Vec3f> circle;
        cv::HoughCircles(thresh_image,circle,cv::HOUGH_GRADIENT_ALT,1.5,20,100,0.8,10,500);
        return circle;
    }

    std::vector<cv::Point2f> perspective_transform_get_center_point(cv::Mat pre_image,std::vector<std::vector<cv::Point> > contours,
                                                                    cv::Size after_transform_image_size) {
        std::vector<cv::Point2f> dst_pts = {
            {0, 0}, {0, static_cast<float>(after_transform_image_size.height)},
            {
                static_cast<float>(after_transform_image_size.width),
                static_cast<float>(after_transform_image_size.height)
            },
            {static_cast<float>(after_transform_image_size.width), 0}
        }; //变换后的四个点坐标

        std::vector<cv::Point2f> corners;
        for (auto &contour : contours[0]) {
            corners.emplace_back(contour.x,contour.y);
        }
        algorithm::four_points_sorting(corners);


        std::vector<cv::Point2f> src_pts = { //变换前的四个点坐标
            {corners[0].x, corners[0].y},
            {corners[1].x, corners[1].y},
            {corners[2].x, corners[2].y},
            {corners[3].x, corners[3].y}
        };
        std::cout<<contours[0]<<std::endl;
        cv::Mat H = cv::getPerspectiveTransform(src_pts,dst_pts); //获得透视变换矩阵
        cv::Mat H_inv = H.inv();

        cv::Mat3b output_image;
        cv::warpPerspective(pre_image,output_image,H,cv::Size(after_transform_image_size.width,after_transform_image_size.height));

        cv::Point2f center_point=cv::Point(output_image.cols/2,output_image.rows/2);
        std::vector<cv::Point2f> pre_center_points;
        std::vector<cv::Point2f> center_points;
        center_points.push_back(center_point);

        cv::perspectiveTransform(center_points,pre_center_points,H_inv);

        return pre_center_points;
    }
}
