#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "cameracontrolshelp.h"
#include "playerinfo.h"


namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QImage* HighMap;
    QImage* GreyScale(QImage *origin);
private slots:
    void on_actionQuit_triggered();

    void on_actionCamera_Controls_triggered();
    void on_actionLoad_Gray_Map_triggered();
    void on_actionLoad_Color_Map_triggered();
private:
    Ui::MainWindow *ui;
    CameraControlsHelp cHelp;
    PlayerInfo playerInfoWindow;
};


#endif // MAINWINDOW_H
