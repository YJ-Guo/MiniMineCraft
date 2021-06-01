#include "mainwindow.h"
#include <ui_mainwindow.h>
#include "cameracontrolshelp.h"
#include <QDesktopWidget>
#include <QResizeEvent>
#include <QFileDialog>
#include <QMessageBox>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), cHelp(), HighMap(new QImage())
{
    ui->setupUi(this);
    ui->mygl->setFocus();
    this->playerInfoWindow.show();
    playerInfoWindow.move(QApplication::desktop()->screen()->rect().center() - this->rect().center() + QPoint(this->width() * 1.0, 0));

    connect(ui->mygl, SIGNAL(sig_sendPlayerPos(QString)), &playerInfoWindow, SLOT(slot_setPosText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerVel(QString)), &playerInfoWindow, SLOT(slot_setVelText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerAcc(QString)), &playerInfoWindow, SLOT(slot_setAccText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerLook(QString)), &playerInfoWindow, SLOT(slot_setLookText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerChunk(QString)), &playerInfoWindow, SLOT(slot_setChunkText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerTerrainZone(QString)), &playerInfoWindow, SLOT(slot_setZoneText(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionCamera_Controls_triggered()
{
    cHelp.show();
}

void MainWindow::on_actionLoad_Gray_Map_triggered() {
    QString filepath = QFileDialog::getOpenFileName(0, QString("Load Hight Map Image"),
                                                    QString(),
                                                    QString("*.png *.jpg *.bmp *.jpeg"));
    if(filepath.isEmpty()) {
        // Check for file validity
        std::cout << "Invalid Image Choice!" << std::endl;
        return;
    } else {
        if(!(HighMap->load(filepath))) {
            QMessageBox::information(this,
                                     tr("Fail to Open Selected Image!"),
                                     tr("Fail to Open Selected Image!"));
            return;
        }
    }

//    std::cout << "Image width:" << HighMap->width() << std::endl;
//    std::cout << "Image height:" << HighMap->height() << std::endl;

    ui->mygl->UpdateTerrainWithHightMap(HighMap, false);
}

void MainWindow::on_actionLoad_Color_Map_triggered() {
    QString filepath = QFileDialog::getOpenFileName(0, QString("Load Hight Map Image"),
                                                    QString(),
                                                    QString("*.png *.jpg *.bmp *.jpeg"));
    if(filepath.isEmpty()) {
        // Check for file validity
        std::cout << "Invalid Image Choice!" << std::endl;
        return;
    } else {
        if(!(HighMap->load(filepath))) {
            QMessageBox::information(this,
                                     tr("Fail to Open Selected Image!"),
                                     tr("Fail to Open Selected Image!"));
            return;
        }
    }

    // Change the Color Map to Grey Scale
//    HighMap = GreyScale(HighMap);
    HighMap = new QImage(HighMap->mirrored());
//    std::cout << "Image width:" << HighMap->width() << std::endl;
//    std::cout << "Image height:" << HighMap->height() << std::endl;
//    HighMap = HighMap->mirrored();
    ui->mygl->UpdateTerrainWithHightMap(HighMap, true);
}

// Convert the Color Image to Grey Scale Image
QImage* MainWindow::GreyScale(QImage *origin) {
    QImage * newImage = new QImage(origin->width(), origin->height(), QImage::Format_ARGB32);
        QColor oldColor;

        for (int x = 0; x < newImage->width(); ++x){
            for (int y = 0; y < newImage->height(); ++y){
                oldColor = QColor(origin->pixel(x,y));
                int grey = oldColor.red() * 0.3 + oldColor.green() * 0.59 + oldColor.blue() * 0.11;
                newImage->setPixel(x, y, qRgb(grey,  grey, grey));
            }
        }

        return newImage;
}
