#ifndef DRAGGABLEWIDGET_H
#define DRAGGABLEWIDGET_H

#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QLabel>

//recurso para su estudio



// llamada a la clase
// DraggableWidget *widget1 = new DraggableWidget("Widget 1");





class DraggableWidget : public QWidget {
public:
    DraggableWidget(const QString &text, QWidget *parent = nullptr)
        : QWidget(parent) {

setMinimumSize(100, 50); // Tamaño mínimo para evitar pixmaps vacíos

        QLabel *label = new QLabel(text, this);
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(label);
        setLayout(layout);
        setAcceptDrops(true);
    }

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {


            dragStartPosition = event->pos();


        }
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if (!(event->buttons() & Qt::LeftButton)) return;
        if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance()) return;

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        mimeData->setText("DraggableWidget");
        drag->setMimeData(mimeData);

        QPixmap pixmap(this->size()); // crea el efecto de arrastrar el item
        if (!pixmap.isNull()) {
            this->render(&pixmap);
            drag->setPixmap(pixmap);
            drag->setHotSpot(event->pos());
        } else {
            // Alternativa si no se puede crear el pixmap
            drag->setPixmap(QPixmap(1, 1)); // Pixmap mínimo
        }




        Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
    }

    void dragEnterEvent(QDragEnterEvent *event) override {
        if (event->mimeData()->hasFormat("text/plain")) {
            event->acceptProposedAction();
        }
    }

    void dropEvent(QDropEvent *event) override {
        if (event->mimeData()->hasFormat("text/plain")) {
            QWidget *source = qobject_cast<QWidget*>(event->source());

            if (source && source != this) {
                // 1. Obtener el layout de destino (donde se soltó)
                QVBoxLayout *targetLayout = qobject_cast<QVBoxLayout*>(this->parentWidget()->layout());

                if (targetLayout) {
                    // 2. Remover el widget de su layout original
                    if (source->parentWidget()) {
                        if (QLayout *sourceLayout = source->parentWidget()->layout()) {
                            sourceLayout->removeWidget(source);
                        }
                    }

                    // 3. Insertar en la nueva posición (antes del widget actual)
                    int targetIndex = targetLayout->indexOf(this);
                    targetLayout->insertWidget(targetIndex, source);

                    // 4. Asegurar que el widget tenga el parent correcto
                    source->setParent(this->parentWidget());

                    event->acceptProposedAction();
                }
            }
        }
    }

private:
    QPoint dragStartPosition;
};





#endif // DRAGGABLEWIDGET_H
