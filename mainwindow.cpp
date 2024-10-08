#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    model = new QStringListModel(this);
    ui->lstvwSourceFiles->setModel(model);
    ui->lstvwSourceFiles->installEventFilter(this);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    ui->edtAlphaLevel->setValidator(new QIntValidator(0, 255, this));
    ui->edtCropWidth->setValidator(new QIntValidator(0, 255, this));
    ui->edtCropHeight->setValidator(new QIntValidator(0, 255, this));
    ui->edtColumnNumber->setValidator(new QIntValidator(0, 255, this));

    loadSettings();
}

MainWindow::~MainWindow() {
    saveSettings();

    if(ui) delete ui;
    if(model) delete model;
    pixmapRects.clear();
}

bool MainWindow::eventFilter(QObject* object, QEvent* event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Delete) {
            if(model->rowCount() <= 0) {
                return false;
            }
            int index = ui->lstvwSourceFiles->currentIndex().row();
            pixmapRects.erase(pixmapRects.begin() + index);
            model->removeRow(index);
            if(model->rowCount() > 0) {
              index = ui->lstvwSourceFiles->currentIndex().row();
              ui->lblPreview->setPixmap(QPixmap::fromImage(pixmapRects[index].image));
              ui->lblSize->setText(QString::number(pixmapRects[index].rect.width()) + " x " + QString::number(pixmapRects[index].rect.height()));
            }
            else {
              ui->lblPreview->clear();
              ui->lblSize->setText("0 x 0");
            }
        }
        if (keyEvent->key() == 'R') {
            if(model->rowCount() <= 0) {
                return false;
            }
            int index = ui->lstvwSourceFiles->currentIndex().row();
            pixmapRects[index].image = QImage(model->index(index).data().toString());
            ui->lblPreview->setPixmap(QPixmap::fromImage(pixmapRects[index].image));
            ui->lblSize->setText(QString::number(pixmapRects[index].rect.width()) + " x " + QString::number(pixmapRects[index].rect.height()));
        }
        return true;
    }
    else {
        return QObject::eventFilter(object, event);
    }
}

void MainWindow::on_btnFilesOpen_clicked() {
    QStringList files = QFileDialog::getOpenFileNames(this, "Select one or more files to open", "/home", "Images (*.png)");
    int row;
    for(auto it = files.begin(); it != files.end(); it++) {
        row = model->rowCount();
        model->insertRows(row, 1);
        QModelIndex index = model->index(row);
        model->setData(index, it->toLocal8Bit().constData());
        QImage image(index.data().toString());
        QSize size = image.size();
        pixmapRects.push_back(stPixmapRect(QRect(0, 0, size.width(), size.height()), image,
            QFileInfo(model->index(row).data().toString()).fileName()));
    }
}

void MainWindow::on_btnRemoveColor_clicked() {
    int index = ui->lstvwSourceFiles->currentIndex().row();
    if(model->rowCount() <= 0 || index < 0) {
        return;
    }
    QImage& image = pixmapRects[index].image;
    QPixmap maskPixmap(QPixmap::fromImage(image));
    if(maskPixmap.isNull()) {
        return;
    }
    maskPixmap.setMask(maskPixmap.createHeuristicMask());
    image = QImage(maskPixmap.toImage());

    ui->lblPreview->setPixmap(maskPixmap);
}

void MainWindow::on_btnCompactImage_clicked() {
    int index = ui->lstvwSourceFiles->currentIndex().row();
    if(model->rowCount() <= 0 || index < 0) {
        return;
    }
    QImage& image = pixmapRects[index].image;
    QPixmap mainPixmap(QPixmap::fromImage(image));
    if(mainPixmap.isNull()) {
        return;
    }

    QString preLevel = ui->edtAlphaLevel->text();
    int level = 50;
    if(preLevel.length() > 0) {
      level = preLevel.toInt();
    }

    //detect new rect
    int min[2] = {image.width(), image.height()}, max[2] = {0, 0};
    for (int x = 1 ; x < image.width() - 1; x++) {
        for (int y = 1 ; y < image.height() - 1; y++) {
            QColor currentPixel = image.pixelColor(x, y);
            if (currentPixel.alpha() > level) {
                if(min[0] > x) min[0] = x;
                if(min[1] > y) min[1] = y;
                if(max[0] < x) max[0] = x;
                if(max[1] < y) max[1] = y;
            }
        }
    }

    QRect rect(min[0], min[1], max[0], max[1]);
    image = image.copy(rect);

    QPixmap newPixmap = QPixmap::fromImage(image);
    QSize size = newPixmap.size();

    ui->lblPreview->setPixmap(newPixmap);
    ui->lblSize->setText(QString::number(size.width()) + " x " + QString::number(size.height()));
}

QString MainWindow::createResultPngFile(std::vector<stPixmapRect>& result, QSize resultSize) {
    QString fullFileName = "";
    QImage resultImage(QSize(resultSize.width(), resultSize.height()), QImage::Format_RGBA8888);
    QPainter resultPainter(&resultImage);
    for(size_t i=0; i < result.size(); i++) {
        QRect rect = result[i].rect;
        resultPainter.fillRect(rect, Qt::transparent);
        resultPainter.setCompositionMode(QPainter::CompositionMode_Source);
        resultPainter.drawImage(rect, result[i].image);
    }
    QPixmap mainPixmap = QPixmap::fromImage(resultImage);
    ui->lblPreview->setPixmap(mainPixmap);
    ui->lblSize->setText(QString::number(resultSize.width()) + " x " + QString::number(resultSize.height()));

    QString outFile = ui->edtOutFilename->text().simplified();
    if(outFile.length() <= 0) {
        outFile = "out";
    }
    fullFileName = ui->edtDirectoryPath->text() + "/" + outFile + ".png";
    QFile pngFile(fullFileName);
    pngFile.open(QIODevice::WriteOnly);
    mainPixmap.save(&pngFile, "PNG");
    return fullFileName;
}

void MainWindow::createPlistFile(std::vector<stPixmapRect>& result, QSize resultSize, QString pngFilename) {
    if(pngFilename.length() <= 0) {
        return;
    }

    QString prefixName;
    QString outFile = ui->edtOutFilename->text().simplified();
    if(outFile.length() <= 0) {
        outFile = "out";
    }
    QString fullFileName = ui->edtDirectoryPath->text() + "/"+ outFile + ".plist";
    QFile plistFfile(fullFileName);
    if (!plistFfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream out(&plistFfile);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE plist>\n<plist version=\"1.0\">\n";
    out << "\t<dict>\n\t\t<key>frames</key>\n\t\t<dict>\n";
    for(size_t i=0; i < result.size(); i++) {
        if(ui->edtKeyPrefix->isEnabled()) {
            prefixName = ui->edtKeyPrefix->text().simplified();
            if(prefixName.length() <= 0) {
                prefixName = "pt";
            }
            prefixName += QString::number(i);
        }
        else {
            if(result[i].filename.length() > 0) {
                prefixName = result[i].filename;
                int last = prefixName.lastIndexOf(".");
                if(last != -1) {
                    prefixName = prefixName.left(last);
                }
            }
            else {
                prefixName = "pt";
                prefixName += QString::number(i);
            }
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
    out << "\t\t\t<string>" << pngFilename << "</string>\n";
    out << "\t\t\t<key>size</key>\n";
    out << "\t\t\t<string>{" << resultSize.width() << "," << resultSize.height() << "}</string>\n";
    out << "\t\t\t<key>textureFileName</key>\n";
    out << "\t\t\t<string>" << pngFilename << "</string>\n";
    out << "\t\t</dict>\n";
    out << "\t</dict>\n</plist>";
}

//save func
void MainWindow::on_btnPackImages_clicked() {
    int rowNumber = model->rowCount();
    if(!rowNumber) return;

    std::vector<stPixmapRect> result;
    QSize resultSize(0, 0);
    QRadioButton* rbt = ui->grpbPackType->findChild<QRadioButton*>("rbtPackTypeCompact");
    if(rbt) {
        if(rbt->isChecked()) {
            result = packRectsCompact(pixmapRects, resultSize);
        }
        else {
            QString strColumns = ui->edtColumnNumber->text();
            int columns = 0;
            if(strColumns.length() > 0) {
                columns = strColumns.toInt();
            }
            result = packRectsByOrder(pixmapRects, resultSize, columns);
        }
    }

    createPlistFile(result, resultSize, createResultPngFile(result, resultSize));

    result.clear();
}

std::vector<stPixmapRect> MainWindow::packRectsCompact(std::vector<stPixmapRect> rects, QSize& size) {
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

    int startWidth = std::max(qCeil(qSqrt(area / 0.95)), maxWidth);//maxWidth alway smaller?
    std::vector<QRect> spaces;
    spaces.push_back(QRect(0, 0, startWidth, INT_MAX));

    for(size_t i = 0; i < rects.size(); i++) {
        for(size_t j = 0; j < spaces.size(); j++) {
            if(rects[i].rect.width() > spaces[j].width() || rects[i].rect.height() > spaces[j].height()) {
                continue;
            }

            //add rect
            stPixmapRect pixmapRect(QRect(spaces[j].x(), spaces[j].y(), rects[i].rect.width(), rects[i].rect.height()), rects[i].image, rects[i].filename);
            packed.push_back(pixmapRect);
            resultSize.setWidth(std::max(resultSize.width(), pixmapRect.rect.x() + pixmapRect.rect.width()));
            resultSize.setHeight(std::max(resultSize.height(), pixmapRect.rect.y() + pixmapRect.rect.height()));

            //split space (need control order to push)
            QRect lastSpace = spaces[j];
            spaces.erase(spaces.begin() + j);
            spaces.insert(spaces.begin() + j, QRect(lastSpace.x() + pixmapRect.rect.width(),
                lastSpace.y(),
                lastSpace.width() - pixmapRect.rect.width(),
                pixmapRect.rect.height()));
            spaces.insert(spaces.begin() + j + 1, QRect(lastSpace.x(),
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

std::vector<stPixmapRect> MainWindow::packRectsByOrder(std::vector<stPixmapRect> inRects, QSize& resultSize, int cellsNumber) {
  std::vector<stPixmapRect> packed;
  if(inRects.empty()) {
    return packed;
  }

  QSize offset(0, 0);
  int currentCell = 0;
  int maxY = 0;

  for(size_t i=0; i<inRects.size(); i++) {
    stPixmapRect nextRect = stPixmapRect(QRect(inRects[i].rect.x() + offset.width(),
      inRects[i].rect.y() + offset.height(),
      inRects[i].rect.width(),
      inRects[i].rect.height()),
      inRects[i].image,
      inRects[i].filename);
    QString info = QString::number(nextRect.rect.x()) + " " +
      QString::number(nextRect.rect.x()) + " " +
      QString::number(nextRect.rect.width()) + " " +
      QString::number(nextRect.rect.height());
    packed.push_back(nextRect);
    offset.setWidth(nextRect.rect.x() + nextRect.rect.width());
    maxY = std::max(maxY, nextRect.rect.y() + nextRect.rect.height());
    resultSize.setWidth(std::max(resultSize.width(), nextRect.rect.x() + nextRect.rect.width()));
    resultSize.setHeight(std::max(resultSize.height(), nextRect.rect.y() + nextRect.rect.height()));
    currentCell ++;
    if(currentCell >= cellsNumber) {
      offset.setWidth(0);
      offset.setHeight(std::max(offset.height(), maxY));
      currentCell = 0;
    }
  }

  return packed;
}

std::vector<stPixmapRect> MainWindow::packFromAtlas(QImage& image) {
    //here ideal atlas sitaution (offset exist and it same everywhere)
    //need add support of manual values (offset, size, count)
    //v1 - no spaces
    //v2 - spaces only between sprites
    //v21 - spaces between not equal
    //v3 - spaces every where
    //v31 - spaces on edges and between not equal
    //values (countX, countY, edgeX, edgeY, offsetX, offsetY, witdh, height)
    std::vector<stPixmapRect> packed;

    QPoint offset = QPoint(0, 0);
    QSize size = QSize(0, 0);
    bool end = false;
    int count[2];

    if(image.rect().width() <= 0 || image.rect().height() <= 0) {
      return packed;
    }

    QString preLevel = ui->edtAlphaLevel->text();
    int level = 50;
    if(preLevel.length() > 0) {
      level = preLevel.toInt();
    }

    QString data;
    //left top of first image
    for (int x = 0; x < image.width(); x++) {
        for (int y = 0; y < image.height(); y++) {
            if (image.pixelColor(x, y).alpha() > level) {
                offset.setX(x);
                offset.setY(y);
                end = true;
                break;
            }
        }
        if(end) break;
    }

    //right of first image
    for (int x = offset.x(); x < image.width(); x++) {
        if (image.pixelColor(x, offset.y()).alpha() < level) {
            size.setWidth(x - offset.x());
            break;
        }
    }

    //bottom of first image
    for (int y = offset.y(); y < image.height(); y++) {
        if (image.pixelColor(offset.x(), y).alpha() < level) {
            size.setHeight(y - offset.y());
            break;
        }
    }

    count[0] = image.width() / (offset.x() + size.width());
    count[1] = image.height() / (offset.y() + size.height());

    for(int i=0; i<count[0]; i++) {
        for(int j=0; j<count[1]; j++) {
            stPixmapRect nextRect = stPixmapRect(QRect(offset.x() * (i + 1)  + size.width() * i,
                offset.y() * (j + 1) + size.height() * j,
                size.width(), size.height()), image, "");
            packed.push_back(nextRect);
        }
    }

    return packed;
}

bool pointInRects(const QPoint& point, const std::vector<stPixmapRect>& rects) {
    for(size_t i=0; i<rects.size(); i++) {
        if(rects[i].rect.contains(point)) {
            return true;
        }
    }
    return false;
}

void MainWindow::packFromAtlas2(QImage& image, std::vector<stPixmapRect>& rects) {
    if(image.rect().width() <= 0 || image.rect().height() <= 0) {
      return;
    }

    QString preLevel = ui->edtAlphaLevel->text();
    int level = 50;
    if(preLevel.length() > 0) {
      level = preLevel.toInt();
    }

    QRect rect = QRect(0, 0, 0, 0);

    //left top image
    bool end = false;
    for (int x = 0; x < image.width(); x++) {
        for (int y = 0; y < image.height(); y++) {
            if (image.pixelColor(x, y).alpha() > level && !pointInRects(QPoint(x, y), rects)) {
                rect.setX(x);
                rect.setY(y);
                rect.setWidth(1);
                rect.setHeight(1);
                end = true;
                break;
            }
        }
        if(end) break;
    }
    if(!end) {
        return;
    }

    //right of image
    for (int x = rect.x(); x < image.width(); x++) {
        if (image.pixelColor(x, rect.y()).alpha() < level) {
            rect.setWidth(x - rect.x());
            break;
        }
    }

    //bottom of mage
    for (int y = rect.y(); y < image.height(); y++) {
        if (image.pixelColor(rect.x(), y).alpha() < level) {
            rect.setHeight(y - rect.y());
            break;
        }
    }

    if(end) {
        rects.push_back(stPixmapRect(rect, image, ""));
        packFromAtlas2(image, rects);
    }
}

void MainWindow::on_btnSelectDirectory_clicked() {
    ui->edtDirectoryPath->setText(QFileDialog::getExistingDirectory(this, tr("Open Directory"), "C:\\", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
}

void MainWindow::on_btnReset_clicked() {
    ui->lblPreview->clear();
    ui->lblPreview->setText("Preview Image");
    ui->lblSize->setText("0 x 0");
    model->removeRows(0, model->rowCount());
    pixmapRects.clear();
}

void MainWindow::on_rbtPref_prefix_clicked() {
    ui->edtKeyPrefix->setEnabled(true);
}

void MainWindow::on_rbtPref_filename_clicked() {
    ui->edtKeyPrefix->setEnabled(false);
}

void MainWindow::on_btnCrop_clicked() {
    int index = ui->lstvwSourceFiles->currentIndex().row();
    if(model->rowCount() <= 0 || index < 0) {
        return;
    }
    QImage& image = pixmapRects[index].image;
    QPixmap mainPixmap(QPixmap::fromImage(image));
    if(mainPixmap.isNull()) {
        return;
    }

    QString strVal;
    int cropWidth;
    int cropHeight;

    strVal = ui->edtCropWidth->text();
    cropWidth = 0;
    if(strVal.length() > 0) {
        cropWidth = strVal.toInt();
    }

    strVal = ui->edtCropHeight->text();
    cropHeight = 0;
    if(strVal.length() > 0) {
        cropHeight = strVal.toInt();
    }

    if(cropWidth * 2 >= image.rect().width() || cropHeight * 2 >= image.rect().height()) {
        return;
    }

    QRect rect(image.rect().x() + cropWidth, image.rect().y() + cropHeight,
               image.rect().width() - cropWidth*2, image.rect().height() - cropHeight*2);
    image = image.copy(rect);
    QMessageBox box;
    box.setText(QString::number(rect.x()) + " " +
                QString::number(rect.y()) + " " +
                QString::number(rect.width()) + " " +
                QString::number(rect.height()));
    box.exec();

    QPixmap newPixmap = QPixmap::fromImage(image);
    QSize size = newPixmap.size();

    ui->lblPreview->setPixmap(newPixmap);
    ui->lblSize->setText(QString::number(size.width()) + " x " + QString::number(size.height()));
}

void MainWindow::on_lstvwSourceFiles_clicked(const QModelIndex &index) {
    ui->lblPreview->clear();

    QPixmap mainPixmap = QPixmap::fromImage(pixmapRects[index.row()].image);
    QSize size = mainPixmap.size();

    ui->lblSize->setText(QString::number(size.width()) + " x "+ QString::number(size.height()));

    if(ui->lblPreview->width() <= size.width() || ui->lblPreview->height() <= size.height()) {
        float ratio = 0;
        if(size.width() > size.height()) {
            ratio = (float)ui->lblPreview->width() / size.width();
        }
        else {
            ratio = (float)ui->lblPreview->height() / size.height();
        }
        ui->lblPreview->setPixmap(mainPixmap.scaled(size.width() * ratio, size.height() * ratio, Qt::KeepAspectRatio));
    }
    else {
        ui->lblPreview->setPixmap(mainPixmap);
    }
}

void MainWindow::loadSettings() {
    QSettings settings(qApp->applicationDirPath() + "/settings.conf", QSettings::IniFormat);
    settings.beginGroup("main");
    ui->edtOutFilename->setText(settings.value("outFile", "out").toString());
    ui->edtDirectoryPath->setText(settings.value("saveDirectory", qApp->applicationDirPath()).toString());
    ui->edtKeyPrefix->setText(settings.value("keyPrefix", "pt").toString());
    ui->edtKeyPrefix->setEnabled(settings.value("prefixEnable", true).toBool());
    ui->edtAlphaLevel->setText(settings.value("alphaLevel", "50").toString());
    ui->edtCropWidth->setText(settings.value("cropWidth", "0").toString());
    ui->edtCropHeight->setText(settings.value("cropHeight", "0").toString());
    ui->edtColumnNumber->setText(settings.value("columnNumber", "0").toString());
    settings.endGroup();
}

void MainWindow::saveSettings() {
    QSettings settings(qApp->applicationDirPath() + "/settings.conf", QSettings::IniFormat);
    settings.beginGroup("main");
    settings.setValue("outFile", ui->edtOutFilename->text().simplified());
    settings.setValue("saveDirectory", ui->edtDirectoryPath->text());
    settings.setValue("keyPrefix", ui->edtKeyPrefix->text().simplified());
    settings.setValue("prefixEnable", ui->edtKeyPrefix->isEnabled());
    settings.setValue("alphaLevel", ui->edtAlphaLevel->text());
    settings.setValue("cropWidth", ui->edtCropWidth->text());
    settings.setValue("cropHeight", ui->edtCropHeight->text());
    settings.setValue("columnNumber", ui->edtColumnNumber->text());
    settings.endGroup();
}

void MainWindow::on_btnPackFromAtlas_clicked() {
    int row = ui->lstvwSourceFiles->currentIndex().row();
    if(model->rowCount() <= 0 || row < 0) {
        return;
    }
    QModelIndex index = model->index(row);
    QString outFile = ui->edtOutFilename->text().simplified();
    if(outFile.length() <= 0) {
        outFile = "out";
    }
    QString fullFileName = ui->edtDirectoryPath->text() + "/" + outFile + ".png";
    QFile sourceFile(index.data().toString());
    sourceFile.copy(fullFileName);
    QImage& image = pixmapRects[row].image;
    std::vector<stPixmapRect> result;
    packFromAtlas2(image, result);
    //std::vector<stPixmapRect> result = packFromAtlas(image);
    createPlistFile(result, pixmapRects[row].image.size(), fullFileName);
    result.clear();
}

void MainWindow::on_rbtPackTypeCompact_clicked() {
    ui->edtColumnNumber->setEnabled(false);
}

void MainWindow::on_rbtPackTypeOrder_clicked() {
    ui->edtColumnNumber->setEnabled(true);
}
