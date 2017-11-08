#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QPixmap>
#include <QFileDialog>
#include <QString>
#include <iostream>
#include <QTime>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionRGBtoGrayScale_triggered();

    void on_actionOpen_triggered();

    void on_actionSave_triggered();

    void on_actionBicubic_Interpolation_Resize_triggered();

private:
    Ui::MainWindow *ui;

    QString fileName;

    QImage img;
    QImage newimg;
};

#endif // MAINWINDOW_H
