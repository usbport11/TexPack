#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    model = new QStringListModel(this);
    ui->listView->setModel(model);
    ui->listView->installEventFilter(this);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    saveDirectory = qApp->applicationDirPath();
    ui->label_5->setText(saveDirectory);
    ui->edtAlphaLevel->setValidator(new QIntValidator(0, 255, this));
}

MainWindow::~MainWindow() {
    delete ui;
}

bool MainWindow::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Delete) {
            model->removeRow(ui->listView->currentIndex().row());
            ui->label->clear();
            ui->label_2->setText("0 x 0");
        }
        return true;
    }
    else {
        return QObject::eventFilter(object, event);
    }
}

void MainWindow::on_btnFilesOpen_clicked() {
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
    ui->label->setText("Preview Image");
    ui->label_2->setText("0 x 0");
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

void MainWindow::on_btnRemoveColor_clicked() {
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

void MainWindow::on_btnCompactImage_clicked() {
    QPixmap mainPixmap = ui->label->pixmap(Qt::ReturnByValue);
    if(mainPixmap.isNull()) {
        return;
    }

    QImage mainImage(mainPixmap.toImage());
    QString preLevel = ui->edtAlphaLevel->text();
    preLevel.replace(" ", "");
    int level = 50;
    if(preLevel.length() > 0) {
      level = preLevel.toInt();
    }

    //need think about optimize
    int min[2] = {mainImage.width(), mainImage.height()}, max[2] = {0, 0};
    for (int x = 1 ; x < mainImage.width() - 1; x++) {
        for (int y = 1 ; y < mainImage.height() - 1; y++) {
            QColor currentPixel = mainImage.pixelColor(x, y);
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
    ui->label_2->setText(QString::number(size.width()) + " x " + QString::number(size.height()));
}

void MainWindow::on_btnPackImages_clicked() {
    int rowNumber = model->rowCount();
    if(!rowNumber) return;

    std::vector<stPixmapRect> pixmapRects;
    std::vector<QImage> images;
    pixmapRects.reserve(rowNumber);
    images.reserve(rowNumber);

    for(int i=0; i<rowNumber; i++) {
        QImage image(model->index(i).data().toString());
        QSize size = image.size();
        images.push_back(image);
        //pixmapRects.push_back(stPixmapRect(QRect(0, 0, size.width(), size.height()), &images[i]));
        pixmapRects.push_back(stPixmapRect(QRect(0, 0, size.width(), size.height()), &images[i],
            QFileInfo(model->index(i).data().toString()).fileName()));
    }

    QSize resultSize(0, 0);
    std::vector<stPixmapRect> result = packRects2(pixmapRects, resultSize);

    QImage resultImage(QSize(resultSize.width(), resultSize.height()), QImage::Format_RGBA8888);
    QPainter resultPainter(&resultImage);
    for(size_t i=0; i < result.size(); i++) {
        QRect rect = result[i].rect;
        QImage image = *result[i].pImage;
        resultPainter.fillRect(rect, Qt::transparent);
        resultPainter.setCompositionMode(QPainter::CompositionMode_Source);
        resultPainter.drawImage(rect, image);
    }
    QPixmap mainPixmap = QPixmap::fromImage(resultImage);
    ui->label->setPixmap(mainPixmap);
    ui->label_2->setText(QString::number(resultSize.width()) + " x " + QString::number(resultSize.height()));

    //export png
    QString fullFileName;
    fullFileName = saveDirectory + "/"+ ui->edtOutFilename->text() + ".png";
    QFile pngFile(fullFileName);
    pngFile.open(QIODevice::WriteOnly);
    mainPixmap.save(&pngFile, "PNG");

    QString prefixName;

    //export plist
    fullFileName = saveDirectory + "/"+ ui->edtOutFilename->text() + ".plist";
    QFile plistFfile(fullFileName);
    if (plistFfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&plistFfile);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE plist>\n<plist version=\"1.0\">\n";
        out << "\t<dict>\n\t\t<key>frames</key>\n\t\t<dict>\n";
        for(size_t i=0; i < result.size(); i++) {
            if(ui->edtKeyPrefix->isEnabled()) {
                prefixName = ui->edtKeyPrefix->text();
                if(prefixName.length() <= 0) {
                    prefixName = "pt";
                }
                prefixName += QString::number(i);
            }
            else {
                prefixName = result[i].filename;
                prefixName = prefixName.left(prefixName.lastIndexOf("."));
            }
            out << "\t\t\t<key>"<< prefixName << "</key>\n";
            out << "\t\t\t<dict>\n";
            out << "\t\t\t\t<key>aliases</key>\n";
            out << "\t\t\t\t<array/>\n";
            out << "\t\t\t\t<key>spriteOffset</key>\n";
            out << "\t\t\t\t<string>{0,0}</string>\n";
            out << "\t\t\t\t<key>spriteSize</key>\n";
            out << "\t\t\t\t<string>{" << result[i].rect.width() << "," << result[i].rect.height() << "}</string>\n";
            out << "\t\t\t\t<key>spriteSourceSize</key>\n";
            out << "\t\t\t\t<string>{" << result[i].rect.width() << "," << result[i].rect.height() << "}</string>\n";
            out << "\t\t\t\t<key>textureRect</key>\n";
            out << "\t\t\t\t<string>{{" << result[i].rect.x() << "," << result[i].rect.y() << "},{" << result[i].rect.width() << "," << result[i].rect.height() << "}}</string>\n";
            out << "\t\t\t\t<key>textureRotated</key>\n";
            out << "\t\t\t\t<false/>\n";
            out << "\t\t\t\t<key>triangles</key>\n";
            //may change
            out << "\t\t\t\t<string>1 2 3 0 1 3</string>\n";
            out << "\t\t\t\t<key>vertices</key>\n";
            //may change
            out << "\t\t\t\t<string>" << result[i].rect.width() << " " << result[i].rect.height() << " " <<
                0 << " " << result[i].rect.height() << " " <<
                0 << " " << 0 << " " <<
                result[i].rect.width() << " " << 0 <<
                "</string>\n";
            out << "\t\t\t\t<key>verticesUV</key>\n";
            //may change
            out << "\t\t\t\t<string>" << result[i].rect.x() + result[i].rect.width() << " " << result[i].rect.y() + result[i].rect.height() << " " <<
                result[i].rect.x() << " " << result[i].rect.y() + result[i].rect.height() << " " <<
                result[i].rect.x() << " " << result[i].rect.y() << " " <<
                result[i].rect.x() + result[i].rect.width() << " " << result[i].rect.y() <<
                "</string>\n";
            out << "\t\t\t</dict>\n";
        }
        out << "\t\t</dict>\n";
        out << "\t\t<key>metadata</key>\n";
        out << "\t\t<dict>\n";
        out << "\t\t\t<key>format</key>\n";
        out << "\t\t\t<integer>3</integer>\n";
        out << "\t\t\t<key>pixelFormat</key>\n";
        //may change
        out << "\t\t\t<string>RGBA8888</string>\n";
        out << "\t\t\t<key>premultiplyAlpha</key>\n";
        out << "\t\t\t<false/>\n";
        out << "\t\t\t<key>realTextureFileName</key>\n";
        out << "\t\t\t<string>" << ui->edtOutFilename->text() << ".png" << "</string>\n";
        out << "\t\t\t<key>size</key>\n";
        out << "\t\t\t<string>{" << resultSize.width() << "," << resultSize.height() << "}</string>\n";
        out << "\t\t\t<key>textureFileName</key>\n";
        out << "\t\t\t<string>" << ui->edtOutFilename->text() << ".png</string>\n";
        out << "\t\t</dict>\n";
        out << "\t</dict>\n</plist>";
    }

    pixmapRects.clear();
    images.clear();
    result.clear();
}

std::vector<stPixmapRect> MainWindow::packRects2(std::vector<stPixmapRect> rects, QSize& size) {
    std::vector<stPixmapRect> packed;
    if(rects.empty()) {
        return packed;
    }

    QSize resultSize(0, 0);
    int area = 0;
    int maxWidth = 0;
    for(size_t i = 0; i < rects.size(); i++) {
        area += rects[i].rect.width() * rects[i].rect.height();
        maxWidth = std::max(maxWidth, rects[i].rect.width());
    }

    struct {
        bool operator()(stPixmapRect a, stPixmapRect b) const {
            return a.rect.height() > b.rect.height();
        }
    } heightLess;
    std::sort(rects.begin(), rects.end(), heightLess);

    int startWidth = std::max(qCeil(qSqrt(area / 0.95)), maxWidth); //optimal width?
    std::vector<QRect> spaces;
    spaces.push_back(QRect(0, 0, startWidth, INT_MAX));

    for(size_t i = 0; i < rects.size(); i++) {
        for(size_t j = 0; j < spaces.size(); j++) {
            if(rects[i].rect.width() > spaces[j].width() || rects[i].rect.height() > spaces[j].height()) {
                continue;
            }

            //add rect
            stPixmapRect pixmapRect(QRect(spaces[j].x(), spaces[j].y(), rects[i].rect.width(), rects[i].rect.height()), rects[i].pImage, rects[i].filename);
            packed.push_back(pixmapRect);
            resultSize.setWidth(std::max(resultSize.width(), pixmapRect.rect.x() + pixmapRect.rect.width()));
            resultSize.setHeight(std::max(resultSize.height(), pixmapRect.rect.y() + pixmapRect.rect.height()));

            //split space
            QRect lastSpace = spaces[j];
            spaces.erase(spaces.begin() + j);
            //left
            spaces.push_back(QRect(lastSpace.x() + pixmapRect.rect.width(),
                                   lastSpace.y(),
                                   lastSpace.width() - pixmapRect.rect.width(),
                                   pixmapRect.rect.height()));
            //bottom
            spaces.push_back(QRect(lastSpace.x(),
                                   lastSpace.y() + pixmapRect.rect.height(),
                                   lastSpace.width(),
                                   lastSpace.height() - pixmapRect.rect.height()));
            break;
        }
    }
    spaces.clear();
    size = resultSize;

    return packed;
}

void MainWindow::on_btnSelectDirectory_clicked() {
    saveDirectory = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "C:\\", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->label_5->setText(saveDirectory);
}

void MainWindow::on_btnReset_clicked() {
    model->removeRows(0, model->rowCount());
    ui->label->clear();
    ui->label->setText("Preview Image");
    ui->label_2->setText("0 x 0");
}

void MainWindow::on_rbtPref_prefix_clicked() {
    ui->edtKeyPrefix->setEnabled(true);
}

void MainWindow::on_rbtPref_filename_clicked() {
    ui->edtKeyPrefix->setEnabled(false);
}
