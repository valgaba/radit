#ifndef AUDIOSETTINGS_H
#define AUDIOSETTINGS_H

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

class AudioSettings
{
public:
    static QString sweeperFolder()
    {
        QSettings settings(settingsFile(), QSettings::IniFormat);
        return settings.value(QStringLiteral("Audio/sweeperFolder")).toString();
    }

    static void setSweeperFolder(const QString &path)
    {
        QSettings settings(settingsFile(), QSettings::IniFormat);
        settings.setValue(QStringLiteral("Audio/sweeperFolder"),
                          path.trimmed().isEmpty()
                              ? QString()
                              : QDir::cleanPath(QFileInfo(path).absoluteFilePath()));
        settings.sync();
    }

private:
    static QString settingsFile()
    {
        return QDir(QCoreApplication::applicationDirPath())
            .filePath(QStringLiteral("interface.ini"));
    }
};

#endif
