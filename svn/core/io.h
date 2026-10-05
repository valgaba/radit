#ifndef IO_H
#define IO_H




#include <QObject>
#include <QDebug>
#include <QLayout>

class ContentsBase;
class TabPlayer;


class Io: public QObject {
    Q_OBJECT

public:

    explicit Io(QObject *parent = nullptr);
    ~Io();


    bool SaveListPlayer(QLayout* layout, const QString& filename, QString* error = nullptr);
    bool LoadListPlayer(ContentsBase* contents, const QString& filename, QString* error = nullptr);
    bool SavePlayer(TabPlayer* player, const QString& filename, QString* error = nullptr);
    bool LoadPlayer(TabPlayer* player, const QString& filename, QString* error = nullptr);

private:



};





#endif // IO_H
