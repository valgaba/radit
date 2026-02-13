#ifndef CONTENTSBASE_H
#define CONTENTSBASE_H




#include <QWidget>
#include <QVBoxLayout>
#include <QList>
#include <QDebug>


#include "core/Clipboard.h"


class ContentsBase: public QWidget{


    Q_OBJECT


private:


    QList<QWidget*>  list;




public:
    QVBoxLayout *layout;

    Clipboard &clipboard=Clipboard::instance();


    explicit ContentsBase(QWidget *parent = 0);
    ~ContentsBase();




protected:

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;

    void paintEvent(QPaintEvent *event) override;

private slots:


public slots:





};





#endif // CONTENTSBASE_H
