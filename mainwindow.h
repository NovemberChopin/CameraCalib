#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <QMessageBox>

#include "intrinsiccalibration.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void chooseImage();
    void chooseSingleImg();
    void intrinsicCalib();
    void clear();
    void changeTab();
    void showImageInLabel(cv::Mat img);
    void fixImg();
    void updateCameraIntrinsic();

    QString path_;
    std::vector<std::string> img_paths;

private:
    Ui::MainWindow *ui;
    IntrinsicCalibration calibrator;

    QString singleImgPath = "";
    cv::Mat singleImg;
};

#endif // MAINWINDOW_H
