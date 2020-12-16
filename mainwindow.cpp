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

    QPoint p1(-1,-1), p2(-1,-1);
    for (int x = 0 ; x < mainImage.width(); x++) {
        for (int y = 0 ; y < mainImage.height(); y++) {
            if(p1.x() != -1 && p1.y() != -1 && p2.x() != -1 && p2.y() != -1) {
                break;
            }
            if(p1.x() == -1 || p1.y() == -1) {
                QColor currentPixel = (mainImage.pixelColor(x, y));
                if (currentPixel.alpha() > level) {
                    if(p1.x() == -1) p1.setX(x);
                    if(p1.y() == -1) p1.setY(y);
                }
            }
            if(p2.x() == -1 || p2.y() == -1) {
                QColor currentPixel = (mainImage.pixelColor(mainImage.width() - x, mainImage.height() - y));
                if (currentPixel.alpha() > level) {
                    if(p2.x() == -1) p2.setX(mainImage.width() - x);
                    if(p2.y() == -1) p2.setY(mainImage.height() - y);
                }
            }
        }
    }
    QMessageBox msgBox;
    msgBox.setText("(" + QString::number(p1.x()) + ", "+ QString::number(p1.x()) + ") (" + QString::number(p2.x()) + ", "+ QString::number(p2.x())+ ")");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}
