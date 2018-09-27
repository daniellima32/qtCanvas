#ifndef COORDINATESMANIP_H
#define COORDINATESMANIP_H

#include <QPointF>
#include <QLineF>

class CoordinatesManip
{
public:
    CoordinatesManip();

    static std::vector<QPointF> getArrowPoints(QLineF line);
    static QPointF normalize(QPointF point);
    static QPointF product(QPointF point, int n);
    static double norma(QPointF entry);
};

#endif // COORDINATESMANIP_H
