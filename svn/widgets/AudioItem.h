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
    int dayweek;
    int hour, minute, second;

    QString filePath;
    double filesecond;



public:
    // Constructor
    explicit AudioItem(QWidget *parent = nullptr);

    void setdayweek(int dayweek);
    void sethour(int hour);
    void setminute(int minute);
    void setsecond(int second);

    void setfilePath(QString filePath);
    void setfilesecond(double filesecond);



    int getdayweek() const;
    int gethour() const;
    int getminute() const;
    int getsecond() const;

    QString getfilePath() const;
    double getfilesecond()const ;

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
