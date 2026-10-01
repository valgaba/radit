#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

class Player;

class Config{

public:

    static bool saveConfig(const QString& filename,
                             const QList<Player*>& players);

    static bool loadConfig(const QString& filename,
                             const QList<Player*>& players);
};



#endif // CONFIG_H
