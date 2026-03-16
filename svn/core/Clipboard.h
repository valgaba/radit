#ifndef CLIPBOARD_H
#define CLIPBOARD_H


#include <QList>
#include <QWidget>
#include <QObject>
#include <QDebug>


class Clipboard : public QObject {
    Q_OBJECT

public:
    static Clipboard& instance();  // Singleton para acceder a la instancia global

   // void copyWidget(QWidget *widget);  // Copia un widget al portapapeles
   // QWidget* pasteWidget();  // Pega el último widget copiado
  //  void clearClipboard();   // Limpia el portapapeles
    QList<QWidget*> lista;  // Lista de widgets copiados



private:

    explicit Clipboard(QObject *parent = nullptr);  // Constructor privado (Singleton)
    Clipboard(const Clipboard&) = delete;  // Evita copia
    Clipboard& operator=(const Clipboard&) = delete;  // Evita asignación


};


#endif // CLIPBOARD_H
