#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QStringListModel>
#include <QMessageBox>
#include <QTextCodec>
#include <QPainter>
#include <QImage>
#include <QBitmap>
#include <QtMath>
#include <QTextStream>
#include <vector>
#include <algorithm>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct stPixmapRect {
    QRect rect;
    QImage* pImage;
    stPixmapRect() {
        rect = QRect(0,0,0,0);
        pImage = nullptr;
    }
    stPixmapRect(QRect inRect, QImage* inImage) {
        rect = inRect;
        pImage = inImage;
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnFilesOpen_clicked();
    void on_listView_clicked(const QModelIndex &index);
    void on_btnRemoveColor_clicked();
    void on_btnCompactImage_clicked();
    void on_btnPackImages_clicked();
    std::vector<stPixmapRect> packRects2(std::vector<stPixmapRect> rects);

private:
    Ui::MainWindow* ui;
    QStringListModel *model;
};
#endif // MAINWINDOW_H
