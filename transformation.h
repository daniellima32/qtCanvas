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

bool isPointOfLink(const QPointF &linkOrigin,
                   const QPointF &linkDestiny,
                   const QPointF &point)
{
    //Verificar se link está no quadrilátero definido pelos pontos
    //Isso faz com que se elimine da verificação os pontos externos
    QRectF rect (linkOrigin, linkDestiny);
    return rect.contains(point);
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
