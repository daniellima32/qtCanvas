#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    ~MainWindow();
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void refreshPixmap();
    void resizeEvent(QResizeEvent*);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void dealWithcontextMenuEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
private:
    Ui::MainWindow *ui;
    bool controlIsDown = false;
    bool drawingSelection = false;
    bool elementsBeeingMoved = false;
    bool temporaryElementInserted = false;
    QPixmap pixmap;
    QPointF lastMouseWindowPosition = {0.0, 0.0};
    std::vector<QPointF> selectedRect;

    bool labelOfElementBeeingChanged = false;
    bool labelOfLinkBeeingChanged = false;
    uint idOfElementOwnerOfLabel;
    uint idOfLinkOwnerOfLabel;
    uint idLabel;
    QPointF labelDiffBackup;
};

#endif // MAINWINDOW_H
