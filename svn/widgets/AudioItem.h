#ifndef AUDIOITEM_H
#define AUDIOITEM_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>

// Clase que representa un elemento de audio arrastrable
class AudioItem : public QWidget {
    Q_OBJECT

private:
    QPoint dragStartPosition; // Guarda la posición inicial para el arrastre

    QString filePath;
    double filesecond;



public:
    // Constructor
    explicit AudioItem(QWidget *parent = nullptr);

     virtual AudioItem* copy() const = 0; // Método virtual puro





protected:
    // Eventos de ratón para implementar el arrastre
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    // Eventos de drag and drop
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;


};

#endif // AUDIOITEM_H
