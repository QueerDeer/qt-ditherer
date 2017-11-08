#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//find nearest color for our given in rgb in our palette
int nearestColor( int r, int g, int b, const QColor *palette, int size )
{
    if (palette == 0)
        return 0;

    int dr = palette[0].red() - r;
    int dg = palette[0].green() - g;
    int db = palette[0].blue() - b;

    int minDist =  dr*dr + dg*dg + db*db;
    int nearest = 0;

    for ( auto i = 1; i < size; ++i )
    {
        dr = palette[i].red() - r;
        dg = palette[i].green() - g;
        db = palette[i].blue() - b;

        int dist = dr*dr + dg*dg + db*db;

        if ( dist < minDist )
        {
            minDist = dist;
            nearest = i;
        }
    }

    return nearest;
}

void MainWindow::on_actionRGBtoGrayScale_triggered()
{
    std::cout << "waiting for dithering..." << std::endl;

    newimg = img;

    //create b&w palette
    int iMaxColors = 255;
    int t;
    QColor *palette = new QColor [iMaxColors];

    for ( t=0; t<iMaxColors/2; ++t )	{
        int fGreyLevel =  0;
        palette[t] = QColor (fGreyLevel, fGreyLevel, fGreyLevel);
    }
    for ( t; t<iMaxColors; ++t )	{
        int fGreyLevel =  255;
        palette[t] = QColor (fGreyLevel, fGreyLevel, fGreyLevel);
    }

/*
                for ( t=0; t<iMaxColors/4; ++t)	{
                    int fGreyLevel =  0;
                    palette[t] = QColor (fGreyLevel, fGreyLevel, fGreyLevel);
                }
                for ( t; t<iMaxColors/2; ++t)	{
                    int fGreyLevel =  85;
                    palette[t] = QColor (fGreyLevel, fGreyLevel, fGreyLevel);
                }
                for ( t; t<iMaxColors-iMaxColors/4; t++)	{
                    int fGreyLevel =  170;
                    palette[t] = QColor (fGreyLevel, fGreyLevel, fGreyLevel);
                }
                for ( t; t<iMaxColors; ++t)	{
                    int fGreyLevel =  255;
                    palette[t] = QColor (fGreyLevel, fGreyLevel, fGreyLevel);
                }
*/

    int size = iMaxColors;

    //grayscaling and dithering
    QImage dImage( newimg.width(), newimg.height(), QImage::Format_Indexed8); //for 8, not rgb's 32 bits per pix

    int i;
    dImage.setColorCount(size);
    for ( i = 0; i < size; ++i )
        dImage.setColor( i, palette[ i ].rgb() );

    int *rerr1 = new int [ newimg.width() * 2 ];
    int *gerr1 = new int [ newimg.width() * 2 ];
    int *berr1 = new int [ newimg.width() * 2 ];

    memset( rerr1, 0, sizeof( int ) * newimg.width() * 2 );
    memset( gerr1, 0, sizeof( int ) * newimg.width() * 2 );
    memset( berr1, 0, sizeof( int ) * newimg.width() * 2 );

    int *rerr2 = rerr1 + newimg.width();
    int *gerr2 = gerr1 + newimg.width();
    int *berr2 = berr1 + newimg.width();

    QTime timer;
    timer.start();
    for ( auto j = 0; j < newimg.height(); ++j )
    {
        uint *ip = (uint * )newimg.scanLine( j );
        uchar *dp = dImage.scanLine( j );

        for ( i = 0; i < newimg.width(); ++i )
        {
            rerr1[i] = rerr2[i] + qRed( *ip );
            rerr2[i] = 0;
            gerr1[i] = gerr2[i] + qGreen( *ip );
            gerr2[i] = 0;
            berr1[i] = berr2[i] + qBlue( *ip );
            berr2[i] = 0;
            ip++;
        }

        *dp++ = nearestColor( rerr1[0], gerr1[0], berr1[0], palette, size );

        for ( i = 1; i < newimg.width()-1; ++i )
        {
            int indx = nearestColor( rerr1[i], gerr1[i], berr1[i], palette, size );
            *dp = indx;

            int rerr = rerr1[i];
            rerr -= palette[indx].red();
            int gerr = gerr1[i];
            gerr -= palette[indx].green();
            int berr = berr1[i];
            berr -= palette[indx].blue();

            // diffuse red error
            rerr1[ i+1 ] += ( rerr * 7 ) >> 4;
            rerr2[ i-1 ] += ( rerr * 3 ) >> 4;
            rerr2[  i  ] += ( rerr * 5 ) >> 4;
            rerr2[ i+1 ] += ( rerr ) >> 4;

            // diffuse green error
            gerr1[ i+1 ] += ( gerr * 7 ) >> 4;
            gerr2[ i-1 ] += ( gerr * 3 ) >> 4;
            gerr2[  i  ] += ( gerr * 5 ) >> 4;
            gerr2[ i+1 ] += ( gerr ) >> 4;

            // diffuse blue error
            berr1[ i+1 ] += ( berr * 7 ) >> 4;
            berr2[ i-1 ] += ( berr * 3 ) >> 4;
            berr2[  i  ] += ( berr * 5 ) >> 4;
            berr2[ i+1 ] += ( berr ) >> 4;

            dp++;
        }

        *dp = nearestColor( rerr1[i], gerr1[i], berr1[i], palette, size );
    }
    std::cout << timer.elapsed() << std::endl;

    delete [] rerr1;
    delete [] gerr1;
    delete [] berr1;

    newimg = dImage;

    //------------------------------------------------

    std::cout << "done" << std::endl;

    ui->label_2->setPixmap(QPixmap::fromImage(newimg));
    ui->label_2->setScaledContents( true );
    ui->label_2->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
}

unsigned char clamp(float x)
    {
        return x < 0.0f ? 0 : (x > 255.0f ? 255 : (unsigned char)x);
    }

QColor cubicInterpolate(QColor p[4], float x)
{
    float red = p[1].red() + (-0.5 * p[0].red() + 0.5 * p[2].red()) * x + (p[0].red() - 2.5 * p[1].red() + 2.0 * p[2].red() - 0.5 * p[3].red()) * x * x + (-0.5 * p[0].red() + 1.5 * p[1].red() - 1.5 * p[2].red() + 0.5 * p[3].red()) * x * x * x;
    float green = p[1].green() + (-0.5 * p[0].green() + 0.5 * p[2].green()) * x + (p[0].green() - 2.5 * p[1].green() + 2.0 * p[2].green() - 0.5 * p[3].green()) * x * x + (-0.5 * p[0].green() + 1.5 * p[1].green() - 1.5 * p[2].green() + 0.5 * p[3].green()) * x * x * x;
    float blue = p[1].blue() + (-0.5 * p[0].blue() + 0.5 * p[2].blue()) * x + (p[0].blue() - 2.5 * p[1].blue() + 2.0 * p[2].blue() - 0.5 * p[3].blue()) * x * x + (-0.5 * p[0].blue() + 1.5 * p[1].blue() - 1.5 * p[2].blue() + 0.5 * p[3].blue()) * x * x * x;

    QColor tmp(clamp(red), clamp(green), clamp(blue));
    //std::cout << red << " " << green << " " << blue << std::endl;
    return tmp;
}

QColor bicubicInterpolate (QColor p[4][4], float x, float y)
{
    QColor tmp[4];
    tmp[0] = cubicInterpolate(p[0], y);
    tmp[1] = cubicInterpolate(p[1], y);
    tmp[2] = cubicInterpolate(p[2], y);
    tmp[3] = cubicInterpolate(p[3], y);
    return cubicInterpolate(tmp, x);
}

void MainWindow::on_actionBicubic_Interpolation_Resize_triggered()
{
    std::cout << "waiting for interpolation..." << std::endl;

    //Реализовать увеличение изображения в N раз с использованием бикубической
    //интерполяции, где N положительное действительное число, не более 10.
    int N = 4;
    QImage dImage( N*img.width()-4*N, N*img.height()-4*N, QImage::Format_RGB32);
    QColor p[4][4];

    QTime timer;
    timer.start();
    for ( auto i=0; i<img.width()-4; ++i )
        for ( auto j=0; j<img.height()-4; ++j )
        {
            for ( auto k=i; k<i+4; ++k )
                for ( auto l=j; l<j+4; ++l )
                    p[k-i][l-j]=img.pixelColor(k,l);

            for ( auto m=0; m<N; ++m )
                for ( auto n=0; n<N; ++n )
                {
                    QColor tmpPix = bicubicInterpolate (p, (float)m/8, (float)n/8);
                    dImage.setPixelColor(i*N+m, j*N+n, tmpPix);
                }
        }

    std::cout << timer.elapsed() << std::endl;

    newimg = dImage;

    //------------------------------------------------

    ui->label_2->setPixmap(QPixmap::fromImage(newimg));
    ui->label_2->setScaledContents( true );
    ui->label_2->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
}

void MainWindow::on_actionOpen_triggered()
{
    fileName = QFileDialog::getOpenFileName(this,
                                            tr("Open Image"), "",
                                            tr("Image (*.jpg *jpeg *png);;All Files (*)"));

    if (fileName.isEmpty())
    {
        return;
    }
    else
    {
        QImage buffer(fileName);
        img = buffer;

        ui->label->setPixmap(QPixmap::fromImage(img));
        ui->label->setScaledContents( true );
        ui->label->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    }
}

void MainWindow::on_actionSave_triggered()
{
    if (fileName.isEmpty())
    {
        return;
    }
    else
    {
        QString newName = QFileDialog::getSaveFileName(
                    this,
                    tr("Save image"),
                    QDir::currentPath(),
                    tr("Image (*.png *.jpg *.JPG *.jpeg *.JPEG);;All Files (*)") );

        newimg.save(newName);
    }
}
