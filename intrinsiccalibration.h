#pragma once

#include "common.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <opencv2/opencv.hpp>

#define MIN_CALI_IMAGE_NUM 15


class IntrinsicCalibration
{
public:
    IntrinsicCalibration() {}

    bool Calibrate(const std::vector<std::string> &file_names,
                   const int &grid_size = 50, // in milimeter 50mm
                   const int &corner_width = 15, const int &corner_height = 17);

    // bool addSingleImage(const std::string &img_path);

    bool undistortSingleImage(const std::string &image_path,
                              const std::string &output_image_path);

    bool getCameraIntrinsicParam(cv::Mat &camera_intrinsic, cv::Mat &camera_dist)
    {
        camera_intrinsic = camera_intrinsic_.clone();
        camera_dist = camera_dist_.clone();
        return true;
    }
    bool getCameraIntrinsicParam(
        std::vector<std::vector<double>> &camera_intrinsic,
        std::vector<double> camera_dist)
    {
        camera_intrinsic.clear();
        camera_dist.clear();
        for (int i = 0; i < 3; i++){
            camera_intrinsic.push_back(std::vector<double>());
            camera_intrinsic_.row(i).copyTo(camera_intrinsic[i]);
        }
        camera_dist_.col(0).copyTo(camera_dist);
        return true;
    }

    double getCameraFy()
    {
        return camera_intrinsic_.at<double>(1, 1);
    }

    void applyFixImage(cv::Mat &img, cv::Mat &undistorted_image) {
        cv::remap(img, undistorted_image, map1, map2, cv::INTER_LINEAR);
    }

    void updateUndistortMap() {
        cv::initUndistortRectifyMap(camera_intrinsic_, camera_dist_, cv::Mat(),
                                    camera_intrinsic_, img_size_, CV_32FC1, this->map1, this->map2);
    }

    cv::Mat camera_intrinsic_ = cv::Mat::zeros(3, 3, CV_32FC1);
    cv::Mat camera_dist_ = cv::Mat::zeros(1, 5, CV_32FC1);

private:
    // input image path
    std::string img_dir_path_;
    // output undistorted image path
    std::string undistort_image_path_;
    std::string selected_image_path_;

    // intrinsic params

    // image size
    cv::Size img_size_;
    // grid size in mm
    int grid_size_;
    // board corner size
    cv::Size corner_size_;
    // successfully corner-detected image num
    int valid_img_num_;
    // save 3D position of corners in each chessboard
    std::vector<std::vector<cv::Point3f>> object_points_;
    // save 2D pixel position of corners in each chessboard
    std::vector<std::vector<cv::Point2f>> image_points_;

    // extrinsic rotation and translation
    // camera or chessboard?
    std::vector<cv::Mat> R_mats_, t_mats_;
    cv::Mat map1;
    cv::Mat map2;
    // option param
    // int calibration_option_;
    // bool init_undistort_;

private:
    // bool checkReprojectionError () {};
    bool undistortImages(const std::vector<std::string> &image_names);

    void addPoints(const std::vector<cv::Point2f>& image_corners,
                   const std::vector<cv::Point3f>& object_corners)
    {
        image_points_.push_back(image_corners);
        object_points_.push_back(object_corners);
    }

    // bool saveCalibrationResult () {};

};
