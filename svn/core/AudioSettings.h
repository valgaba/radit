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

    static QString scheduleTransitionFolder()
    {
        QSettings settings(settingsFile(), QSettings::IniFormat);
        return settings.value(QStringLiteral("Audio/scheduleTransitionFolder")).toString();
    }

    static void setScheduleTransitionFolder(const QString &path)
    {
        QSettings settings(settingsFile(), QSettings::IniFormat);
        settings.setValue(QStringLiteral("Audio/scheduleTransitionFolder"),
                          path.trimmed().isEmpty()
                              ? QString()
                              : QDir::cleanPath(QFileInfo(path).absoluteFilePath()));
        settings.sync();
    }

    static bool scheduleTransitionsEnabled()
    {
        QSettings settings(settingsFile(), QSettings::IniFormat);
        return settings.value(QStringLiteral("Audio/scheduleTransitionsEnabled"), false).toBool();
    }

    static void setScheduleTransitionsEnabled(bool enabled)
    {
        QSettings settings(settingsFile(), QSettings::IniFormat);
        settings.setValue(QStringLiteral("Audio/scheduleTransitionsEnabled"), enabled);
        settings.sync();
    }

    static int minimumStandbyTrackSeconds()
    {
        QSettings settings(settingsFile(), QSettings::IniFormat);
        return qBound(1, settings.value(QStringLiteral("Audio/minimumStandbyTrackSeconds"), 30).toInt(), 600);
    }

    static void setMinimumStandbyTrackSeconds(int seconds)
    {
        QSettings settings(settingsFile(), QSettings::IniFormat);
        settings.setValue(QStringLiteral("Audio/minimumStandbyTrackSeconds"), qBound(1, seconds, 600));
        settings.sync();
    }

    static int scheduleTransitionLateToleranceSeconds()
    {
        QSettings settings(settingsFile(), QSettings::IniFormat);
        return qBound(0, settings.value(QStringLiteral("Audio/scheduleTransitionLateToleranceSeconds"), 5).toInt(), 60);
    }

    static void setScheduleTransitionLateToleranceSeconds(int seconds)
    {
        QSettings settings(settingsFile(), QSettings::IniFormat);
        settings.setValue(QStringLiteral("Audio/scheduleTransitionLateToleranceSeconds"), qBound(0, seconds, 60));
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
