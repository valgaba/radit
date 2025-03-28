#ifndef EXTERNALWINDOW_H
#define EXTERNALWINDOW_H

#include <QWidget>
#include <QPixmap>
#include <QFrame>
#include <QLabel>
#include <QGraphicsScene>




class ExternalWindow : public QWidget {

     Q_OBJECT

private:
     QFrame *frame;
     QLabel *label;
     QPixmap scaledPixmap;




public:

    void renderFrame(const QPixmap &pixmap);

    explicit ExternalWindow( QWidget *parent = 0);
    ~ExternalWindow();



protected:

    void resizeEvent(QResizeEvent *event) override;


private slots:




};






#endif // EXTERNALWINDOW_H
