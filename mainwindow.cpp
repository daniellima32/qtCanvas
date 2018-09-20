#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include "transformation.h"
#include <QMouseEvent>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->refreshPixmap();
}



void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(event->rect(), this->pixmap);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)

{

}
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::RightButton)
    {
        QPointF windowPos = viewPortToWindow1({event->x(), event->y()});
        QPointF diff = {lastMouseWindowPosition.x() - windowPos.x(), lastMouseWindowPosition.y() - windowPos.y()}; //better
        translateRect(diff, transformation::window);
        this->refreshPixmap();
    }
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    static float zoomIn = 0.9;
    static float zoomOut = 1.1;
    if (event->angleDelta().y() > 0)
    {
        zoom(zoomIn, event->x(), event->y());
    }
    else
    {
        zoom(zoomOut, event->x(), event->y());
    }
    this->refreshPixmap();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::RightButton)
    {
        QPointF windowPos = viewPortToWindow1({event->x(), event->y()});
        lastMouseWindowPosition = windowPos;
    }
    else if (event->buttons() == Qt::LeftButton)
    {
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshPixmap()
{
    pixmap = QPixmap(size());
    pixmap.fill(Qt::white);

    QPainter painter(&pixmap);
    painter.initFrom(this);

   //Pontos da demanda
   const QPointF points[4] =
   {
       windowToViewPort1(QPointF(-150, 30)),
       windowToViewPort1(QPointF(-130, 40)),
       windowToViewPort1(QPointF(-150, 50)),
       windowToViewPort1(QPointF(-170, 40))
   };

   //Desenhando demanda
   painter.setPen(Qt::red);
   painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));
   //painter.drawConvexPolygon(points, 4);
   painter.drawConvexPolygon(getDemandPoints(windowToViewPort1({-130, 40})).data(), 4);

   //Desenhando reservoir
   painter.setBrush(QBrush(QColor(20, 170, 255), Qt::SolidPattern));
   painter.setPen(QColor(20, 170, 255));
   //painter.drawConvexPolygon(points2, 3);
   painter.drawConvexPolygon(getReservoirPoints(windowToViewPort1({0, 40})).data(), 3);


    update();
}
