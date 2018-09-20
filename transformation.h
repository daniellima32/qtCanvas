#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <QPointF>

struct Rect
{
    QPointF point;
    double width;
    double height;
    Rect(QPointF point, double width, double height):
        point(point), width(width), height(height){}
};

namespace transformation
{
    Rect world = {QPointF{-180.0, 90.0}, 360.0, 180.0};
    Rect window = world;
    Rect viewPort = {QPointF{0.0, 0.0}, 800.0, 600.0};
}

using namespace transformation;

double r = 10;

std::vector<QPointF> getReservoirPoints(QPointF point)
{
    std::vector<QPointF>points =
    {
        QPointF(point.x(), point.y()-r),   //p1
        QPointF(point.x()+r, point.y()+r),   //p2
        QPointF(point.x()-r, point.y()+r)  //p3
    };

    return points;
}

std::vector<QPointF> getDemandPoints(QPointF point)
{
    std::vector<QPointF>points =
    {
        QPointF(point.x(), point.y()-r),   //p1
        QPointF(point.x()+r, point.y()),   //p2
        QPointF(point.x(), point.y()+r),  //p3
        QPointF(point.x()-r, point.y())  //p4
    };

    return points;
}

QPointF windowToViewPort1(QPointF windowPoint)
{
    QPointF view;
    view.setX(((viewPort.width * (windowPoint.x() - window.point.x())) +  (viewPort.point.x() * window.width)) / window.width);
    view.setY(((viewPort.height * (window.point.y() - windowPoint.y()))  + (viewPort.point.y() * window.height)) / window.height);
    return view;
}

QPointF viewPortToWindow1(QPointF viewPoint)
{
    QPointF windowPoint;
    windowPoint.setX((((viewPoint.x() - viewPort.point.x()) * window.width) + viewPort.width * window.point.x()) / viewPort.width);
    windowPoint.setY(((viewPort.height * window.point.y()) - ((viewPoint.y() - viewPort.point.y()) * window.height)) / viewPort.height);
    return windowPoint;
}

void scaleRect(QPointF scaleFactor, Rect& rect)
{
    rect.point.setX(rect.point.x() * scaleFactor.x());
    rect.point.setY(rect.point.y() * scaleFactor.y());
    rect.width = rect.width * scaleFactor.x();
    rect.height = rect.height * scaleFactor.y();
}

void scalePoint(QPointF scaleFactor, QPointF& point)
{
    point.setX(point.x() * scaleFactor.x());
    point.setY(point.y() * scaleFactor.y());
}

void translateRect(QPointF translateFactor, Rect& rect)
{
    rect.point.setX(rect.point.x() + translateFactor.x());
    rect.point.setY(rect.point.y() + translateFactor.y());
}

void translatePoint(QPointF translateFactor, QPointF& point)
{
    point.setX(point.x() + translateFactor.x());
    point.setY(point.y() + translateFactor.y());
}

void zoom(float zoomFactor, double x, double y)
{
    QPointF viewPortPoint (x, y);
    QPointF windowPoint = viewPortToWindow1(viewPortPoint);

    //Deve levar window para a origem
    translateRect({-windowPoint.x(),
                  -windowPoint.y()},
                  window);

    //Fazer scale em window
    scaleRect({zoomFactor,zoomFactor}, window);
    //window.point.setX();

    //Deve levar window de volta para (x,y) (x,y é posição de tela)
    translateRect({+windowPoint.x(),
                 +windowPoint.y()},
                  window);


}

#endif // TRANSFORMATION_H
