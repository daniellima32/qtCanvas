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

    //Mudando local do release
    QPointF windowPos = viewPortToWindow1({event->x(), event->y()});
    if (!elementsBeeingMoved)
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
                if (!drawingSelection && !controlIsDown) element.isSelected = false;
            }
        }
    }
    drawingSelection = false;
    elementsBeeingMoved = false;

    /*if (someElementWasClicked(windowPos) && !someElementIsSelected())
    {
        dealWithcontextMenuEvent(event);
    }*/

    if (mapOfOrigPosOfMovedElements.size() > 0) mapOfOrigPosOfMovedElements.clear();

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
        //Deve descobrir se deve mover elementos ou fazer pan de window

        //Para mover elementos o último presionar deve ter sido em um nó
        //e algum elemento deve estar selecionado
        if ((someElementWasClicked(lastMouseWindowPosition) || elementsBeeingMoved) &&
                someElementIsSelected())
        {
            //Deve mover elementos
            elementsBeeingMoved = true;
            //Deve salvar as posições originais dos elementos movidos caso não tenham sido salvas ainda
            if (mapOfOrigPosOfMovedElements.size() == 0)
            {
                for (auto &el: elements)
                {
                    if (el.isSelected)
                    {
                        mapOfOrigPosOfMovedElements[el.id] = el.point;
                    }
                }
            }

            //Deve transladar os pontos selecionados
            for (auto &el: elements)
            {
                if (el.isSelected)
                {
                    QPointF translateFactor (windowPos.x() - lastMouseWindowPosition.x(),
                                             windowPos.y() - lastMouseWindowPosition.y());

                    el.point = mapOfOrigPosOfMovedElements[el.id];   //Restaura valor original
                    translatePoint(translateFactor, el.point);       //transladando ponto
                }
            }
        } // fim de mover elementos
        else
        {
            //Fazer pan de window

            QPointF diff = {lastMouseWindowPosition.x() - windowPos.x(), lastMouseWindowPosition.y() - windowPos.y()}; //better
            translateRect(diff, transformation::window);
        }
    }
    else if (event->buttons() == Qt::LeftButton)
    {
        selectedRect.clear();
        //Desenhar rectangulo saindo de lastMouseWindowPosition ate posição atual em window
        selectedRect.push_back(lastMouseWindowPosition);
        selectedRect.push_back(windowPos);

        QPointF origin = windowToViewPort1(selectedRect[0]);
        QPointF final = windowToViewPort1(selectedRect[1]);
        std::vector<QPointF> vec = getRectPoints(origin, final);
        QPoint pLeftTopViewPort = getLeftTop(vec);
        QPoint pRightBottomViewPort = getBottomRight(vec);
        QRect selectionRect = {viewPortToWindow2(pLeftTopViewPort), viewPortToWindow2(pRightBottomViewPort)};
        for (auto &el: elements)
        {
            if (selectionRect.contains(el.point.x(), el.point.y()))
            {
                el.isSelected = true;
            }
            else if (!controlIsDown)
            {
                el.isSelected = false;
            }
        }
        drawingSelection = true;
    }
    this->refreshPixmap();
}

void MainWindow::clickInContextMenu()
{
    int a = 10;
}

void MainWindow::dealWithcontextMenuEvent(QMouseEvent *event)
{
    QMenu menu(this);
    QPointF viewPortPos (event->x(), event->y());
    QPointF windowPos = viewPortToWindow1(viewPortPos);

    //Descobrir se clicou sobre algum elemento
    if (someElementWasClicked(windowPos))
    {
        ElementsData element;
        uint id;
        aquireIDOfClickedElement(windowPos, id);
        aquireElementByID(id, element);

        if (element.type != ElementType::DEMAND)
        {
            QAction* actDemanda = new QAction(tr("&Demanda"), this);
            menu.addAction(actDemanda);
            connect(actDemanda, &QAction::triggered, this, [=](){
                changeElementType(id, ElementType::DEMAND);
            });
        }

        if (element.type != ElementType::RESERVOIR)
        {
            QAction* actReservoir = new QAction(tr("&Reservoir"), this);
            menu.addAction(actReservoir);
            connect(actReservoir, &QAction::triggered, this, [=](){
                changeElementType(id, ElementType::RESERVOIR);
            });
        }

        if (element.type != ElementType::JUNCTION)
        {
            QAction* actJunction = new QAction(tr("&Junction"), this);
            menu.addAction(actJunction);
            connect(actJunction, &QAction::triggered, this, [=](){
                changeElementType(id, ElementType::JUNCTION);
            });
        }

        menu.exec(event->globalPos());
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
    QPointF windowPos = viewPortToWindow1({event->x(), event->y()});
    lastMouseWindowPosition = windowPos;
    if (event->buttons() == Qt::RightButton)
    {
        if (someElementWasClicked(windowPos) && !someElementIsSelected())
        {
            dealWithcontextMenuEvent(event);
        }
    }
    /*else if (event->buttons() == Qt::LeftButton)
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
    }*/
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

    //Desenhar links
    for(auto link: links)
    {
        painter.setBrush(QBrush(Qt::blue, Qt::SolidPattern));

        if (!link.isSelected)
        {
            painter.setPen(Qt::blue);
        }
        else
        {
            QPen pen(Qt::red);
            pen.setWidth(2);
            painter.setPen(pen);
        }

        uint idOrigin = link.origin;
        uint idDestiny = link.destiny;

        ElementsData originElement, destinyElement;
        aquireElementByID(idOrigin, originElement);
        aquireElementByID(idDestiny, destinyElement);

        painter.drawLine(windowToViewPort1(originElement.point),
                         windowToViewPort1(destinyElement.point));
    }

    //Desenhar elementos
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
        else if (el.type == ElementType::JUNCTION)
        {
            painter.setBrush(QBrush(Qt::blue, Qt::SolidPattern));

            if (!el.isSelected)
            {
                painter.setPen(Qt::blue);
            }
            else
            {
                QPen pen(Qt::red);
                pen.setWidth(2);
                painter.setPen(pen);
            }

            painter.drawEllipse (windowToViewPort1({el.point}), 2*radius, 2*radius);
        }
    }

    //Checar se existe retangulo sendo selecionado
    if (selectedRect.size() == 2)
    {
        //Desenhar retângulo
        painter.setBrush(QBrush(Qt::transparent, Qt::SolidPattern));
        painter.setPen(Qt::black);

        QPointF origin = windowToViewPort1(selectedRect[0]);
        QPointF final = windowToViewPort1(selectedRect[1]);

        painter.drawPolygon(getRectPoints(origin, final).data(), 4);
        selectedRect.clear();
    }

    update();
}
