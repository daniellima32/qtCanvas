#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <QPointF>
#include <QRect>

enum ElementType
{
    DEMAND, RESERVOIR, JUNCTION, LINK
};

struct ElementsData
{
    uint id;                //ID que descreve esse elemento
    QPointF point;          //Posição GPS do nó
    ElementType type;       //Tipo do elemento
    bool isSelected;        //Indicação se está selecionado ou não
};

struct LinkData
{
    uint id;                //ID que descreve esse elemento
    uint origin;            //id do nó origem
    uint destiny;           //id do nó destino
    bool isSelected;        //Indicação se está selecionado ou não
};

#define MPOINT2POINT(mpt, pt)   ((pt).x = (mpt).x, (pt).y = (mpt).y)
#define POINT2MPOINT(pt, mpt)   ((mpt).x = (SHORT)(pt).x, (mpt).y = (SHORT)(pt).y)
#define POINTS2VECTOR2D(pt0, pt1, vect) ((vect).x = (double)((pt1).x - (pt0).x), \
                                         (vect).y = (double)((pt1).y - (pt0).y))

typedef struct tagPOINT
{
    long  x;
    long  y;
} POINT, *PPOINT, *LPPOINT;

typedef struct tagVECTOR2D
{

    double     x;
    double     y;

} VECTOR2D, *PVECTOR2D;

typedef struct tagPROJECTION
{

    VECTOR2D   ttProjection;
    VECTOR2D   ttPerpProjection;
    double     LenProjection;
    double     LenPerpProjection;

} PROJECTION, *PPROJECTION;

double vDotProduct(PVECTOR2D v0, PVECTOR2D v1)
{
    double dotprod;
    dotprod = (v0 == NULL || v1 == NULL)
              ? 0.0
              : (v0->x * v1->x) + (v0->y * v1->y);
    return(dotprod);
}

PVECTOR2D vSubtractVectors(PVECTOR2D v0,
                           PVECTOR2D v1, PVECTOR2D v)
{
    if(v0 == NULL || v1 == NULL)
    {
        v = (PVECTOR2D)NULL;
    }
    else
    {
        v->x = v0->x - v1->x;
        v->y = v0->y - v1->y;
    }

    return(v);
}

double   vVectorSquared(PVECTOR2D v0)
{
    double dSqLen;

    if(v0 == NULL)
    {
        dSqLen = 0.0;
    }
    else
    {
        dSqLen = (double)(v0->x * v0->x) + (double)(v0->y * v0->y);
    }

    return (dSqLen);
}

double vVectorMagnitude(PVECTOR2D v0)
{
    double dMagnitude;

    if(v0 == NULL)
    {
        dMagnitude = 0.0;
    }
    else
    {
        dMagnitude = sqrt(vVectorSquared(v0));
    }

    return (dMagnitude);
}

void vProjectAndResolve(PVECTOR2D v0, PVECTOR2D v1, PPROJECTION ppProj)
{
    VECTOR2D ttProjection, ttOrthogonal;
    double proj1;
    //
    //obtain projection vector
    //
    //c = a * b
    //    ----- b
    //    |b|^2
    //
    proj1 = vDotProduct(v0, v1) / vDotProduct(v1, v1);
    ttProjection.x = v1->x * proj1;
    ttProjection.y = v1->y * proj1;
    //
    //obtain perpendicular projection : e = a - c
    //
    vSubtractVectors(v0, &ttProjection,
                     &ttOrthogonal);
    //
    //fill PROJECTION structure with appropriate values
    //
    ppProj->LenProjection = vVectorMagnitude(
                                &ttProjection);
    ppProj->LenPerpProjection = vVectorMagnitude(
                                    &ttOrthogonal);
    ppProj->ttProjection.x = ttProjection.x;
    ppProj->ttProjection.y = ttProjection.y;
    ppProj->ttPerpProjection.x = ttOrthogonal.x;
    ppProj->ttPerpProjection.y = ttOrthogonal.y;
}

double vDistFromPointToLine(LPPOINT pt0, LPPOINT pt1, LPPOINT ptTest)
{
    VECTOR2D ttLine, ttTest;
    PROJECTION pProjection;
    POINTS2VECTOR2D(*pt0, *pt1, ttLine);
    POINTS2VECTOR2D(*pt0, *ptTest, ttTest);
    vProjectAndResolve(&ttTest, &ttLine,
                       &pProjection);
    return(pProjection.LenPerpProjection);
}

bool HitTestLine(POINT pt0, POINT pt1,
                 POINT ptMouse, int nWidth)
{
    POINT PtM;
    VECTOR2D tt0, tt1;
    double dist;
    int nHalfWidth;
    //
    //Get the half width of the line to adjust for hit testing of wide lines.
    //
    nHalfWidth = (nWidth / 2 < 1) ? 1 : nWidth / 2;
    //
    //Convert the line into a vector using the two endpoints.
    //
    POINTS2VECTOR2D(pt0, pt1, tt0);
    //
    //Convert the mouse points (short) into a POINT structure (long).
    //
    MPOINT2POINT(ptMouse , PtM);
    //
    //Convert the line from the left endpoint to the mouse point into a vector.
    //
    POINTS2VECTOR2D(pt0, PtM, tt1);
    //
    //Obtain the distance of the point from the line.
    //
    dist = vDistFromPointToLine(&pt0, &pt1, &PtM);
    //
    //Return TRUE if the distance of the point from the line is within the width
    //of the line
    //
    return (dist >= -nHalfWidth
            && dist <= nHalfWidth);
}


bool isPointOfLink(const QPointF &linkOrigin,
                   const QPointF &linkDestiny,
                   const QPointF &point)
{
    //Verificar se link está no quadrilátero definido pelos pontos
    //Isso faz com que se elimine da verificação os pontos externos
    QRectF rect (linkOrigin, linkDestiny);
    if (rect.contains(point))
    {
        POINT source, destiny, click;
        source.x = linkOrigin.x();
        source.y = linkOrigin.y();
        destiny.x = linkDestiny.x();
        destiny.y = linkDestiny.y();
        click.x = point.x();
        click.y = point.y();

        return HitTestLine(source, destiny, click, 1);
    }
    else if (std::abs(linkOrigin.x() - linkDestiny.x()) < 0.0001)
    {
        //A função contans de QRectF não lida muito bem com precisão de doubles
        if((std::abs(linkOrigin.x() - point.x()) < 0.5))  //Coloco a precisão de "meio pixel"
        {
            //deve checar o eixo y

            return (point.y() > linkOrigin.y() && point.y() < linkDestiny.y());
        }

        //deve checar o y tmb
    }

    return false;
}


//Checa se o clique foi feito em um elemento especifico
//QPointF &mousePos é dado em coordenada de mundo
bool isClickedInElement(const QPointF &elementCenter,
                        const QPointF &mousePos,
                        float radius = 5.0)
{
    float distance = std::sqrt(std::pow(elementCenter.x() - mousePos.x(), 2)
                                + std::pow(elementCenter.y() - mousePos.y(), 2));
    return distance <= radius;
}

std::vector<LinkData> links =
{
    {
        5,
        0,
        1,
        false
    },
    {
        6,
        2,
        3,
        false
    }
};

bool alreadyExistsLinksWithOriginAndDestiny(uint origin, uint destiny)
{
    for (auto link: links)
    {
        if (
                (link.origin == origin && link.destiny == destiny) ||
                (link.destiny == origin && link.origin == destiny)
            )
            return true;
    }
    return false;
}

std::vector<ElementsData> elements =
{
    {
        0,
        {-130, 40},
        ElementType::DEMAND,
        false
    },
    {
        1,
        {-130, 60},
        ElementType::DEMAND,
        false,
    },
    {
        2,
        {0, 40},
        ElementType::RESERVOIR,
        false
    },
    {
        3,
        {0, 60},
        ElementType::RESERVOIR,
        false
    },
    {
        4,
        {-80, 40},
        ElementType::JUNCTION,
        false
    }
};

float radius = 5.0;

//Este mapa deve salvar as posições originais dos elementos que estão sendo salvos
std::map<uint, QPointF> mapOfOrigPosOfMovedElements;

uint getNextAvailableIDOFNode()
{
    if (elements.size() == 0) return 0;

    uint max = 0;
    for (auto element : elements)
    {
        if (element.id > max) max = element.id;
    }

    return max + 1;
}

uint getNextAvailableIDOFLink()
{
    if (links.size() == 0) return 0;

    uint max = 0;
    for (auto link: links)
    {
        if (link.id > max) max = link.id;
    }

    return max + 1;
}

//Checa se o clique foi feito em algum elemento da rede
//QPointF &mousePos é dado em coordenada de mundo
bool someElementWasClicked(const QPointF &mousePos,
                           float radius = 5.0)
{
    for (auto &element : elements)
    {
        if (isClickedInElement(element.point, mousePos, radius))
        {
            return true;
        }
    }
    return false;
}

bool aquireIDOfClickedElement(const QPointF &mousePos,
                              uint& id,
                           float radius = 5.0)
{
    for (auto &element : elements)
    {
        if (isClickedInElement(element.point, mousePos, radius))
        {
            id = element.id;
            return true;
        }
    }
    return false;
}

bool aquireElementByID(const uint id, ElementsData& el)
{
    for (auto element : elements)
    {
        if (element.id == id)
        {
            el = element;
            return true;
        }
    }
    return false;
}

void changeElementType(uint id, ElementType newType)
{
    for (auto &el: elements)
    {
        if (el.id == id)
        {
            if (newType != el.type)
            {
                el.type = newType;
            }
        }
    }

    //element.type = newType;
}

//Checa se algum elemento está selecionado
bool someElementIsSelected()
{
    for (auto &element : elements)
    {
        if (element.isSelected)
        {
            return true;
        }
    }
    return false;
}

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

QPoint viewPortToWindow2(QPointF viewPoint)
{
    QPoint windowPoint;
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

std::vector<QPointF> getRectPoints (QPointF origin, QPointF final)
{
    std::vector<QPointF> points = {
                                    {origin.x(), origin.y()},
                                    {origin.x() + (final.x() - origin.x()), origin.y()},
                                    {origin.x() + (final.x() - origin.x()), origin.y() + (final.y() - origin.y())},
                                    {origin.x(), origin.y() + (final.y() - origin.y())}};
    return points;
}

//Essa função deve ser aplicada recebendo viewport
QPoint getLeftTop(std::vector<QPointF> vec)
{
    bool valid = true;
    for (size_t i = 0; i< vec.size(); i++)
    {
        for (size_t j = 0; j< vec.size(); j++)
        {
            //if (!(vec[i].x()  <= vec[j].x() && vec[i].y()  >= vec[j].y()))
            if (vec[i].x()  > vec[j].x() || vec[i].y()  < vec[j].y())
            {
                valid = false;
                break;
            }
        }
        if (valid)
            return QPoint(vec[i].x(), vec[i].y());
        else
            valid = true;
    }
    return QPoint(0,0);
}

QPoint getBottomRight(std::vector<QPointF> vec)
{
    bool valid = true;
    for (size_t i = 0; i< vec.size(); i++)
    {
        for (size_t j = 0; j< vec.size(); j++)
        {
            //if (!(vec[i].x()  >= vec[j].x() && vec[i].y()  <= vec[j].y()))
            if (vec[i].x()  < vec[j].x() || vec[i].y()  > vec[j].y())
            {
                valid = false;
                break;
            }
        }
        if (valid)
            return QPoint(vec[i].x(), vec[i].y());
        else
            valid = true;
    }
    return QPoint(0,0);
}

#endif // TRANSFORMATION_H
