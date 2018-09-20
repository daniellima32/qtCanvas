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

void MainWindow::resizeEvent(QResizeEvent* event)
{
    this->refreshPixmap();
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


    for(auto el: elements)
    {
        //Se for demanda
        if(el.type == ElementType::DEMAND)
        {
            painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));

            if (!el.isSelected)
            {
                painter.setPen(Qt::red);
            }
            else
            {
                QPen pen(Qt::black);
                pen.setWidth(2);
                painter.setPen(pen);
            }
            painter.drawConvexPolygon(getDemandPoints(windowToViewPort1(el.point)).data(), 4);
        }
        else if (el.type == ElementType::RESERVOIR)
        {
            painter.setBrush(QBrush(QColor(20, 170, 255), Qt::SolidPattern));

            if (!el.isSelected)
            {
                painter.setPen(QColor(20, 170, 255)); //cor azul claro
            }
            else
            {
                QPen pen(Qt::red);
                pen.setWidth(2);
                painter.setPen(pen);
            }
            painter.drawConvexPolygon(getReservoirPoints(windowToViewPort1({el.point})).data(), 3);
        }
    }

    update();
}
