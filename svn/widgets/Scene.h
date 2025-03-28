#ifndef SCENE_H
#define SCENE_H


#include <QGraphicsScene>





class Scene: public QGraphicsScene

{
    Q_OBJECT

private:


public:



    explicit Scene(QObject *parent = nullptr);
    ~Scene();





protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;



private slots:


};








#endif // SCENE_H
