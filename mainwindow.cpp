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

    for (int i = 1; i < size; i++ )
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

    newimg = img; //because we don't need tograyscale method

//    //create grayscale palette
//    int iMaxColors = 256; //flexible
//    int t;
//    QColor *palette = new QColor [iMaxColors];
//            for ( t=0; t<iMaxColors; t++)	{
//                float fGreyLevel =  (float)t/(float)iMaxColors * 255;
//                palette[t] = QColor ((int)fGreyLevel, (int)fGreyLevel, (int)fGreyLevel);
//            }
//    int size = iMaxColors;
//    //

    //create black&white palette
    int iMaxColors = 255; //flexible
    int t;
    QColor *palette = new QColor [iMaxColors];
            for ( t=0; t<iMaxColors/2; t++)	{
                float fGreyLevel =  0;
                palette[t] = QColor ((int)fGreyLevel, (int)fGreyLevel, (int)fGreyLevel);
            }
            for ( t; t<iMaxColors; t++)	{
                float fGreyLevel =  255;
                palette[t] = QColor ((int)fGreyLevel, (int)fGreyLevel, (int)fGreyLevel);
            }
    int size = iMaxColors;
    //

    //grayscaling and dithering
    QImage dImage( newimg.width(), newimg.height(), QImage::Format_Indexed8); //for 8, not rgb's 32 bits per pix

    int i;
    dImage.setColorCount(size);
    for ( i = 0; i < size; i++ )
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

    for ( int j = 0; j < newimg.height(); j++ )
    {
        //let's go deeper to pix'es bits and evaluating errors
        uint *ip = (uint * )newimg.scanLine( j );
        uchar *dp = dImage.scanLine( j );

        for ( i = 0; i < newimg.width(); i++ )
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

        for ( i = 1; i < newimg.width()-1; i++ )
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
        tr("Image (*.jpg *.jpeg *.png *.bmp);;All Files (*)") );

        //type only name of file for saving
        newimg.save(newName + ".jpg");
    }
}
