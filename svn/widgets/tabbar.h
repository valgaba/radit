#ifndef TABBAR_H
#define TABBAR_H



#include <QTabBar>
#include <QColor>


class TabBar: public QTabBar
{

    Q_OBJECT


    private:


    public:



       explicit TabBar(QWidget *parent = 0);
       ~TabBar();

       void setTabColor(int index, const QColor &color);
       QColor tabColor(int index) const;




      protected:

        void paintEvent(QPaintEvent *event) override;

        void dragEnterEvent(QDragEnterEvent *event) override;
        void dragMoveEvent(QDragMoveEvent *event) override;


    private slots:

    public slots:



};



#endif // TABBAR_H
