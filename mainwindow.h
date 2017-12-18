#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QPixmap>
#include <QFileDialog>
#include <QString>
#include <iostream>
#include <QTime>
#include <math.h>

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

    void on_actionAWGN_triggered();

    void on_actionImpulse_Noise_triggered();

    void on_actionBlending_triggered();
    
    void on_actionGlobal_Contrast_triggered();

    void on_actionLinear_Filtration_triggered();

    void on_actionMediana_triggered();

    void on_actionBinarization_triggered();

    void on_actionBDZ_triggered();

private:
    Ui::MainWindow *ui;

    QString fileName;

    QImage img;
    QImage newimg;
    QImage newimg2;

    unsigned char header[14];
    unsigned char * info;
    unsigned char ** image;
    unsigned char colors[256][3];
    unsigned char RGBreserved[256];
    int width, height, bits, header_size;

    int find_pix_color(int x, int y, unsigned char **im);
    int readRLE(FILE *f);
    int readBMP_Win(FILE *f);
    void readBMP_CORE(FILE *f);
    int readBMP(QString file_name);
    void writeBMP(QString file_name);
    bool FFT(double *Rdat, double *Idat, int N, int LogN, int Ft_Flag);
    int getClosestLog(const int number);
};

#endif // MAINWINDOW_H
