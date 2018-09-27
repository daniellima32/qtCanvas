#include "coordinatesmanip.h"
#include "NumericalUtils.h"

CoordinatesManip::CoordinatesManip()
{

}

double CoordinatesManip::norma(QPointF entry)
{
    return sqrt(entry.x()*entry.x() + entry.y()*entry.y());
}

QPointF CoordinatesManip::normalize(QPointF point)
{
    double n = norma(point);
    QPointF ret = {point.x()/n, point.y()/n};
    return ret;
}

QPointF CoordinatesManip::product(QPointF point, int n)
{
    QPointF ret = {point.x() * n, point.y() * n};
    return ret;
}

std::vector<QPointF> CoordinatesManip::getArrowPoints(QLineF line)
{
    std::vector<QPointF> vec;
    QPointF p0, p1, p2;

    // Encontra o centro do trecho
    //wxPoint center = line.getPoint(0.5); // Encontra o centro do trecho
    QPointF center = {(line.p1().x() + line.p2().x())/2,
                     (line.p1().y() + line.p2().y())/2};

    //Vector2D v0(line.end.x - line.start.x, line.end.y - line.start.y);
    QPointF ponto1 = line.p1();
    QPointF ponto2 = line.p2();
    QPointF v0 (ponto2.x() - ponto1.x(), ponto2.y() - ponto1.y());

    //Dependendo do denominador, não pode fazer a normalização
    if (NumericalUtils::compareDoubles(v0.x(), 0) && NumericalUtils::compareDoubles(v0.x(), 0))
        return vec;

    v0 = normalize(v0);

    //v0 = v0 * 5; // vetor na direção da seta
    v0 = product(v0, 5);

    //Vector2D v1(-v0.y(), v0.x());
    //Vector2D v2(v0.y(), -v0.x());
    QPointF v1 (-v0.y(), v0.x());
    QPointF v2 (v0.y(), -v0.x());


    p0 = {center.x() + v0.x() * 2, center.y() + v0.y() * 2};
    p1 = {center.x() + v1.x(), center.y() + v1.y()};
    p2 = {center.x() + v2.x(), center.y() + v2.y()};

    double xMin = p0.x(), yMin = p0.y();
    double xMax = p0.x(), yMax = p0.y();

    vec.push_back(p0);
    vec.push_back(p1);
    vec.push_back(p2);

    for (size_t i = 0; i < 3; ++i)
    {
        //if (p[i].x < xMin)
        if (vec[i].x() < xMin)
        {
            xMin = vec[i].x();
        }
        else if (vec[i].x() >= xMax)
        {
            xMax = vec[i].x();
        }
        if (vec[i].y() < yMin)
        {
            yMin = vec[i].y();
        }
        else if (vec[i].y() >= yMax)
        {
            yMax = vec[i].y();
        }
    }

    return vec;
}
