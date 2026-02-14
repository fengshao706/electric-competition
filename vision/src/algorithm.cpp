//
// Created by wjr on 2026/2/13.
//

#include "../inc/algorithm.h"

namespace algorithm {
    void four_points_sorting(std::vector<cv::Point2f> &corners) {
        cv::Point2f center(0.f, 0.f);
        for (auto& p : corners) {
            center += p;
        }
        center *= 0.25f;

        std::sort(corners.begin(), corners.end(),
        [&center](const cv::Point2f& a, const cv::Point2f& b) {
            return std::atan2(a.y - center.y, a.x - center.x) <
                   std::atan2(b.y - center.y, b.x - center.x);
        });

        int tl_idx = 0;
        for (int i = 1; i < 4; ++i)
            if (corners[i].y + corners[i].x < corners[tl_idx].y + corners[tl_idx].x)
                tl_idx = i;

        // 重新旋转
        std::rotate(corners.begin(), corners.begin() + tl_idx, corners.end());
    }
}
