#ifndef IO_H
#define IO_H




#include <QObject>
#include <QDebug>
#include <QLayout>

class ContentsBase;


class Io: public QObject {
    Q_OBJECT

public:

    explicit Io(QObject *parent = nullptr);
    ~Io();


    void saveContentsPlayer(QLayout* layout, const QString& filename);
    void loadContentsPlayer(ContentsBase* contents, const QString& filename);

private:



};





#endif // IO_H
