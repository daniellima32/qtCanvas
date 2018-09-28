#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include "transformation.h"
#include <QMouseEvent>
#include <iostream>
#include <QMessageBox>
#include <QInputDialog>
#include "coordinatesmanip.h"

void updateMap()
{
    mapIDToElement.clear();
    for (auto &el: elements)
    {
        mapIDToElement[el.id] = &el;
    }

    mapIDToLink.clear();
    for (auto &link: links)
    {
        mapIDToLink[link.id] = &link;
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    updateMap();

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
    else if (event->key() == Qt::Key_D ||           //Usarei a tecla d para deletar
             event->key() == Qt::Key_Delete)        //Usarei a tecla delete para deletar
    {
        ElementsData origin, destiny;
        //Descobrir se existem elementos origem e destino a ser removidos
        //e o link entre eles não selecionado
        for (size_t index = 0; index < links.size(); index++)
        {
            if (!links[index].isSelected) //Se o link não está selecionado
            {
                //Descobrir origem
                aquireElementByID(links[index].origin, origin);
                //Descobrir destino
                aquireElementByID(links[index].destiny, destiny);

                if (origin.isSelected || destiny.isSelected)
                {
                    //Removendo links (não selecionados) que estão entre os elementos selecionados
                    //links.erase(links.begin()+index);
                    links.erase(links.begin()+static_cast<int>(index));
                    index--;
                }
            }
        }

        for (size_t index = 0; index < links.size(); ++index)
        {
            if (links[index].isSelected)
            {
                links.erase(links.begin()+static_cast<int>(index));
                index--;
            }
        }

        for (size_t index = 0; index < elements.size(); ++index)
        {
            if (elements[index].isSelected)
            {
                elements.erase(elements.begin()+static_cast<int>(index));
                index--; //Esse index-- é usado porque um novo elemento de indice index
                //será atribuído pela função erase
            }
        }

        this->refreshPixmap();
    }
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        const QString title = "title";
        const QString label = "label";
        const QString text = "text";

        QInputDialog dialog(this);
        dialog.setOptions(QInputDialog::UsePlainTextEditForTextInput);
        dialog.setWindowTitle(title);
        dialog.setLabelText(label);
        dialog.setTextValue(text);
        QString newName;

        int ret = dialog.exec();

        if (ret == 1)  //Usuário inseriu texto
        {
            newName = dialog.textValue();
            Label labelVec;

            uint nextId = getNextAvailableIDOFNode();
            QPointF windowPos = viewPortToWindow1({
                                                      static_cast<double>(event->x()),
                                                      static_cast<double>(event->y())
                                                  });

            //Criar a cadeia de titulo
            QStringList list = newName.split("\n");
            double diffHeight = 15.0;
            //for (auto part: list)
            for (int index = list.size()-1; index >= 0; index--)
            {
                labelVec.push_back({{-5.0, diffHeight}, list[index]});
                diffHeight+=15.0;
            }

            elements.push_back(
                            {
                                nextId,
                                windowPos,
                                ElementType::DEMAND,
                                false,
                                labelVec
                            }
                              );

            updateMap(); //Atualiza o mapa de elementos

            this->refreshPixmap();
        }
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    selectedRect.clear(); //Apaga a indicação de possível região selecionada

    //Mudando local do release
    QPointF windowPos = viewPortToWindow1({static_cast<double>(event->x()),
                                           static_cast<double>(event->y())});
    if (!elementsBeeingMoved)
    {
        ElementsData el;
        bool foundEl = aquireClickedElement(windowPos, el);
        for (auto &element : elements)
        {
            //if (isClickedInElement(element.point, windowPos))
            if (foundEl && element.id == el.id)
            {
                element.isSelected = !element.isSelected;
            }
            else
            {
                //se control não está pressionado
                if (!drawingSelection && !controlIsDown) element.isSelected = false;
            }
        }

        if (!foundEl)
        {
            LinkData l;
            bool ret = aquireClickedLink(windowPos, l);

            for (auto &link: links)
            {
                if (ret && link.id == l.id) link.isSelected = !link.isSelected;
                else
                {
                    //se control não está pressionado
                    if (!drawingSelection && !controlIsDown) link.isSelected = false;
                }
            }
        }
        else
        {
            if (!drawingSelection && !controlIsDown)
            {
                for (auto &link: links)
                {
                     link.isSelected = false;
                }
            }
        }
    }
    drawingSelection = false;
    elementsBeeingMoved = false;
    labelOfElementBeeingChanged = false;
    labelOfLinkBeeingChanged = false;

    //Checa se um elemento temporário foi inserido
    if (temporaryElementInserted)
    {
        //Deve checar se o local de saída foi sobre algum nó já exitente
        //Se sim, deve apenas fazer uma ligação entre o ló de origem e o nó de destino
        //e setar temporaryElementInserted=false

        //Remover último elemento inserido
        ElementsData elementCopy = elements[elements.size()-1];
        elements.pop_back();
        //Remover link
        LinkData linkCopy = links[links.size()-1];
        links.pop_back();

        if (someElementWasClicked(windowPos))
        {
            uint id, nextId;
            bool ret1 = true, ret2 = true;
            ret1 = aquireIDOfClickedElement(lastMouseWindowPosition, id);
            ret2 = aquireIDOfClickedElement(windowPos, nextId);

            if (ret1 && ret2)
            {
                //Checar se ja existe link entre esses elementos
                if (alreadyExistsLinksWithOriginAndDestiny(id, nextId))
                {
                    //Informar ao usuário que não pode inserir elemento
                    QMessageBox msgBox;
                    msgBox.setWindowTitle("Atenção");
                    msgBox.setText("Já existe um link entre esses nós.");
                    msgBox.exec();
                }
                else
                {
                    //Fazer link com o novo elemento
                    uint nextLinkId = getNextAvailableIDOFLink();

                    links.push_back(
                    {
                        nextLinkId,
                        id,           //id da origem
                        nextId,       //id do destino
                        false,
                        LinkType::NATURAL,
                        {{{-5, 15}, "Link "+ QString::number(nextLinkId)}}
                    });
                }
            }
        }
        else
        {
            //Caso contrário, deve manter o elemento criado e setar temporaryElementInserted=false
            elements.push_back(elementCopy);

            //será criado manualmente
            links.push_back(linkCopy);
        }

        temporaryElementInserted=false;

        updateMap(); //Atualiza o mapa de elementos
    }

    if (mapOfOrigPosOfMovedElements.size() > 0) mapOfOrigPosOfMovedElements.clear();

    this->refreshPixmap();
}

void MainWindow::resizeEvent(QResizeEvent* )
{
    this->refreshPixmap();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QPointF viewPortPos = {static_cast<double>(event->x()),
                static_cast<double>(event->y())};
    QPointF windowPos = viewPortToWindow1(viewPortPos);

    bool ret = true;
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
            bool allOk = true;

            //Verifica se pode mover
            for (auto &el: elements)
            {
                if (el.isSelected)
                {
                    QPointF translateFactor (windowPos.x() - lastMouseWindowPosition.x(),
                                             windowPos.y() - lastMouseWindowPosition.y());

                    el.point = mapOfOrigPosOfMovedElements[el.id];   //Restaura valor original

                    //Aqui faço uma verificação para saber se esse deslocamento é permitido,
                    //uma vez que o elemento está selecionado
                    //se sim, eu faço o movimento

                    QPointF test = el.point;
                    translatePoint(translateFactor, test);       //transladando ponto
                    test = windowToViewPort1(test); //obtenho a ref dele no viewport
                    //verifico se está ok (dentro do viewport)
                    if (!(test.x() >= viewPort.point.x() &&
                        test.y() >= viewPort.point.y() &&
                        test.x() <= viewPort.point.x() + viewPort.width &&
                        test.y() <= viewPort.point.y() + viewPort.height))
                    {
                        allOk = false;
                        break;
                    }
                }
            }

            //Translada os elementos
            for (auto &el: elements)
            {
                if (el.isSelected)
                {
                    QPointF translateFactor (windowPos.x() - lastMouseWindowPosition.x(),
                                             windowPos.y() - lastMouseWindowPosition.y());

                    el.point = mapOfOrigPosOfMovedElements[el.id];   //Restaura valor original

                    //verifico se está ok (dentro do viewport)
                    if (allOk)
                    {
                        translatePoint(translateFactor, el.point);
                        mapOfLastValidPosOfMovedElements[el.id] = el.point;
                    }
                    else
                    {
                        el.point = mapOfLastValidPosOfMovedElements[el.id];
                    }
                }
            }
        } // fim de mover elementos
        //Inicio de detectar alteração de posição de label de elemento node
        else if(labelOfElementBeeingChanged || someLabelOfElementWasClicked(windowToViewPort1(lastMouseWindowPosition)))
        {
            if (!labelOfElementBeeingChanged)
            {
                ret = getLabelOfElementThatWasClicked(idOfElementOwnerOfLabel, idLabel, labelDiffBackup, windowToViewPort1(lastMouseWindowPosition));
            }

            if (ret)
            {
                labelOfElementBeeingChanged = true; //No próximo evento, labelBeeingChanged já tem o valor true,
                //evitando fazer a parte direita da comparação do if

                QPointF translateFactor (-(viewPortPos.x() - windowToViewPort1(lastMouseWindowPosition).x()),
                                                     -(viewPortPos.y() - windowToViewPort1(lastMouseWindowPosition).y()));

                QPointF pDiff = labelDiffBackup;
                translatePoint(translateFactor, pDiff);
                mapIDToElement[idOfElementOwnerOfLabel]->label[idLabel].linPointDif = pDiff;
            }
        }
        //Detectar clique em label de link
        else if(labelOfLinkBeeingChanged || someLabelOfLinkWasClicked(windowToViewPort1(lastMouseWindowPosition)))
        {
            QPointF viewPortPos (static_cast<double>(event->x()),
                                 static_cast<double>(event->y()));
            if (!labelOfLinkBeeingChanged)
            {
                ret = getLabelOfLinkThatWasClicked(idOfLinkOwnerOfLabel, idLabel, labelDiffBackup, windowToViewPort1(lastMouseWindowPosition));
            }

            if (ret)
            {
                labelOfLinkBeeingChanged = true; //No próximo evento, labelBeeingChanged já tem o valor true,
                //evitando fazer a parte direita da comparação do if

                QPointF translateFactor (-(viewPortPos.x() - windowToViewPort1(lastMouseWindowPosition).x()),
                                                     -(viewPortPos.y() - windowToViewPort1(lastMouseWindowPosition).y()));

                QPointF pDiff = labelDiffBackup;
                translatePoint(translateFactor, pDiff);
                mapIDToLink[idOfLinkOwnerOfLabel]->label[idLabel].linPointDif = pDiff;
            }
        }
        else
        {
            //Fazer pan de window
            //Checar se é uma posição inválida de mouse
            if (checkInvalidMousePos(viewPortPos)) return;

            QPointF diff = {lastMouseWindowPosition.x() - windowPos.x(), lastMouseWindowPosition.y() - windowPos.y()}; //better
            translateRect(diff, transformation::window);

            QPointF newPos;
            //Ver se após a translação algum elemento selecionado não está ok
            for (auto &el: elements)
            {
                if (el.isSelected)
                {
                    newPos = windowToViewPort1(el.point);
                    if (!(newPos.x() >= viewPort.point.x() &&
                        newPos.y() >= viewPort.point.y() &&
                        newPos.x() <= viewPort.point.x() + viewPort.width &&
                        newPos.y() <= viewPort.point.y() + viewPort.height))
                    {
                        translateRect({-diff.x(), -diff.y()}, transformation::window);
                        break;
                    }
                }
            }
        }
    }
    else if (event->buttons() == Qt::LeftButton)
    {
        if (someElementWasClicked(lastMouseWindowPosition))
        {
            if (!temporaryElementInserted)
            {
                //Deve criar um elemento temporário
                uint nextId = getNextAvailableIDOFNode();

                ElementsData element;
                uint id;
                ret = aquireIDOfClickedElement(lastMouseWindowPosition, id);
                ret =  ret && aquireElementByID(id, element);

                if (ret)
                {
                    elements.push_back(
                                    {
                                        nextId,
                                        windowPos,
                                        element.type,
                                        false,
                                        {{{-5, 15}, "Elemento "+ QString::number(nextId)}}
                                    }
                                      );

                    uint nextLinkId = getNextAvailableIDOFLink();
                    links.push_back(
                    {
                        nextLinkId,
                        id,
                        nextId,
                        false,
                        LinkType::NATURAL,
                        {{{-5, 15}, "Link "+ QString::number(nextLinkId)}},
                        {} //Sem elementos intermediários
                    }
                                );
                }

                temporaryElementInserted = true;
            }
            else
            {
                //Alterar posição de último elemento
                //O link é alterado automaticamente
                ElementsData element;
                if (elements.size() > 0)
                    elements[elements.size()-1].point = windowPos;
            }

        }
        else
        {
            //Desenhar retangulo representando a seleção
            selectedRect.clear();
            //Desenhar rectangulo saindo de lastMouseWindowPosition ate posição atual em window
            selectedRect.push_back(lastMouseWindowPosition);
            selectedRect.push_back(windowPos);

            QPointF origin = windowToViewPort1(selectedRect[0]);
            QPointF final = windowToViewPort1(selectedRect[1]);
            std::vector<QPointF> vec = getRectPoints(origin, final);
            QPoint pLeftTopViewPort = getLeftTop(vec);
            QPoint pRightBottomViewPort = getBottomRight(vec);

            QPointF pLeftTopViewPortInWinCoord = viewPortToWindow1(pLeftTopViewPort);
            QPointF pRightBottomViewPortInWinCoord = viewPortToWindow1(pRightBottomViewPort);

            QRect selectionRect
                    (
                        QPoint(
                            static_cast<int>(pLeftTopViewPortInWinCoord.x()),
                            static_cast<int>(pLeftTopViewPortInWinCoord.y())
                            ),
                        QPoint(
                            static_cast<int>(pRightBottomViewPortInWinCoord.x()),
                            static_cast<int>(pRightBottomViewPortInWinCoord.y())
                            )
                    );
            for (auto &el: elements)
            {
                if (
                        selectionRect.contains(
                            static_cast<int> (el.point.x()),
                            static_cast<int> (el.point.y())
                                              )
                   )
                {
                    el.isSelected = true;
                }
                else if (!controlIsDown)
                {
                    el.isSelected = false;
                }
            }

            ElementsData originLink, destinyLink;
            //Fazer seleção dos links
            for (auto &link: links)
            {
                //Descobrir origem
                aquireElementByID(link.origin, originLink);
                //Descobrir destino
                aquireElementByID(link.destiny, destinyLink);
                if (selectionRect.contains(
                            static_cast<int>(originLink.point.x()),
                            static_cast<int>(originLink.point.y())
                                           ) &&
                    selectionRect.contains(
                            static_cast<int>(destinyLink.point.x()),
                            static_cast<int>(destinyLink.point.y())
                                          )
                   )
                {
                    link.isSelected = true;
                }
                else if (!controlIsDown)
                {
                    link.isSelected = false;
                }
            }

            drawingSelection = true;
        }
    }
    this->refreshPixmap();
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
    }
    else if (someLinkWasClicked(windowPos))
    {
        LinkData link;
        aquireClickedLink(windowPos, link);
        uint id = link.id;

        if (link.type != LinkType::NATURAL)
        {
            QAction* actNat = new QAction(tr("&Natural"), this);
            menu.addAction(actNat);
            connect(actNat, &QAction::triggered, this, [=](){
                changeLinkType(id, LinkType::NATURAL);
            });
        }

        if (link.type != LinkType::ARTIFICIAL)
        {
            QAction* actArt = new QAction(tr("&Artificial"), this);
            menu.addAction(actArt);
            connect(actArt, &QAction::triggered, this, [=](){
                changeLinkType(id, LinkType::ARTIFICIAL);
            });
        }

        //Sempre sugerir quebrar link
        //Quebra de link
        QAction* breakArt = new QAction(tr("&Inserir quebra de link"), this);
        menu.addAction(breakArt);
        connect(breakArt, &QAction::triggered, this, [=](){
            //changeLinkType(id, LinkType::ARTIFICIAL);
            insertBreakInPath(id, viewPortPos);
        });

        //Se tem quebra de link, sugerir a remoção de quebra de link
        if (link.path.size() == 0)
        {
            //Quebra de link
            QAction* removeBreakArt = new QAction(tr("&Remover quebras de link"), this);
            menu.addAction(removeBreakArt);
            connect(removeBreakArt, &QAction::triggered, this, [=](){;
                clearPath(link.id);
            });
        }
    }

    menu.exec(event->globalPos());
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    static double zoomIn = 0.9;
    static double zoomOut = 1.1;
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
    QPointF windowPos = viewPortToWindow1({static_cast<double>(event->x()),
                                           static_cast<double>(event->y())});
    lastMouseWindowPosition = windowPos;
    if (event->buttons() == Qt::RightButton)
    {
        if (someElementOrLinkWasClicked(windowPos) && !someElementIsSelected())
        {
            dealWithcontextMenuEvent(event);
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

    painter.setFont(QFont("times", 12));

    //Desenhar links
    for(auto link: links)
    {
        painter.setBrush(QBrush(Qt::blue, Qt::SolidPattern));

        if (!link.isSelected)
        {
            if (link.type == LinkType::NATURAL)
                painter.setPen(Qt::blue);
            else
                painter.setPen(Qt::black);
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
        bool ret1, ret2;
        ret1 = aquireElementByID(idOrigin, originElement);
        ret2 = aquireElementByID(idDestiny, destinyElement);

        if (!ret1 || !ret2) continue;

        //Desenhar a reta
        /*painter.drawLine(windowToViewPort1(originElement.point),
                         windowToViewPort1(destinyElement.point));*/

        //Inicio de novo código
        //Se não tem elementos de quebra, otimizo operação de desenho
        if (link.path.size() == 0)
        {
            painter.drawLine(windowToViewPort1(originElement.point),
                                     windowToViewPort1(destinyElement.point));

            //Desenhar arrow do link
            /*if (link.type == LinkType::NATURAL)
            {
                painter.setBrush(QBrush(Qt::blue, Qt::SolidPattern));
                painter.setPen(Qt::blue);
            }
            else
            {
                painter.setBrush(QBrush(Qt::black, Qt::SolidPattern));
                painter.setPen(Qt::black);
            }*/

            QLineF line(windowToViewPort1(originElement.point), windowToViewPort1(destinyElement.point));
            std::vector<QPointF> vec = CoordinatesManip::getArrowPoints(line);

            if (vec.size() == 0) continue;
            painter.drawConvexPolygon(vec.data(), 3);
        }
        else
        {
            QPointF p1, p2;
            p1 = originElement.point;

            /*if (link.type == LinkType::NATURAL)
            {
                painter.setBrush(QBrush(Qt::blue, Qt::SolidPattern));
                painter.setPen(Qt::blue);
            }
            else
            {
                painter.setBrush(QBrush(Qt::black, Qt::SolidPattern));
                painter.setPen(Qt::black);
            }*/

            //Descobrir em qual dos paths foi clicado
            for (size_t index = 0; index < link.path.size(); index++)
            {
                p2 = link.path[index];

                //Desenha link de p1 para p2
                painter.drawLine(windowToViewPort1(p1), windowToViewPort1(p2));

                //Desenha símbolo da quebra em p2
                painter.drawRect(windowToViewPort1(p2).x(), windowToViewPort1(p2).y(), 3, 3);

                //Desenhar arrow do link
                QLineF line(windowToViewPort1(p1), windowToViewPort1(p2));
                std::vector<QPointF> vec = CoordinatesManip::getArrowPoints(line);

                if (vec.size() == 0) continue;
                painter.drawConvexPolygon(vec.data(), 3);

                p1 = p2;
            }

            //Desenha link do último para o primeiro
            painter.drawLine(windowToViewPort1(link.path[link.path.size()-1]),
                    windowToViewPort1(destinyElement.point));

            //Desenhar arrow do link
            /*if (link.type == LinkType::NATURAL)
            {
                painter.setBrush(QBrush(Qt::blue, Qt::SolidPattern));
                painter.setPen(Qt::blue);
            }
            else
            {
                painter.setBrush(QBrush(Qt::black, Qt::SolidPattern));
                painter.setPen(Qt::black);
            }*/

            QLineF line(
                        windowToViewPort1(link.path[link.path.size()-1]),
                        windowToViewPort1(destinyElement.point)
                    );
            std::vector<QPointF> vec = CoordinatesManip::getArrowPoints(line);

            if (vec.size() == 0) continue;
            painter.drawConvexPolygon(vec.data(), 3);
        }

        //Fim de novo código

        //Desenhar arrow do link
        /*if (link.type == LinkType::NATURAL)
        {
            painter.setBrush(QBrush(Qt::blue, Qt::SolidPattern));
            painter.setPen(Qt::blue);
        }
        else
        {
            painter.setBrush(QBrush(Qt::black, Qt::SolidPattern));
            painter.setPen(Qt::black);
        }

        QLineF line(windowToViewPort1(originElement.point), windowToViewPort1(destinyElement.point));
        std::vector<QPointF> vec = CoordinatesManip::getArrowPoints(line);

        if (vec.size() == 0) continue;
        painter.drawConvexPolygon(vec.data(), 3);*/

        //Desenhar label de link
        //Escrever o label
        QPointF halfPoint ((originElement.point.x() + destinyElement.point.x())/2,
                           (originElement.point.y() + destinyElement.point.y())/2);
        for(auto entry : link.label)
        {
            QPoint point (static_cast<int>(windowToViewPort1(halfPoint).x() - entry.linPointDif.x()),
                    static_cast<int>(windowToViewPort1(halfPoint).y() - entry.linPointDif.y()));
            painter.drawText(point.x(), point.y(), entry.content);

            //Se o elemento está selecionado, deve desenhar as opções para alterar posição do título
            if (link.isSelected)
            {
                painter.setBrush(QBrush(Qt::black, Qt::SolidPattern));
                painter.setPen(Qt::black);

                painter.drawRect(point.x()-5, point.y(), 3, 3);
            }
        }

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

            painter.drawEllipse (windowToViewPort1({el.point}),
                                 2* static_cast<double>(radius),
                                 2* static_cast<double>(radius));
        }

        //Escrever o label
        for(auto entry : el.label)
        {
            QPoint point (static_cast<int>(windowToViewPort1(el.point).x() - entry.linPointDif.x()),
                    static_cast<int>(windowToViewPort1(el.point).y() - entry.linPointDif.y()));
            painter.drawText(point.x(), point.y(), entry.content);

            //Se o elemento está selecionado, deve desenhar as opções para alterar posição do título
            if (el.isSelected)
            {
                painter.setBrush(QBrush(Qt::black, Qt::SolidPattern));
                painter.setPen(Qt::black);

                painter.drawRect(point.x()-5, point.y(), 3, 3);
            }
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
