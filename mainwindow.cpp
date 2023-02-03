#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"

#include <iostream>
#include <dirent.h>

#include <QFileDialog>
#include <QFile>
#include <QTextCodec>
#include <QFileInfo>
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>

using namespace cv;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 设置只能输入数字
    ui->lineEdit_2->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));
    ui->lineEdit_3->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));
    ui->lineEdit_4->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));

//    ui->lineEdit_2->setText("50");  // 默认棋盘格宽
//    ui->lineEdit_3->setText("15");  // 默认内角点数量（宽）
//    ui->lineEdit_4->setText("17");  // 默认内角点数量（高）

    ui->tabWidget->setCurrentIndex(0);      // 设置默认 tab 页面

    resize(1200, 800);
    connect(ui->action, &QAction::triggered, this, &MainWindow::chooseImage);
    connect(ui->btn_chooseDir, &QPushButton::clicked, this, &MainWindow::chooseImage);
    connect(ui->btn_calib, &QPushButton::clicked, this, &MainWindow::intrinsicCalib);
    connect(ui->btn_clear, &QPushButton::clicked, this, &MainWindow::clear);
    connect(ui->apply_param, &QPushButton::clicked, this, &MainWindow::changeTab);

    connect(ui->btn_chooseSingleImg, &QPushButton::clicked, this, &MainWindow::chooseSingleImg);
    connect(ui->btn_fix, &QPushButton::clicked, this, &MainWindow::fixImg);
}

void MainWindow::fixImg() {
    if (this->singleImgPath == "") {
        QMessageBox::information(this, "注意", "请先选择需要修复的图片");
        return;
    }
    // 更新相机参数
    this->updateCameraIntrinsic();
    // 更新 map1, map2
    this->calibrator.updateUndistortMap();
    // 修复畸变
    cv::Mat undistorted_image;
    this->calibrator.applyFixImage(this->singleImg, undistorted_image);
    this->showImageInLabel(undistorted_image);
}

void MainWindow::chooseSingleImg() {
    ui->statusBar->showMessage(tr("选择需要修复的图像"), 3000);
    this->singleImgPath = QFileDialog::getOpenFileName(this, "选择图片", "~/", QStringLiteral("*png *jpg"));
    // 如果用户没有选择路径，直接返回
    if (this->singleImgPath == "") return;
    ui->img_dir->setText(this->singleImgPath);
    this->singleImg = cv::imread(this->singleImgPath.toStdString());
    this->showImageInLabel(this->singleImg);
}

void MainWindow::updateCameraIntrinsic() {
    bool fx_ok, fy_ok, u0_ok, v0_ok;
    double fx = ui->line_fx->text().toDouble(&fx_ok);
    double fy = ui->line_fy->text().toDouble(&fy_ok);
    double u0 = ui->line_u0->text().toDouble(&u0_ok);
    double v0 = ui->line_v0->text().toDouble(&v0_ok);

    bool k1_ok, k2_ok, p1_ok, p2_ok, p3_ok;
    double k1 = ui->line_k1->text().toDouble(&k1_ok);
    double k2 = ui->line_k2->text().toDouble(&k2_ok);
    double p1 = ui->line_p1->text().toDouble(&p1_ok);
    double p2 = ui->line_p2->text().toDouble(&p2_ok);
    double p3 = ui->line_p3->text().toDouble(&p3_ok);
    if (fx_ok && fy_ok && u0_ok && v0_ok && k1_ok && k2_ok && p1_ok && p2_ok && p3_ok) {

    } else {
        QMessageBox::information(this, "提示", "请正确输入参数值！");
        return;
    }
    this->calibrator.camera_intrinsic_.at<double>(0, 0) = fx;
    this->calibrator.camera_intrinsic_.at<double>(1, 1) = fy;
    this->calibrator.camera_intrinsic_.at<double>(0, 2) = u0;
    this->calibrator.camera_intrinsic_.at<double>(1, 2) = v0;
    this->calibrator.camera_dist_.at<double>(0, 0) = k1;
    this->calibrator.camera_dist_.at<double>(0, 1) = k2;
    this->calibrator.camera_dist_.at<double>(0, 2) = p1;
    this->calibrator.camera_dist_.at<double>(0, 3) = p2;
    this->calibrator.camera_dist_.at<double>(0, 4) = p3;
}

void MainWindow::showImageInLabel(cv::Mat img) {
    cv::Mat temp;
    if(img.channels()==4)
        cv::cvtColor(img, temp,cv::COLOR_BGRA2RGB);
    else if (img.channels()==3)
        cv::cvtColor(img, temp,cv::COLOR_BGR2RGB);
    else
        cv::cvtColor(img, temp,cv::COLOR_GRAY2RGB);

    QImage qImg = QImage((const unsigned char*)(temp.data), temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(qImg);
    QPixmap scale_pixmap = pixmap.scaled(ui->label_img->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_img->setScaledContents(true);
    ui->label_img->setPixmap(scale_pixmap);
}

void MainWindow::changeTab() {

    ui->tabWidget->setCurrentIndex(1);
    ui->line_fx->setText(QString("%1").arg(calibrator.camera_intrinsic_.at<double>(0, 0)));
    ui->line_fy->setText(QString("%1").arg(calibrator.camera_intrinsic_.at<double>(1, 1)));
    ui->line_u0->setText(QString("%1").arg(calibrator.camera_intrinsic_.at<double>(0, 2)));
    ui->line_v0->setText(QString("%1").arg(calibrator.camera_intrinsic_.at<double>(1, 2)));

    ui->line_k1->setText(QString("%1").arg(calibrator.camera_dist_.at<double>(0, 0)));
    ui->line_k2->setText(QString("%1").arg(calibrator.camera_dist_.at<double>(0, 1)));
    ui->line_p1->setText(QString("%1").arg(calibrator.camera_dist_.at<double>(0, 2)));
    ui->line_p2->setText(QString("%1").arg(calibrator.camera_dist_.at<double>(0, 3)));
    ui->line_p3->setText(QString("%1").arg(calibrator.camera_dist_.at<double>(0, 4)));
}

void MainWindow::clear() {
    ui->statusBar->clearMessage();
    ui->listWidget->clear();    // 清空图片
    // 清空标定板信息
    ui->lineEdit->clear();
    ui->lineEdit_2->clear();
    ui->lineEdit_3->clear();
    ui->lineEdit_4->clear();
    // 清空参数展示信息
    this->calibrator.camera_intrinsic_ = cv::Mat::zeros(3, 3, CV_32F);
    this->calibrator.camera_dist_ = cv::Mat::zeros(1, 5, CV_32F);
    ui->textBrowser->clear();
    ui->btn_calib->setEnabled(true);
}

void MainWindow::chooseImage() {
    ui->statusBar->showMessage(tr("选择图像路径"), 2000);
    QString path = QFileDialog::getExistingDirectory(this, "选择图片路径", "./");

    // 如果用户没有选择路径，直接返回
    if (path == "") return;

    this->path_ = path;
    ui->lineEdit->setText(path);
    QDir dir(path);
    dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);

    QStringList img_names = dir.entryList();

    ui->listWidget->setViewMode(QListWidget::IconMode);
    ui->listWidget->setIconSize(QSize(150, 150));
    ui->listWidget->setSpacing(5);
    ui->listWidget->setResizeMode(QListView::Adjust);
    ui->listWidget->setMovement(QListView::Static);

    for (int i=0; i<img_names.size(); i++) {
        QListWidgetItem *imgItem = new QListWidgetItem;
        QString tmp_path = path + "/" + img_names.at(i);
        imgItem->setIcon(QIcon(tmp_path));
        imgItem->setText(img_names.at(i));
        ui->listWidget->addItem(imgItem);

        this->img_paths.push_back(tmp_path.toStdString());
    }
}

void MainWindow::intrinsicCalib() {
    ui->statusBar->showMessage(tr("开始标定..."));
    ui->btn_calib->setEnabled(false);

    QString gard_size_str = ui->lineEdit_2->text();
    QString width_str = ui->lineEdit_3->text();
    QString hight_str = ui->lineEdit_4->text();
    if (gard_size_str.size()==0 || width_str.size()==0 || hight_str.size()==0) {
        QMessageBox::information(this, "提示", "请输入标定板信息！");
        return;
    }
    bool gard_size_ok, width_ok, hight_ok;
    int grid_size = gard_size_str.toInt(&gard_size_ok);
    int width = width_str.toInt(&width_ok);
    int hight = hight_str.toInt(&hight_ok);
    if (gard_size_ok && width_ok && hight_ok) {

    } else {
        QMessageBox::information(this, "提示", "请输入数字！");
        return;
    }

    calibrator.Calibrate(this->img_paths, grid_size, width, hight);
    ui->statusBar->showMessage(tr("标定成功"));
    ui->textBrowser->insertPlainText("相机内参矩阵: \n");
    QString intrinsic = QString("Size: %1 %2\n").arg(calibrator.camera_intrinsic_.rows)
                                              .arg(calibrator.camera_intrinsic_.cols);
    ui->textBrowser->insertPlainText(intrinsic);
    for (int i = 0; i < calibrator.camera_intrinsic_.rows; i++) {
        QString res = QString("%1\t%2\t%3\n").arg(calibrator.camera_intrinsic_.at<double>(i, 0))
                                             .arg(calibrator.camera_intrinsic_.at<double>(i, 1))
                                             .arg(calibrator.camera_intrinsic_.at<double>(i, 2));
        ui->textBrowser->insertPlainText(res);
    }
    ui->textBrowser->insertPlainText("相机内参:\n");
    ui->textBrowser->insertPlainText(QString("Fx:%1\n").arg(calibrator.camera_intrinsic_.at<double>(0, 0)));
    ui->textBrowser->insertPlainText(QString("Fy:%1\n").arg(calibrator.camera_intrinsic_.at<double>(1, 1)));

    ui->textBrowser->insertPlainText(QString("U0:%1\n").arg(calibrator.camera_intrinsic_.at<double>(0, 2)));
    ui->textBrowser->insertPlainText(QString("V0:%1\n").arg(calibrator.camera_intrinsic_.at<double>(1, 2)));


    ui->textBrowser->insertPlainText("\n畸变参数: \n");
//    QString distortion = QString("Size: %1 %2\n").arg(calibrator.camera_dist_.rows).arg(calibrator.camera_dist_.cols);
//    ui->textBrowser->insertPlainText(distortion);
    QString dist_res = QString("K1:%1\nK2:%2\nP1:%3\nP2:%4\nK3:%5\n").arg(calibrator.camera_dist_.at<double>(0, 0))
                                         .arg(calibrator.camera_dist_.at<double>(0, 1))
                                         .arg(calibrator.camera_dist_.at<double>(0, 2))
                                        .arg(calibrator.camera_dist_.at<double>(0, 3))
                                        .arg(calibrator.camera_dist_.at<double>(0, 4));
    ui->textBrowser->insertPlainText(dist_res);

}


MainWindow::~MainWindow()
{
    delete ui;
}







