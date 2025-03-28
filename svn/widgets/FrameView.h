#ifndef FRAMEVIEW_H
#define FRAMEVIEW_H

#include <QVBoxLayout>
#include <QMouseEvent>
#include <QLabel>
#include <QTimer>
#include <QFrame>

#include "widgets/frame.h"
#include "widgets/menu.h"
#include "widgets/ExternalWindow.h"



class FrameView: public Frame

{
    Q_OBJECT

private:

    QFrame *frame;
    Menu *menu;
    QLabel *label;

   ExternalWindow *externalwindow;

  QTimer *timer;
   int timeLeft = 10; // Inicializar el tiempo restante

public:

QVBoxLayout *layout;

    explicit FrameView( QWidget *parent = 0);
    ~FrameView();





protected:

    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:

    void updateCountdown();




};





#endif // FRAMEVIEW_H
