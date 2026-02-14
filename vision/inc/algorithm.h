//
// Created by wjr on 2026/2/13.
//

#ifndef ELECTRIC_COMPETITION_ALGORITHM_H
#define ELECTRIC_COMPETITION_ALGORITHM_H

#include "opencv2/opencv.hpp"

namespace algorithm {
    void four_points_sorting(std::vector<cv::Point2f> &corners);
}

#endif //ELECTRIC_COMPETITION_ALGORITHM_H