#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QList>

class Player;
class Capture;
class Planner;

class Config{

public:

    static bool saveConfig(const QString& filename,
                             const QList<Player*>& players, Capture *capture = nullptr,
                             QString *error = nullptr, Planner *planner = nullptr);

    static bool loadConfig(const QString& filename,
                             const QList<Player*>& players, Capture *capture = nullptr,
                             QString *error = nullptr, Planner *planner = nullptr);
};



#endif // CONFIG_H
