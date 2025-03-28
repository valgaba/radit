#ifndef VIEW_H
#define VIEW_H
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>





class View : public QGraphicsView {
public:
    explicit View(QGraphicsScene *scene);

protected:
    void wheelEvent(QWheelEvent *event) override;


    ~View();

};




#endif // VIEW_H
