#ifndef TABBAR_H
#define TABBAR_H



#include <QTabBar>


class TabBar: public QTabBar
{

    Q_OBJECT


    private:


    public:



       explicit TabBar(QWidget *parent = 0);
       ~TabBar();




      protected:

        void dragEnterEvent(QDragEnterEvent *event) override;
        void dragMoveEvent(QDragMoveEvent *event) override;


    private slots:

    public slots:



};



#endif // TABBAR_H
