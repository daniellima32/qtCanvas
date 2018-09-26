#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <QPointF>
#include <QRect>
#include <QLineF>

enum ElementType
{
    DEMAND, RESERVOIR, JUNCTION, LINK
};

enum LinkType
{
    NATURAL, ARTIFICIAL
};

struct LabelLine
{
    QPointF linPointDif; //Em coordenada de tela
    QString content;
};

using Label = std::vector<LabelLine>;

struct ElementsData
{
    uint id;                //ID que descreve esse elemento
    QPointF point;          //Posição GPS do nó
    ElementType type;       //Tipo do elemento
    bool isSelected;        //Indicação se está selecionado ou não
    Label label;
};

struct LinkData
{
    uint id;                //ID que descreve esse elemento
    uint origin;            //id do nó origem
    uint destiny;           //id do nó destino
    bool isSelected;        //Indicação se está selecionado ou não
    LinkType type;
    Label label;
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
        false,
        LinkType::NATURAL,
        {{{-5, 15}, "Link 0"}}
    },
    {
        6,
        2,
        3,
        false,
        LinkType::ARTIFICIAL,
        {{{-5, 15}, "Link 1"}}
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

std::map<uint, ElementsData*> mapIDToElement;
std::map<uint, LinkData*> mapIDToLink;

std::vector<ElementsData> elements =
{
    {
        0,
        {-130, 40},
        ElementType::DEMAND,
        false,
        {{{-5, 15}, "Demanda 0"}}
    },
    {
        1,
        {-130, 60},
        ElementType::DEMAND,
        false,
        {{{-5, 15}, "Demanda 1"}}
    },
    {
        2,
        {0, 40},
        ElementType::RESERVOIR,
        false,
        {{{-5, 15}, "Reservoir 2"}}
    },
    {
        3,
        {0, 60},
        ElementType::RESERVOIR,
        false,
        {{{-5, 15}, "Reservoir 3"}}
    },
    {
        4,
        {-80, 40},
        ElementType::JUNCTION,
        false,
        {{{-5, 15}, "Junction 4"}}
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

bool aquireClickedElement(const QPointF &mousePos,
                          ElementsData &el,
                          float radius = 2.5)
{
    for (auto &element : elements)
    {
        if (isClickedInElement(element.point, mousePos, radius))
        {
            el = element;
            return true;
        }
    }
    return false;
}

bool aquireElementByID(const uint id, ElementsData& el)
{
    for (auto &element : elements)
    {
        if (element.id == id)
        {
            el = element;
            return true;
        }
    }
    return false;
}

bool someLinkWasClicked(const QPointF &mousePos)
{
    ElementsData origin, destiny;
    bool ret1, ret2;
    for (auto &link : links)
    {
        ret1 = aquireElementByID(link.origin, origin);
        ret2 = aquireElementByID(link.destiny, destiny);
        if (ret1 && ret2)
        {
            if(isPointOfLink(origin.point, destiny.point, mousePos))
            {
                return true;
            }
        }
    }
    return false;
}

bool someElementOrLinkWasClicked(const QPointF &mousePos)
{
    return  (someElementWasClicked(mousePos) || someLinkWasClicked(mousePos));
}

bool aquireClickedLink(const QPointF &mousePos, LinkData &l)
{
    ElementsData origin, destiny;
    bool ret1, ret2;
    for (auto &link : links)
    {
        ret1 = aquireElementByID(link.origin, origin);
        ret2 = aquireElementByID(link.destiny, destiny);
        if (ret1 && ret2)
        {
            if(isPointOfLink(origin.point, destiny.point, mousePos))
            {
                l = link;
                return true;
            }
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
}

void changeLinkType(uint id, LinkType newType)
{
    for (auto &link: links)
    {
        if (link.id == id)
        {
            if (newType != link.type)
            {
                link.type = newType;
            }
        }
    }
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

bool someLabelOfElementWasClicked(const QPointF &mouseViwPortPos)
{
    Label l;
    QPointF point;
    for (auto el: elements)
    {
        //Para pular elementos desnecessários
        if (!el.isSelected) continue;

        l = el.label;
        //for (auto line: l)   //l é um vector of LabelLine
        for (int index = 0; index < l.size(); ++index)
        {
            //i-ésima entrada de Labelline

            //deve alterar point para somar a posição de el
            point = QPoint((int) windowToViewPort1(el.point).x() - l[index].linPointDif.x() -5,
                    (int) windowToViewPort1(el.point).y() - l[index].linPointDif.y());

            if (isClickedInElement(point, mouseViwPortPos, 5.0))
            {
                return true;
            }
        }
    }

    return false;
}


bool someLabelOfLinkWasClicked(const QPointF &mouseViwPortPos)
{
    Label l;
    QPointF point;

    ElementsData origEl, destEl;

    for (auto link: links)
    {
        //Para pular elementos desnecessários
        if (!link.isSelected) continue;

        //Obtem origem
        aquireElementByID(link.origin, origEl);
        //Obtem destino
        aquireElementByID(link.destiny, destEl);
        QPointF halfPoint ((origEl.point.x() + destEl.point.x())/2,
                           (origEl.point.y() + destEl.point.y())/2);

        l = link.label;
        for (int index = 0; index < l.size(); ++index)
        {
            //i-ésima entrada de Labelline

            //deve alterar point para somar a posição de el
            point = QPoint((int) windowToViewPort1(halfPoint).x() - l[index].linPointDif.x() -5,
                    (int) windowToViewPort1(halfPoint).y() - l[index].linPointDif.y());

            if (isClickedInElement(point, mouseViwPortPos, 5.0))
            {
                return true;
            }
        }
    }

    return false;
}

bool getLabelOfElementThatWasClicked(uint &idOfElementOwnerOfLabel, uint &idLabel, QPointF &labelDiffBackup, const QPointF &mouseViwPortPos)
{
    Label l;
    QPointF point;
    for (auto el: elements)
    {
        //Para pular elementos desnecessários
        if (!el.isSelected) continue;

        l = el.label;
        //for (auto line: l)   //l é um vector of LabelLine
        for (int index = 0; index < l.size(); ++index)
        {
            //i-ésima entrada de Labelline

            //deve alterar point para somar a posição de el
            point = QPoint((int) windowToViewPort1(el.point).x() - l[index].linPointDif.x() -5,
                    (int) windowToViewPort1(el.point).y() - l[index].linPointDif.y());

            if (isClickedInElement(point, mouseViwPortPos, 5.0))
            {
                idOfElementOwnerOfLabel = el.id;
                idLabel = index;
                labelDiffBackup = l[index].linPointDif;
                return true;
            }
        }
    }

    return false;
}

bool getLabelOfLinkThatWasClicked(uint &idOfLinkOwnerOfLabel, uint &idLabel, QPointF &labelDiffBackup, const QPointF &mouseViwPortPos)
{
    Label l;
    QPointF point;

    ElementsData origEl, destEl;

    for (auto link: links)
    {
        //Para pular elementos desnecessários
        if (!link.isSelected) continue;

        //Obtem origem
        aquireElementByID(link.origin, origEl);
        //Obtem destino
        aquireElementByID(link.destiny, destEl);
        QPointF halfPoint ((origEl.point.x() + destEl.point.x())/2,
                           (origEl.point.y() + destEl.point.y())/2);

        l = link.label;
        //for (auto line: l)   //l é um vector of LabelLine
        for (int index = 0; index < l.size(); ++index)
        {
            //i-ésima entrada de Labelline

            //deve alterar point para somar a posição de el
            point = QPoint((int) windowToViewPort1(halfPoint).x() - l[index].linPointDif.x() -5,
                    (int) windowToViewPort1(halfPoint).y() - l[index].linPointDif.y());

            if (isClickedInElement(point, mouseViwPortPos, 5.0))
            {
                idOfLinkOwnerOfLabel = link.id;
                idLabel = index;
                labelDiffBackup = l[index].linPointDif;
                return true;
            }
        }
    }

    return false;
}

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

double norma(QPointF entry)
{
    return sqrt(entry.x()*entry.x() + entry.y()*entry.y());
}

QPointF normalize(QPointF point)
{
    double n = norma(point);
    QPointF ret = {point.x()/n, point.y()/n};
    return ret;
}

QPointF product(QPointF point, int n)
{
    QPointF ret = {point.x() * n, point.y() * n};
    return ret;
}

std::vector<QPointF> getArrowPoints(QLineF line)
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
    if (v0.x() == 0 && v0.y() == 0) return vec;

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

    float xMin = p0.x(), yMin = p0.y();
    float xMax = p0.x(), yMax = p0.y();

    vec.push_back(p0);
    vec.push_back(p1);
    vec.push_back(p2);

    for (int i = 0; i < 3; ++i)
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

    //dc.DrawPolygon(3, p);
    //m_arrowRect = wxRect(xMin, yMin, xMax - xMin, yMax - yMin);
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
