#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QList>

class Player;
class Capture;

class Config{

public:

    static bool saveConfig(const QString& filename,
                             const QList<Player*>& players, Capture *capture = nullptr, QString *error = nullptr);

    static bool loadConfig(const QString& filename,
                             const QList<Player*>& players, Capture *capture = nullptr, QString *error = nullptr);
};



#endif // CONFIG_H
