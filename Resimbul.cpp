#include "Resimbul.h"
#include "ui_Resimbul.h"
#include <opencv2/opencv.hpp>
#include <QFileDialog>
#include <QMessageBox>
using namespace cv;

typedef struct imagedata{
    QString* filename;
    int *histogram;
    int distance;
}Imagedata;

void (*histfunc)(Mat*,int*);
int histSize;

Mat ReadImage(QString path);
void _64binHistogram(Mat *img,int *histogram);//64 bin histogram yöntemine göre histogram dizisini oluşturur
void LBPhistogram(Mat *img,int *histogram);//LBP yöntemine göre histogram dizisini oluşturur.
void Hog(Mat* img,int* histogram);
int findDistance(int *hist,int *histogram);//verilen iki histogram arasındaki öklid mesafesini bulur.
void sortArray(Imagedata *data,int size,int towhere);
void search(QString sex,QString category,QString filename,Imagedata *retarr);
QStringList listCategory(QString sex);
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QDir dir("Database/");
    QStringList filelist = dir.entryList();
    filelist.pop_front();
    filelist.pop_front();
    ui->sexCombo->addItems(filelist);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    //Gözat tıklandı
    QString path = QFileDialog::getOpenFileName(this,"Dosya Seç","Database/","*");
    ui->fileedit->setText(path);

}

void MainWindow::on_pushButton_2_clicked()
{
    //Ara tıklandı
    if(ui->colorcheck->isChecked()){
        histfunc=_64binHistogram;
        histSize=64;

    }else if(ui->texturecheck->isChecked()){
        histfunc=LBPhistogram;
        histSize=256;
    }else if(ui->shapecheck->isChecked()){
        histfunc=Hog;
        histSize=18;
    }
    Imagedata *ret;
    int i;
    if(!ui->categoryCombo->currentText().compare(QString::fromUtf8("Tümü"))){
        QStringList categories = listCategory(ui->sexCombo->currentText());
        int count = categories.count();
        ret = (Imagedata*)calloc(count*5,sizeof(Imagedata));
        for(i=0;i<count;i++){
            search(ui->sexCombo->currentText(),categories.at(i),ui->fileedit->text(),&ret[i*5]);
        }
        sortArray(ret,count*5,5);
    }
    else{
        ret = (Imagedata*)calloc(5,sizeof(Imagedata));
        search(ui->sexCombo->currentText(),ui->categoryCombo->currentText(),ui->fileedit->text(),ret);
    }
    for(i=0;i<5;i++){
        QFileInfo info(*ret[i].filename);
        QString winname = info.fileName() +" "+ QString::number(i);
        imshow(winname.toStdString(),ReadImage(*ret[i].filename));
    }
    cvWaitKey(0);
    cvDestroyAllWindows();
}

void MainWindow::on_sexCombo_currentIndexChanged(const QString &arg1)
{
    //Cinsiyet seçildi
    ui->categoryCombo->clear();
    QStringList categories = listCategory(ui->sexCombo->currentText());
    categories.push_front(QString::fromUtf8("Tümü"));
    ui->categoryCombo->addItems(categories);
}
QStringList listCategory(QString sex){
    QDir dir("Database/"+sex+"/");
    QStringList filelist = dir.entryList();
    filelist.pop_front();
    filelist.pop_front();
    return filelist;
}

void MainWindow::on_categoryCombo_currentIndexChanged(const QString &arg1)
{
    //Kategori seçildi
}
void search(QString sex,QString category,QString filename,Imagedata *retarr){
    QDir dir( "Database/"+sex+"/"+category+"/");
    QStringList filelist = dir.entryList();
    filelist.pop_front();
    filelist.pop_front();
    Mat searching = ReadImage(filename);
    resize(searching,searching,Size(293,409));
    int height = searching.rows/3;
    int width = searching.cols/3;
    Rect rect(width,height,width,height);
    searching = searching(rect);
    Imagedata searchingdata;
    searchingdata.filename = &filename;
    searchingdata.histogram = (int*)calloc(histSize,sizeof(int));
    histfunc(&searching,searchingdata.histogram);

    int filelistsize = filelist.count();
    Imagedata data[filelistsize];
    int i;
    for(i=0;i<filelistsize;i++){
        data[i].filename = new QString(dir.absoluteFilePath(filelist.at(i)));
        data[i].histogram = (int*)calloc(histSize,sizeof(int));
        Mat temp = ReadImage(*data[i].filename)(rect);
        histfunc(&temp,data[i].histogram);
        data[i].distance = findDistance(searchingdata.histogram,data[i].histogram);
        temp.~Mat();
    }
    sortArray(data,filelistsize,6);
    for(i=0;i<5;i++){
        retarr[i]=data[i];
    }
    return;
}


void _64binHistogram(Mat *img,int *histogram){
    int i,j;
    int val;
    for(i=0;i<img->rows;i++){
        for(j=0;j<img->cols;j++){
            val= 0;
            Vec3b pixel = img->at<Vec3b>(i,j);
            val += pixel.val[2]>>6;//Kırmızıdan alınan iki bit eklenir.
            val = val<<2;//iki bit sola kaydırılır.
            val += (pixel.val[1]>>6);//Yeşilden iki bit eklenir.
            val = val<<2;
            val += (pixel.val[0]>>6);//Maviden iki bit eklenir.
            histogram[val]++;//histogram dizisinde oluşan değer artılır.
        }
    }
    return;
}
void LBPhistogram(Mat *img,int *histogram){
    Mat out;
    cvtColor(*img,out,CV_BGR2GRAY);//gri resme çevrilir.
    int i,j,k;
    uchar val;
    int change;
    int last;
    uchar pixel;
    for(i=1;i<img->rows-1;i++){
        for(j=1;j<img->cols-1;j++){
            val=0;
            change = 0;
            last=2;
            pixel = out.at<uchar>(i,j);
            val += (out.at<uchar>(i-1,j-1) > pixel) << 1;//her pixelin etrafında
            val += (out.at<uchar>(i-1,j  ) > pixel) << 2;//saat yönünde dolanarak
            val += (out.at<uchar>(i-1,j+1) > pixel) << 3;//karşılaştırma yapılır.
            val += (out.at<uchar>(i  ,j+1) > pixel) << 4;//Karşılaştırılan değer
            val += (out.at<uchar>(i+1,j+1) > pixel) << 5;//pikselin değerinde büyükse
            val += (out.at<uchar>(i+1,j  ) > pixel) << 6;//1 yazılır, değilse 0
            val += (out.at<uchar>(i+1,j-1) > pixel) << 7;//yazılır ve sola kaydırılır.
            val += (out.at<uchar>(i  ,j-1) > pixel) << 8;
            for(k=0;k<8;k++){
                if((val>>k)%2 == 0){//oluşan değerde 1-0, 0-1 geçiş sayısı
                    if(last==1)     //hesaplanır.
                        change++;
                    last=0;
                }else{
                    if(last==0)
                        change++;
                    last=1;
                }
            }
            if(change>2)//2'den büyükse sabit bir değere yazılır.
                val = 172;
            histogram[val]++;
        }
    }
    return;
}
void Hog(Mat* img,int* histogram){
   Mat out;
   blur(*img,out,Size(3,3));
   out.convertTo(out, CV_32F, 1/255.0);
   Mat gx, gy;
   Sobel(out, gx, CV_32F, 1, 0, 1);
   Sobel(out, gy, CV_32F, 0, 1, 1);
   Mat mag,angle;
   int i,j;
   cartToPolar(gx, gy, mag, angle, 1);
   for(i=0;i<angle.rows;i++){
       for(j=0;j<angle.cols;j++){
           int pixel = angle.at<Vec3b>(i,j).val[0];
           int index = pixel<180 ? pixel: abs((-pixel)%180);
           int val = (20-index%20);
           histogram[index/20]+=val;
           histogram[index/20+1]+=(20-val);
       }
   }
   out.~Mat();
   return;
}
int findDistance(int *hist,int *histogram){
    int i;
    float distance=0;
    for(i=0;i<histSize;i++){
        distance+= pow(hist[i]-histogram[i],2);//iki histogramın mesafesi hesaplanır.
    }
    return (int)sqrt(distance);
}

Mat ReadImage(QString path){
    QFile file(path);
    std::vector<char> buffer;
    buffer.resize(file.size());
    if (!file.open(QIODevice::ReadOnly)) {
        return cv::Mat();
    }
    file.read(buffer.data(), file.size());
    file.close();
    cv::Mat image = cv::imdecode(buffer, CV_LOAD_IMAGE_COLOR);
    return image;
}
void sortArray(Imagedata *data,int size,int towhere){
    int minindex,i,j;
    Imagedata temp;
    for(i=0;i<towhere;i++){
        minindex = i;
        for(j=i+1;j<size;j++){
            if(data[j].distance<data[minindex].distance){
                minindex = j;
            }
        }
        temp = data[i];//mesafelere göre sıralama işlemi
        data[i] = data[minindex];
        data[minindex] = temp;
    }
    return;
}
