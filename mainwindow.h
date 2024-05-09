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
#include <QBuffer>
#include <QtMath>
#include <QTextStream>
#include <QKeyEvent>
#include <vector>
#include <algorithm>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct stPixmapRect {
    QRect rect;//holding new coordinates after pack
    QImage image;
    QString filename;
    stPixmapRect() {
        rect = QRect(0,0,0,0);
        image = QImage();
        filename = "";
    }
    stPixmapRect(QRect inRect, const QImage& inImage) {
        rect = inRect;
        image = inImage;
    }
    stPixmapRect(QRect inRect, const QImage& inImage, QString inFilename) {
        rect = inRect;
        image = inImage;
        filename = inFilename;
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    bool eventFilter(QObject* object, QEvent* event) override;
    void on_btnFilesOpen_clicked();
    void on_listView_clicked(const QModelIndex& index);
    void on_btnRemoveColor_clicked();
    void on_btnCompactImage_clicked();
    void on_btnPackImages_clicked();
    void on_btnSelectDirectory_clicked();
    void on_btnReset_clicked();
    void on_rbtPref_prefix_clicked();
    void on_rbtPref_filename_clicked();

private:
    Ui::MainWindow* ui;
    QStringListModel *model;
    QString saveDirectory;
    std::vector<stPixmapRect> pixmapRects;
    std::vector<stPixmapRect> packRects2(std::vector<stPixmapRect> rects, QSize& size);
};
#endif // MAINWINDOW_H
