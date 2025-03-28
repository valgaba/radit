#ifndef GRAPHICSTEXTITEM_H
#define GRAPHICSTEXTITEM_H

#include <QGraphicsTextItem>

#include <QGraphicsSceneWheelEvent>

class GraphicsTextItem : public QGraphicsTextItem{


    Q_OBJECT

private:


public:
    explicit GraphicsTextItem(QGraphicsItem* parent = nullptr);

   ~GraphicsTextItem();





protected:

   void wheelEvent(QGraphicsSceneWheelEvent *event) override;




private slots:





};

#endif // GRAPHICSTEXTITEM_H
