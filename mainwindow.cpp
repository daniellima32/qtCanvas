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

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control)
    {
        controlIsDown = true;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control)
    {
        controlIsDown = false;
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    selectedRect.clear(); //Apaga a indicação de possível região selecionada
    this->refreshPixmap();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    this->refreshPixmap();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QPointF windowPos = viewPortToWindow1({event->x(), event->y()});
    if (event->buttons() == Qt::RightButton)
    {
        QPointF diff = {lastMouseWindowPosition.x() - windowPos.x(), lastMouseWindowPosition.y() - windowPos.y()}; //better
        translateRect(diff, transformation::window);
    }
    else if (event->buttons() == Qt::LeftButton)
    {
        selectedRect.clear();
        //Desenhar rectangulo saindo de lastMouseWindowPosition ate posição atual em window
        selectedRect.push_back(lastMouseWindowPosition);
        selectedRect.push_back(windowPos);
    }
    this->refreshPixmap();
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
    QPointF windowPos = viewPortToWindow1({event->x(), event->y()});
    lastMouseWindowPosition = windowPos;
    if (event->buttons() == Qt::RightButton)
    {
        //lastMouseWindowPosition = windowPos;
    }
    else if (event->buttons() == Qt::LeftButton)
    {
        for (auto &element : elements)
        {
            if (isClickedInElement(element.point, windowPos))
            {
                element.isSelected = !element.isSelected;
            }
            else
            {
                //se control não está pressionado
                if (!controlIsDown) element.isSelected = false;
            }
        }
    }
    this->refreshPixmap();
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

    //Checar se existe retangulo sendo selecionado
    if (selectedRect.size() == 2)
    {
        //Desenhar retângulo
        painter.setBrush(QBrush(Qt::transparent, Qt::SolidPattern));
        painter.setPen(Qt::black);

        //origin tem que ter o menorx e maiory

        QPointF origin = windowToViewPort1(selectedRect[0]);
        QPointF final = windowToViewPort1(selectedRect[1]);

        painter.drawPolygon(getRectPoints(origin, final).data(), 4);
    }

    update();
}
