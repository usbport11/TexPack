#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    model = new QStringListModel(this);
    ui->listView->setModel(model);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_pushButton_clicked() {
    QStringList files = QFileDialog::getOpenFileNames(this, "Select one or more files to open", "/home", "Images (*.png)");
    if(files.empty()) return;
    model->removeRows(0, model->rowCount());
    int row;
    for(auto it = files.begin(); it != files.end(); it++) {
        row = model->rowCount();
        model->insertRows(row, 1);
        QModelIndex index = model->index(row);
        model->setData(index, it->toLocal8Bit().constData());
    }

    ui->label->clear();
}

void MainWindow::on_listView_clicked(const QModelIndex &index) {
    ui->label->clear();

    QImage mainImage(index.data().toString());
    QPixmap mainPixmap = QPixmap::fromImage(mainImage);
    QSize size = mainPixmap.size();

    ui->label_2->setText(QString::number(size.width()) + " x "+ QString::number(size.height()));

    if(ui->label->width() <= size.width() || ui->label->height() <= size.height()) {
        float ratio = 0;
        if(size.width() > size.height()) {
            ratio = (float)ui->label->width() / size.width();
        }
        else {
            ratio = (float)ui->label->height() / size.height();
        }
        ui->label->setPixmap(mainPixmap.scaled(size.width() * ratio, size.height() * ratio, Qt::KeepAspectRatio));
    }
    else {
        ui->label->setPixmap(mainPixmap);
    }
}

void MainWindow::on_pushButton_2_clicked() {
    QPixmap mainPixmap = ui->label->pixmap(Qt::ReturnByValue);
    if(mainPixmap.isNull()) {
        return;
    }

    QImage mainImage(mainPixmap.toImage());

    QBitmap colorMask = mainPixmap.createMaskFromColor(mainImage.pixel(0,0), Qt::MaskOutColor);
    QImage maskImage(mainPixmap.size(), QImage::Format_RGBA8888);
    QPainter maskPainter(&maskImage);
    maskPainter.setPen(QColor(255, 255, 255));
    maskPainter.drawPixmap(mainPixmap.rect(), colorMask, colorMask.rect());
    maskPainter.end();
    QPixmap maskPixmap = QPixmap::fromImage(maskImage);

    QImage resultImage(mainPixmap.size(), QImage::Format_RGBA8888);
    QPainter resultPainter(&resultImage);
    resultPainter.setClipRegion(QRegion(maskPixmap));
    resultPainter.drawPixmap(0, 0, mainPixmap);
    resultPainter.end();
    mainPixmap = QPixmap::fromImage(resultImage);

    ui->label->setPixmap(mainPixmap);
}

void MainWindow::on_pushButton_3_clicked() {
    QPixmap mainPixmap = ui->label->pixmap(Qt::ReturnByValue);
    if(mainPixmap.isNull()) {
        return;
    }

    QImage mainImage(mainPixmap.toImage());
    int level = 50;

    //need think about optimize
    int min[2] = {mainImage.width(), mainImage.height()}, max[2] = {0, 0};
    for (int x = 1 ; x < mainImage.width() - 1; x++) {
        for (int y = 1 ; y < mainImage.height() - 1; y++) {
            QColor currentPixel = (mainImage.pixelColor(x, y));
            if (currentPixel.alpha() > level) {
                if(min[0] > x) min[0] = x;
                if(min[1] > y) min[1] = y;
                if(max[0] < x) max[0] = x;
                if(max[1] < y) max[1] = y;
            }
        }
    }

    QRect rect(min[0], min[1], max[0], max[1]);
    mainImage = mainImage.copy(rect);
    mainPixmap = QPixmap::fromImage(mainImage);
    QSize size = mainPixmap.size();

    ui->label->setPixmap(mainPixmap);
    ui->label_2->setText(QString::number(size.width()) + " x "+ QString::number(size.height()));
}

std::vector<QRect> MainWindow::Pack(std::vector<QRect> rects) {
    std::vector<QRect> packed;
    if(rects.empty()) {
        return packed;
    }

    int area = 0;
    int maxWidth = 0;
    for(int i = 0; i < rects.size(); i++) {
        //area = rects[i].width() * rects[i].height();
        area *= rects[i].width() * rects[i].height();
        maxWidth = std::max(maxWidth, rects[i].width());
    }

    struct {
        bool operator()(QRect a, QRect b) const {
            return a.height() > b.height();
        }
    } heightLess;
    std::sort(rects.begin(), rects.end(), heightLess);

    int startWidth = std::max(qCeil(qSqrt(area / 0.95)), maxWidth);
    std::vector<QRect> spaces;
    spaces.push_back(QRect(0, 0, startWidth, INT_MAX));

    for(int i = 0; i < rects.size(); i++) {
        for(int j = spaces.size() - 1; j >= 0; j--) {
            QRect space = spaces[j];
            if(rects[i].width() > space.width() || rects[i].height() > space.height()) {
                continue;
            }
            QRect rect = QRect(space.x(), space.y(), rects[i].width(), rects[i].height());
            packed.push_back(rect);
            if(rects[i].width() == space.width() && rects[i].height() == space.height()) {
                QRect last = spaces.back();
                spaces.pop_back();
                if (j < spaces.size()) spaces[j] = last;
            }
            else if(rects[i].height() == space.height()) {
                space.setX(space.x() + rects[i].width());
                space.setWidth(space.width() - rects[i].width());
            }
            else if(rects[i].width() == space.width()) {
                space.setY(space.y() + rects[i].height());
                space.setHeight(space.height() - rects[i].height());
            }
            else {
                spaces.push_back(QRect(space.x() + rects[i].width(), space.y(), space.width() - rects[i].width(), rects[i].height()));
            }
            space.setY(space.y() + rects[i].height());
            space.setHeight(space.height() - rects[i].height());
        }
        //break;
    }

    QFile file("E:\\QtProjects\\test1\\out.txt");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            for(int i = 0; i < packed.size(); i++) {
                out << packed[i].x() << packed[i].y() << packed[i].width() << packed[i].height() << "\n";
            }
        }

    return packed;
}

void MainWindow::on_pushButton_4_clicked() {
    std::vector<QRect> testRects;
    testRects.push_back(QRect(0,0,100,200));
    testRects.push_back(QRect(0,0,50,50));
    testRects.push_back(QRect(0,0,150,50));
    testRects.push_back(QRect(0,0,200,100));
    testRects.push_back(QRect(0,0,100,100));
    std::vector<QRect> result = Pack(testRects);

    QImage resultImage(QSize(800, 800), QImage::Format_RGBA8888);
    QPainter resultPainter(&resultImage);
    for(int i=0; i < result.size(); i++) {
        resultPainter.setPen(i+1);
        resultPainter.drawRect(result[i]);
    }
    QPixmap mainPixmap = QPixmap::fromImage(resultImage);
    ui->label->setPixmap(mainPixmap);
    ui->label_2->setText(QString::number(result.size()));
}
