#include "mainwindow.h"
#include "ui_mainwindow.h"

#define PI 3.1415926536


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

int custom_clamp(float x, float a, float b)
{
    return x < a ? (int)a : (x > b ? (int)b : (int)x);
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
    //Реализовать увеличение изображения в N раз с использованием бикубической
    //интерполяции, где N положительное действительное число, не более 10.
    int N = 3;
    QImage dImage( img.width()/N-N, img.height()/N-N, QImage::Format_RGB32);
    QColor p[4][4];


    for ( auto i=0; i<img.width()-4; i+=N )
        for ( auto j=0; j<img.height()-4; j+=N )
        {
            for ( auto k=i; k<i+4; ++k )
                for ( auto l=j; l<j+4; ++l )
                    p[k-i][l-j]=img.pixelColor(k,l);

            for ( auto m=0.5; m<1; ++m )
                for ( auto n=0.5; n<1; ++n )
                {
                    QColor tmpPix = bicubicInterpolate (p, (float)m/1, (float)n/1);
                    dImage.setPixelColor(i/N, j/N, tmpPix);
                }
        }
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
                                            tr("Image (*.jpg *.jpeg *.png *.bmp);;All Files (*)"));

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
        newimg2.save(newName+"x");
    }
}

//Лабораторная работа № 3:
//Реализовать добавление в изображение аддитивного Гауссового шума, а также импульсного шума.
//Реализовать смешивание (blending) двух изображений 8 bpp одинакового размера, используя в качестве альфа-канала третье изображение 8 bpp.

double generator()
{/* Генерация аддитивного белого гауссовского шума с нулевым средним и стандартным отклонением, равным 1. */
    double temp1;
    double temp2;
    double result;
    int p;

    p = 1;

    while( p > 0 )
    {
        temp2 = ( rand() / ( (double)RAND_MAX ) );
        if ( temp2 == 0 )
            p = 1;
        else
            p = -1;
    }
    temp1 = cos( ( 2.0 * (double)PI ) * rand() / ( (double)RAND_MAX ) );
    result = sqrt( -2.0 * log( temp2 ) ) * temp1;

    return result*100;
}

void MainWindow::on_actionAWGN_triggered()
{
    QImage dImage( img.width(), img.height(), QImage::Format_RGB32);

    QColor tmpColor;
    int tmp;
    int r, g, b;

    for ( auto i=0; i<img.width(); ++i )
        for ( auto j=0; j<img.height(); ++j )
        {
            tmp = generator();

            tmpColor = img.pixelColor(i, j);

            r = tmpColor.red() + tmp;
            g = tmpColor.green() + tmp;
            b = tmpColor.blue() + tmp;

            QColor tmpNewColor(clamp(r), clamp(g), clamp(b));
            dImage.setPixelColor(i, j, tmpNewColor);
        }


    newimg = dImage;

    //------------------------------------------------

    ui->label_2->setPixmap(QPixmap::fromImage(newimg));
    ui->label_2->setScaledContents( true );
    ui->label_2->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
}

void MainWindow::on_actionImpulse_Noise_triggered()
{
    double noiseLvl = 0.1;
    QImage dImage = img;
    QColor tmpColor;
    int tmp;
    int r, g, b;

    int noisepixel = img.width()*img.height()*noiseLvl;
    int x,y;// координаты

    //инициализация генератора случайных чисел
    QTime midnight(0,0,0);
    qsrand(midnight.secsTo(QTime::currentTime()));

    for ( auto i=0;i < noisepixel; ++i)
    {
        x=qrand()%dImage.width();//случайная координата х от 0 до width
        y=qrand()%dImage.height();//случайная координата y от 0 до heigth

        tmp = generator();

        r = tmpColor.red() + tmp;
        g = tmpColor.green() + tmp;
        b = tmpColor.blue() + tmp;


        QColor tmpNewColor(clamp(r), clamp(g), clamp(b));
        dImage.setPixelColor(x, y, tmpNewColor);
    }

    newimg = dImage;

    //------------------------------------------------

    ui->label_2->setPixmap(QPixmap::fromImage(newimg));
    ui->label_2->setScaledContents( true );
    ui->label_2->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
}

void MainWindow::on_actionBlending_triggered()
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
        QImage buffer1( fileName );
        newimg = buffer1;
    }
    ui->label_2->setPixmap(QPixmap::fromImage(newimg));
    ui->label_2->setScaledContents( true );
    ui->label_2->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );

    QString fileName2 = QFileDialog::getOpenFileName(this,
                                                     tr("Open Image"), "",
                                                     tr("Image (*.jpg *jpeg *png);;All Files (*)"));

    if (fileName2.isEmpty())
    {
        return;
    }
    else
    {
        QImage buffer2( fileName2 );
        newimg2 = buffer2;
    }
    ui->label_3->setPixmap(QPixmap::fromImage(newimg2));
    ui->label_3->setScaledContents( true );
    ui->label_3->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );

    QImage dImage( img.width(), img.height(), QImage::Format_RGB32);
    QColor tmpColor1, tmpColor2, tmpColor3;
    double r, g, b;
    double r1, g1, b1;
    double r2, g2, b2;
    double r3, g3, b3;

    // TGT_COLOR = TGT_COLOR * (1 - SRC_ALPHA) + SRC_COLOR * SRC_ALPHA

    for ( auto i = 0; i < img.width(); ++i )
        for ( auto j = 0; j < img.height(); ++j )
        {
            tmpColor1 = img.pixelColor(i, j);
            tmpColor2 = newimg.pixelColor(i,j);
            tmpColor3 = newimg2.pixelColor(i,j);

            r1 = tmpColor1.red();
            g1 = tmpColor1.green();
            b1 = tmpColor1.blue();

            r2 = tmpColor2.red();
            g2 = tmpColor2.green();
            b2 = tmpColor2.blue();

            r3 = tmpColor3.red();
            g3 = tmpColor3.green();
            b3 = tmpColor3.blue();


            r = r2 - r2*(256 - r3)/255 + r1 - r3;
            g = g2 - g2*(256 - g3)/255 + g1 - g3;
            b = b2 - b2*(256 - b3)/255 + b1 - b3;


            QColor tmpNewColor(clamp(r), clamp(g), clamp(b));
            dImage.setPixelColor(i, j, tmpNewColor);
        }

    //------------------------------------------------

    ui->label_3->setPixmap(QPixmap::fromImage(dImage));
    ui->label_3->setScaledContents( true );
    ui->label_3->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );

    newimg = dImage;
}

//Лабораторная работа № 4:
//Реализовать повышение глобального контраста изображения по функции преобразования уровней.
void MainWindow::on_actionGlobal_Contrast_triggered()
{
    QImage dImage = img;
    QColor tmpColor;
    int r;
    unsigned long lSum = 0;
    int HT = 30, CT = 180, lT = 180;
    int lLeftLim, lRightLim;
    unsigned long hist[256];
    unsigned long abLUT[256];

    // вычисление гистограммы яркостей
    for ( auto i=0; i<256; ++i ) hist[i] = 0;

    for ( auto i=0; i<img.width(); ++i )
        for ( auto j=0; j<img.height(); ++j )
        {
            tmpColor = img.pixelColor(i, j);
            hist[tmpColor.red()]++;
        }

    // нахождение левой границы для контрастирования
    for (lLeftLim = 0; lLeftLim < 100; lLeftLim++)
    {
        if (hist[lLeftLim] > HT) break;
        lSum += hist[lLeftLim];
        // С0 = С1 = СT
        if (lSum > CT) break;
    }

    // нахождение правой границы для контрастирования
    lSum = 0;
    for (lRightLim = 255; lRightLim > 150; lRightLim--)
    {
        if (hist[lRightLim] > lT) break;
        lSum += hist[lRightLim];
        if (lSum > lT) break;
    }
    // вычисление таблицы перекодировки (LUT, Look-Up Table)
    for (int i = 0; i < lLeftLim; i++) { abLUT[i] = (unsigned char)0; }
    for (int i = lLeftLim; i < lRightLim; i++)
    {
        abLUT[i]=(unsigned char)(255*(i-lLeftLim)/(lRightLim-lLeftLim));
    }
    for (int i = lRightLim; i < 256; i++) { abLUT[i] = (unsigned char)255; }

    //контрастирование
    for (int j = 0; j < img.height(); j++)
        for (int i = 0; i < img.width(); i++)
        {
            tmpColor = img.pixelColor(i, j);
            r = abLUT[tmpColor.red()];
            QColor tmpNewColor(r, r, r);
            dImage.setPixelColor(i, j, tmpNewColor);
        }
    

    newimg = dImage;

    //------------------------------------------------

    ui->label_2->setPixmap(QPixmap::fromImage(newimg));
    ui->label_2->setScaledContents( true );
    ui->label_2->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
}

//Лабораторная работа № 5:
//Реализовать двумерную линейную фильтрацию (свертку) изображения. Ядро фильтра задается пользователем.
void MainWindow::on_actionLinear_Filtration_triggered()
{
    QImage dImage = img;
    QColor tmpColor;
    float r, g, b;
    float N = 1;
    int matrixSize = 3;
    float matrix[matrixSize][matrixSize]=
    {
        {-1, -1, -1},
        {-1,  9, -1},
        {-1, -1, -1},
    };
    int offset = matrixSize / 2;

    for ( auto x = 0; x < img.width(); ++x )
        for ( auto y = 0; y < img.height(); ++y )
        {
            r = 0;
            g = 0;
            b = 0;
            for (int i = 0; i < matrixSize; i++)
                for (int j= 0; j < matrixSize; j++)
                {

                    int xloc = x + i - offset;
                    int yloc = y + j - offset;


                    tmpColor = img.pixelColor(custom_clamp(xloc, 0, img.width()-offset),
                                              custom_clamp(yloc, 0, img.height()-offset));

                    r += tmpColor.red() * matrix[i][j] / N;
                    g += tmpColor.green() * matrix[i][j] / N;
                    b += tmpColor.blue() * matrix[i][j] / N;
                }


            QColor tmpNewColor(clamp(r), clamp(g), clamp(b));
            dImage.setPixelColor(x, y, tmpNewColor);
        }

    newimg = dImage;

    //------------------------------------------------

    ui->label_2->setPixmap(QPixmap::fromImage(newimg));
    ui->label_2->setScaledContents( true );
    ui->label_2->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
}
//Лабораторная работа № 6:
//Реализовать медианную фильтрацию изображения. Апертура фильтра задается пользователем.
void MainWindow::on_actionMediana_triggered()
{
    QImage dImage = img;
    QColor tmpColor;
    int matrixSize = 3;
    float red_array[matrixSize*matrixSize]={0,0,0,0,0,0,0,0,0};
    float green_array[matrixSize*matrixSize]={0,0,0,0,0,0,0,0,0};
    float blue_array[matrixSize*matrixSize]={0,0,0,0,0,0,0,0,0};
    int offset = matrixSize / 2;

    for ( auto x = 0; x < img.width(); ++x )
        for ( auto y = 0; y < img.height(); ++y )
        {
            for (int i = 0; i < matrixSize; i++)
                for (int j= 0; j < matrixSize; j++)
                {

                    int xloc = x + i - offset;
                    int yloc = y + j - offset;


                    tmpColor = img.pixelColor(custom_clamp(xloc, 0, img.width()-offset),
                                              custom_clamp(yloc, 0, img.height()-offset));

                    red_array[i*matrixSize+j] = tmpColor.red();
                    green_array[i*matrixSize+j] = tmpColor.green();
                    blue_array[i*matrixSize+j] = tmpColor.blue();
                }

            std::sort(red_array, red_array+matrixSize*matrixSize);
            std::sort(green_array, green_array+matrixSize*matrixSize);
            std::sort(blue_array, blue_array+matrixSize*matrixSize);

            QColor tmpNewColor(clamp(red_array[5]), clamp(green_array[5]), clamp(blue_array[5]));
            dImage.setPixelColor(x, y, tmpNewColor);
        }

    newimg = dImage;

    //------------------------------------------------

    ui->label_2->setPixmap(QPixmap::fromImage(newimg));
    ui->label_2->setScaledContents( true );
    ui->label_2->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
}


//Лабораторная работа № 7:
//Реализовать бинаризацию изображения по порогу, заданному пользователем, и подсчет количества 4-х связных областей.
void MainWindow::on_actionBinarization_triggered()
{
    QImage dImage = img;
    QColor tmpColor;
    float neutural, radical;
    int bottom_threshold = 140;

    for ( auto x = 0; x < img.width(); ++x )
        for ( auto y = 0; y < img.height(); ++y )
        {
            tmpColor = img.pixelColor(x, y);
            neutural = 0.3*tmpColor.red() + 0.59*tmpColor.green() + 0.11*tmpColor.blue();

            radical = (neutural > bottom_threshold)? 255 : 0;

            QColor tmpNewColor(radical, radical, radical);
            dImage.setPixelColor(x, y, tmpNewColor);
        }

    newimg = dImage;

    int counter = 0;
    int center, left, top;

    for (auto y = 0; y < newimg.height(); ++y)
    {
        for (auto x = 0; x < newimg.width(); ++x)
        {
            if (x - 1 < 0)
                left = 0;
            else
                left = newimg.pixelColor(x-1, y).red();
            if (y - 1 < 0)
                top = 0;
            else
                top = newimg.pixelColor(x, y-1).red();;

            center = newimg.pixelColor(x, y).red();;
            if (center == 255)
            {
                if ((left == 0) && (top == 0))
                    counter++;
            }
        }
    }

    std::cout << "number of 4-connected regions: " << counter << std::endl;

    //------------------------------------------------

    ui->label_2->setPixmap(QPixmap::fromImage(newimg));
    ui->label_2->setScaledContents( true );
    ui->label_2->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
}



//----------------------------------------------------------------------------------------
int MainWindow::find_pix_color(int x, int y, unsigned char ** im)
{
    return ((colors[im[y][x]][0] + colors[im[y][x]][1] + colors[im[y][x]][2]) / 3);
}

int MainWindow::readRLE(FILE * f)
{
    //std::cout << "read rle" << std::endl;

    int i, j;
    int bit1, bit2;

    for (i = 0; i < height; i++)
    {
        for (j = 0; j <= width; j++)
        {
            bit1 = fgetc(f);
            bit2 = fgetc(f);
            if ((bit1 == 0) && (bit2 == 0))
            {
                j = width;
            }
            else if ((bit1 == 0) && (bit2 == 1))
            {
                fclose(f);
                return 1;
            }
            else if ((bit1 == 0) && (bit2 == 2))
            {
                j += fgetc(f);
                i += fgetc(f);
            }
            else if (bit1 == 0)
            {
                for (int k = 0; k < bit2; k++)
                {
                    image[i][j] = fgetc(f);
                    j++;
                }
                j--;
                if (bit2 % 2 == 1)
                {
                    fgetc(f);
                }
            }
            else
            {
                for (int k = 0; k < bit1; k++)
                {
                    image[i][j] = bit2;
                    j++;
                }
                j--;
            }
        }
    }
    fclose(f);
    return 1;
}

int MainWindow::readBMP_Win(FILE * f)
{
    //std::cout << "read bmp win" << std::endl;

    info = (unsigned char *)malloc(sizeof(unsigned char) * 40);
    info[0] = 40;

    int i, j, tmp;
    for (i = 1; i < 40; i++)
    {
        tmp = fgetc(f);
        info[i] = tmp;
    }

    for (i = 0; i < 256; i++)
    {
        for (j = 0; j < 3; j++)
        {
            tmp = fgetc(f);
            colors[i][j] = tmp;
        }
        tmp = fgetc(f);
        RGBreserved[i] = tmp;
    }

    width = info[5] * 256 + info[4];
    height = info[9] * 256 + info[8];
    bits = info[15] * 256 + info[14];

    image = (unsigned char **)malloc(sizeof(unsigned char *) * height);
    for (i = 0; i < height; i++)
        image[i] = (unsigned char *)malloc(sizeof(unsigned char) * width);

    if (info[16] == 1)
    {
        readRLE(f);
        return 1;
    }

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            tmp = fgetc(f);
            image[i][j] = tmp;
        }
        if (width % 4 != 0)
        {
            for (j = 0; j < 4 - width % 4; j++)
                tmp = fgetc(f);
        }
    }
    fclose(f);
    return 1;
}

void MainWindow::readBMP_CORE(FILE * f)
{
    //std::cout << "read bmp core" << std::endl;

    info = (unsigned char *)malloc(sizeof(unsigned char) * 12);
    info[0] = 12;

    int i, j, tmp;
    for (i = 1; i < 12; i++)
    {
        tmp = fgetc(f);
        info[i] = tmp;
    }

    for (i = 0; i < 256; i++)
    {
        for (j = 0; j < 3; j++)
        {
            tmp = fgetc(f);
            colors[i][j] = tmp;
        }
    }

    width = info[5] * 256 + info[4];
    height = info[7] * 256 + info[6];
    bits = info[11] * 256 + info[10];

    image = (unsigned char **)malloc(sizeof(unsigned char *) * height);
    for (i = 0; i < height; i++)
        image[i] = (unsigned char *)malloc(sizeof(unsigned char) * width);
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            tmp = fgetc(f);
            image[i][j] = tmp;
        }
        if (width % 4 != 0)
        {
            for (j = 0; j < 4 - width % 4; j++)
                tmp = fgetc(f);
        }
    }
    fclose(f);
}

int MainWindow::readBMP(QString file_name)
{
    //std::cout << "read bmp" << std::endl;

    int i, j, tmp;
    FILE * f;
    std::string str = file_name.toStdString();
    const char* char_str = str.c_str();
    f = fopen(char_str, "rb");
    if (f == NULL)
    {
        return -1;
    }

    for (i = 0; i < 14; i++)
    {
        tmp = fgetc(f);
        header[i] = tmp;
    }
    header_size = fgetc(f);

    switch (header_size)
    {
    case 12:
        readBMP_CORE(f);
        break;
    case 40:
        readBMP_Win(f);
        break;
    }
    return 0;
}

void MainWindow::writeBMP(QString file_name)
{
    FILE *f_out;
    int i, j;
    std::string str = file_name.toStdString();
    const char* char_str = str.c_str();
    f_out = fopen(char_str, "wb");
    fwrite(header, sizeof(unsigned char), 14, f_out);
    info[16] = 0;
    fwrite(info, sizeof(unsigned char), header_size, f_out);
    for (i = 0; i < 256; i++)
    {
        for (j = 0; j < 3; j++)
        {
            fwrite(&colors[i][j], sizeof(unsigned char), 1, f_out);
        }
        if (header_size == 40)
            fwrite(&RGBreserved[i], sizeof(unsigned char), 1, f_out);
    }
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            fwrite(&image[i][j], sizeof(unsigned char), 1, f_out);
        }
        if (width % 4 != 0)
            fwrite("0", sizeof(unsigned char), 4 - width % 4, f_out);
    }
    for (int i = 0; i < height; i++)
    {
        free(image[i]);
    }
    free(image);
    free(info);
    fclose(f_out);
}

//БДЗ!
//Читает полутоновое изображение (8 bpp) из файла BMP,  выполняет двумерное быстрое преобразование Фурье.
//На экран выводятся исходное изображение и логарифм амплитуд спектра.
//Требуется поддержка следующих особенностей данных форматов:BMP (Windows, OS/2, RLE8); На выход - любой BMP

#define NUMBER_IS_2_POW_K(x) ((!((x)&((x)-1)))&&((x)>1))
#define FT_DIRECT -1 // Direct transform.
#define FT_INVERSE 1 // Inverse transform.

int MainWindow::getClosestLog(const int number)
{
    if(!NUMBER_IS_2_POW_K(number)) {
        int tmp = 1<<((int)(std::log2(number))+1);
        return tmp;
    }
    else {
        return number;
    }
}

bool MainWindow::FFT(double *Rdat, double *Idat, int N, int LogN, int Ft_Flag)
{
    if((Rdat == NULL) || (Idat == NULL))                  return false;
    if((N > 16384) || (N < 1))                            return false;
    if(!NUMBER_IS_2_POW_K(N))                             return false;
    if((LogN < 2) || (LogN > 14))                         return false;
    if((Ft_Flag != FT_DIRECT) && (Ft_Flag != FT_INVERSE)) return false;

    register int  i, j, n, k, io, ie, in, nn;
    double         ru, iu, rtp, itp, rtq, itq, rw, iw, sr;

    static const double Rcoef[14] =
    {  -1.0000000000000000F, 0.0000000000000000F,  0.7071067811865475F,
       0.9238795325112867F,  0.9807852804032304F,  0.9951847266721969F,
       0.9987954562051724F,  0.9996988186962042F,  0.9999247018391445F,
       0.9999811752826011F,  0.9999952938095761F,  0.9999988234517018F,
       0.9999997058628822F,  0.9999999264657178F
    };
    static const double Icoef[14] =
    {   0.0000000000000000F,  -1.0000000000000000F, -0.7071067811865474F,
        -0.3826834323650897F, -0.1950903220161282F, -0.0980171403295606F,
        -0.0490676743274180F, -0.0245412285229122F, -0.0122715382857199F,
        -0.0061358846491544F, -0.0030679567629659F, -0.0015339801862847F,
        -0.0007669903187427F, -0.0003834951875714F
    };

    nn = N >> 1;
    ie = N;
    for(n=1; n<=LogN; n++)
    {
        rw = Rcoef[LogN - n];
        iw = Icoef[LogN - n];
        if(Ft_Flag == FT_INVERSE) iw = -iw;
        in = ie >> 1;
        ru = 1.0F;
        iu = 0.0F;
        for(j=0; j<in; j++)
        {
            for(i=j; i<N; i+=ie)
            {
                io       = i + in;
                rtp      = Rdat[i]  + Rdat[io];
                itp      = Idat[i]  + Idat[io];
                rtq      = Rdat[i]  - Rdat[io];
                itq      = Idat[i]  - Idat[io];
                Rdat[io] = rtq * ru - itq * iu;
                Idat[io] = itq * ru + rtq * iu;
                Rdat[i]  = rtp;
                Idat[i]  = itp;
            }

            sr = ru;
            ru = ru * rw - iu * iw;
            iu = iu * rw + sr * iw;
        }

        ie >>= 1;
    }

    for(j=i=1; i<N; i++)
    {
        if(i < j)
        {
            io       = i - 1;
            in       = j - 1;
            rtp      = Rdat[in];
            itp      = Idat[in];
            Rdat[in] = Rdat[io];
            Idat[in] = Idat[io];
            Rdat[io] = rtp;
            Idat[io] = itp;
        }

        k = nn;

        while(k < j)
        {
            j   = j - k;
            k >>= 1;
        }

        j = j + k;
    }

    if(Ft_Flag == FT_DIRECT) return true;

    rw = 1.0F / N;

    for(i=0; i<N; i++)
    {
        Rdat[i] *= rw;
        Idat[i] *= rw;
    }

    return true;
}

void MainWindow::on_actionBDZ_triggered()
{
    // read img
    fileName = QFileDialog::getOpenFileName(this,
                                            tr("Open Image"), "",
                                            tr("Image (*.jpg *.jpeg *.png *.bmp);;All Files (*)"));

    if (fileName.isEmpty())
    {
        return;
    }
    else
    {
        readBMP(fileName);

        QImage dImage(width, height, QImage::Format_Grayscale8);
        for (auto i = 0; i < height; ++i)
        {
            for (auto j = 0; j < width; ++j)
            {
                QColor tmpNewColor(image[i][j], image[i][j], image[i][j]);
                dImage.setPixelColor(j, height-1-i, tmpNewColor);
            }
        }

        newimg = dImage;

        ui->label->setPixmap(QPixmap::fromImage(newimg));
        //ui->label->setScaledContents( true );
        //ui->label->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
        //--------------------------------------------------------------------------------

        // 2-d FFT
        double fft_image[height][width];
        double fft_image_Im[height][width];
        for (auto i = 0; i < height; ++i)
        {
            for (auto j = 0; j < width; ++j)
            {
                fft_image[i][j] = image[i][j]*pow(-1, i+j);
                fft_image_Im[i][j] = 0;
            }
        }

        int row_len = getClosestLog(width);
        int col_len = getClosestLog(height);

        double* row = (double*)malloc(row_len * sizeof(double));
        double* col = (double*)malloc(col_len * sizeof(double));
        double* Im_row = (double*)malloc(row_len * sizeof(double));
        double* Im_col = (double*)malloc(col_len * sizeof(double));

        for (auto i = 0; i < height; ++i)
        {
            std::fill_n(row, row_len, 0);
            std::fill_n(Im_row, row_len, 0);

            memcpy(row, &(fft_image[i][0]), width*sizeof(double));
            memcpy(Im_row, &(fft_image_Im[i][0]), width*sizeof(double));

            FFT(row, Im_row, row_len, std::log2(row_len), FT_DIRECT);

            memcpy(&(fft_image[i][0]), row, width*sizeof(double));
            memcpy(&(fft_image_Im[i][0]), Im_row, width*sizeof(double));
        }

        for (auto j = 0; j < width; ++j)
        {
            std::fill_n(col, col_len, 0);
            std::fill_n(Im_col, col_len, 0);

            for (auto k = 0; k < height; ++k)
            {
                memcpy(col+k, &(fft_image[k][j]), sizeof(double));
                memcpy(Im_col+k, &(fft_image_Im[k][j]), sizeof(double));
            }

            FFT(col, Im_col, col_len, std::log2(col_len), FT_DIRECT);

            for (auto l = 0; l < height; ++l)
            {
                memcpy(&(fft_image[l][j]), col+l, sizeof(double));
                memcpy(&(fft_image_Im[l][j]), Im_col+l, sizeof(double));
            }
        }

        QImage dImage2(width, height, QImage::Format_Grayscale8);
        for (auto i = 0; i < height; ++i)
        {
            for (auto j = 0; j < width; ++j)
            {
                QColor tmpNewColor(clamp(10*log(abs(fft_image[i][j]))),
                                   clamp(10*log(abs(fft_image[i][j]))),
                                   clamp(10*log(abs(fft_image[i][j]))));
                dImage2.setPixelColor(j, height-1-i, tmpNewColor);
            }
        }

        ui->label_2->setPixmap(QPixmap::fromImage(dImage2));
        //ui->label_2->setScaledContents( true );
        //ui->label_2->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    }

}
