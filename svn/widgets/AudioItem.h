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

    QString m_filePath;
    double  m_second;
    bool    m_isSelect=false;



public:
    // Constructor
    explicit AudioItem(QWidget *parent = nullptr);

    // --- SETTERS ---
      void setFilePath(const QString &filePath);
      void setSecond(double second);

      // --- GETTERS ---
      QString filePath() const;
      double second() const;

      virtual void setIsSelect(bool value);
      bool isSelect() const;





protected:
    // Eventos de ratón para implementar el arrastre
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;


};

#endif // AUDIOITEM_H
