#include "intrinsiccalibration.h"

#include "intrinsiccalibration.h"
#include "autoimagepicker.h"
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

bool IntrinsicCalibration::Calibrate(const std::string &img_dir_path,
                                     const int &grid_size, // in milimeter 50mm
                                     const int &corner_width,
                                     const int &corner_height)
{
    // read image files
    std::vector<std::string> file_names;
    if (img_dir_path.rfind('/') != img_dir_path.size() - 1){
        img_dir_path_ = img_dir_path + "/";
    }
    else{
        img_dir_path_ = img_dir_path;
    }

    DIR *dir;
    struct dirent *ptr;
    if ((dir = opendir(img_dir_path_.c_str())) == NULL) {
        std::cerr << "[ERROR]Open Image Folder Failed." << std::endl;
        exit(1);
    }
    while ((ptr = readdir(dir)) != NULL) {
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0 &&
            opendir((img_dir_path_ + ptr->d_name).c_str()) == nullptr)
            file_names.push_back(ptr->d_name);
    }
    if (file_names.size() == 0) {
        std::cerr << "[ERROR]No Calibration Image Files.";
        return false;
    }
    closedir(dir);


    grid_size_ = grid_size;
    corner_size_ = cv::Size(corner_width, corner_height);
    std::vector<cv::Point3f> object_corners;
    // generate 3D corner points
    for (int i = 0; i < corner_size_.height; i++) {
        for (int j = 0; j < corner_size_.width; j++) {
            object_corners.push_back(cv::Point3f(i, j, 0.0f));
        }
    }

    // pick image
    cv::Mat init_img = cv::imread(img_dir_path_+file_names[0], 0);
    img_size_ = init_img.size();
    std::cout << file_names[0] << " " << img_size_.width << " " << img_size_.height << " "
    << corner_width << " " << corner_height << std::endl;
    AutoImagePicker image_selector(img_size_.width, img_size_.height,
                                   corner_width, corner_height);
    // detect chessboard corner
//    int total_image_num = file_names.size();
    int detected_image_num = 0;
    for (unsigned int i = 0; i < file_names.size(); i++){
        cv::Mat input_image = cv::imread(img_dir_path_+file_names[i], 0);
        // img_size_ = input_image.size();
        // detect corner
        std::vector<cv::Point2f> image_corners;
        /*
         * @param input_image cv::Mat
         * @param corner_size_ 每行每列内角点个数 cv::Size
         * @param image_corners 输出的内角点坐标
         */
        bool whether_found = cv::findChessboardCorners(input_image,
                                                       corner_size_,
                                                       image_corners);
        // refining pixel coordinates for given 2d points
        if (whether_found){
            // whether select this image for calibration
            if (image_selector.addImage(image_corners)){

                cv::TermCriteria criteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER,
                                        grid_size_, 0.001);
                /*
                 * 检测到的角点作进一步的优化计算，可使角点的精度达到亚像素级别。
                 * @param input_image, image_corners: 输入图像与检测到的角点
                 * @param winSize 计算亚像素角点时考虑的区域的大小 cv::Size
                 * @param 类似 winSize 通常忽略（即Size(-1, -1)）
                 * @param criteria 计算亚像素时停止迭代的标准
                 */
                cv::cornerSubPix(input_image, image_corners, cv::Size(5, 5),
                                cv::Size(-1, -1), criteria);
                // check whether the corner detect is complete
                // if (image_corners.size() == corner_size_.area()) {
                // for (int pt = 0; pt < image_corners.size(); pt++){
                //     if (pt % 15 == 0) std::cout << "==\n";
                //     std::cout << pt%15 << ": " << image_corners[pt].x << ", "
                //                     << image_corners[pt].y << std::endl;
                // }
                addPoints(image_corners, object_corners);
                detected_image_num ++;
                // display corner detect
                cv::drawChessboardCorners(input_image, corner_size_,
                                        image_corners, whether_found);
            }
            if (image_selector.status()) break;
        }
    }
    cv::destroyAllWindows();
    /*
     * @paramobject_points_ 世界坐标系中的点 std::vector<std::vector<cv::Point3f>>
     * image_points_: 对应的图像点 std::vector<std::vector<cv::Point2f>>
     * img_size_: 图像尺寸 cv::Size
     * output:
     * camera_intrinsic_: 内在参数矩阵
     * camera_dist_: 畸变矩阵
     * R_mats_, t_mats_: 外部参数 旋转和平移 std::vector<cv::Mat>
     */
    cv::calibrateCamera(object_points_, image_points_, img_size_,
                        camera_intrinsic_, camera_dist_, R_mats_, t_mats_);

    // check min valid num of images
    if (detected_image_num < MIN_CALI_IMAGE_NUM) {
        std::cerr << "[WARNING]Detected image num less than minium requirement.\n";
    }

    std::cout << "\nrvecs 旋转向量：" <<  std::endl;
    for (unsigned int i=0; i<R_mats_.size(); i++) {
        for (int j=0; j<R_mats_[i].rows; j++) {
            std::cout << R_mats_[i].at<double>(0, j) << ",";
        }
        std::cout << std::endl;
    }
    std::cout << "\nrvecs 位移向量：" << t_mats_[0].rows << " " << t_mats_[0].cols << std::endl;

    return true;
}
