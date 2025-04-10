#ifndef IO_H
#define IO_H




#include <QObject>
#include <QDebug>
#include <QLayout>


class Io: public QObject {
    Q_OBJECT

public:

    explicit Io(QObject *parent = nullptr);
    ~Io();


  void saveWidgetsToJson(QLayout* layout, const QString& filename);
  void loadWidgetsFromJson(QLayout* layout, const QString& filename);
private:



};





#endif // IO_H
