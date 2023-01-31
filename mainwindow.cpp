#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"

#include <iostream>
#include <dirent.h>

#include "intrinsiccalibration.h"

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

    resize(1200, 800);
    connect(ui->action, &QAction::triggered, this, &MainWindow::chooseImage);
    connect(ui->btn_chooseDir, &QPushButton::clicked, this, &MainWindow::chooseImage);
    connect(ui->btn_calib, &QPushButton::clicked, this, &MainWindow::intrinsicCalib);
    connect(ui->btn_clear, &QPushButton::clicked, this, &MainWindow::clear);
}

void MainWindow::clear() {
    ui->listWidget->clear();

    ui->lineEdit->clear();
    ui->lineEdit_2->clear();
    ui->lineEdit_3->clear();
    ui->lineEdit_4->clear();

    ui->textBrowser->clear();
    ui->btn_calib->setEnabled(true);
}

void MainWindow::chooseImage() {
    QString path = QFileDialog::getExistingDirectory(this, "选择图片路径", "./");

    // 如果用户没有选择路径，直接返回
    if (path == "") return;

    this->path_ = path;
//    std::string input_image_path = path.toStdString();
    ui->lineEdit->setText(path);
    QDir dir(path);
    dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);

    QStringList imgList = dir.entryList();

    ui->listWidget->setViewMode(QListWidget::IconMode);
    ui->listWidget->setIconSize(QSize(150, 150));
    ui->listWidget->setSpacing(5);
    ui->listWidget->setResizeMode(QListView::Adjust);
    ui->listWidget->setMovement(QListView::Static);

    for (int i=0; i<imgList.size(); i++) {
        QListWidgetItem *imgItem = new QListWidgetItem;
        imgItem->setIcon(QIcon(path + "/" + imgList.at(i)));
        imgItem->setText(imgList.at(i));
//        imgItem->setSizeHint(QSize(155, 145));
        ui->listWidget->addItem(imgItem);
    }
}

void MainWindow::intrinsicCalib() {

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
    IntrinsicCalibration calibrator;
    calibrator.Calibrate(this->path_.toStdString(), grid_size, width, hight);

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







