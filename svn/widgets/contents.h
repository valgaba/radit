#ifndef CONTENTS_H
#define CONTENTS_H




#include <QWidget>
#include <QVBoxLayout>
#include <QList>
#include <QFrame>
#include <QDebug>

#include "widgets/menu.h"
#include "core/Clipboard.h"


class Contents: public QWidget
{

    Q_OBJECT


    private:

      Menu *menu;
      QPoint mousePos;

     //opciones del menu contectual
      QAction *cutAction;
      QAction *copyAction;
      QAction *pasteAction;
      QAction *deleteAction;
      QAction *propertiesAction;

      bool isCut;  //para las operaciones de cortar pegar


      QList<QWidget*>  list;
      void CleanList(); //crea qlist temporal para funcion de move
      void UpdateList();  //copia qlist temporal en el layout
      void MoveList(int from, int to);


      int frommove; // item para mover
      int tomove; // item para mover





    public:
       QVBoxLayout *layout;
      void AddItem(QWidget * item);
        Clipboard &clipboard=Clipboard::instance();


       explicit Contents(QWidget *parent = 0);
       ~Contents();




    protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;


    private slots:


    public slots:





};


#endif // CONTENTS_H
