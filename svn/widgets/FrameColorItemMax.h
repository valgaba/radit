#ifndef FRAMECOLORITEMMAX_H
#define FRAMECOLORITEMMAX_H





#include <QPainter>

#include "widgets/frame.h"

class FrameColorItemMax : public Frame {
public:
    FrameColorItemMax(QWidget *parent = nullptr) : Frame(parent) {
        setFixedSize(100, 10); // Tamaño del QFrame
        setFrameShape(QFrame::NoFrame);
        setFrameShadow(QFrame::Plain);
        setStyleSheet("Frame { border: none;}");


    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QFrame::paintEvent(event);

        QPainter painter(this);
           painter.setPen(Qt::NoPen);

           painter.setBrush(Qt::blue);

           // Coordenadas de la linea larga
                 QPointF topLeft(0, 0);
                 QPointF topRight(50, 0);
                 QPointF bottomRight(40, 10); // más corto que topRight para efecto trapecio
                 QPointF bottomLeft(0, 10);  // más corto que topLeft para efecto trapecio

                 QVector<QPointF> linelong;
                 linelong << topLeft << topRight << bottomRight << bottomLeft;
                 painter.drawPolygon(linelong);

                 // lineas pequeñas
                 topLeft = QPointF(55, 0);
                 topRight = QPointF(65, 0);
                 bottomRight = QPointF(55, 10);
                 bottomLeft = QPointF(45, 10);

                 QVector<QPointF> line1;
                 line1 << topLeft << topRight << bottomRight << bottomLeft;
                 painter.drawPolygon(line1);

                 // lineas pequeñas
                 topLeft = QPointF(70, 0);
                 topRight = QPointF(80, 0);
                 bottomRight = QPointF(70, 10);
                 bottomLeft = QPointF(60, 10);

                 QVector<QPointF> line2;
                 line2 << topLeft << topRight << bottomRight << bottomLeft;
                 painter.drawPolygon(line2);















    }
};





#endif // FRAMECOLORITEMMAX_H
