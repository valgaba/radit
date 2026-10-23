#ifndef AUDIOITEMFOLDERMAXI_H
#define AUDIOITEMFOLDERMAXI_H
#include "widgets/AudioItemMaxi.h"
#include <QDateTime>
#include <QPair>
#include <QVector>
#include <QPointer>
#include <QHash>
#include <QSet>
#include <QTimer>
#include <atomic>
#include <memory>

class AudioItemFolderMaxi : public AudioItemMaxi
{
    Q_OBJECT
public:
    struct Track { QString path; double seconds = 0; QString artist; };
    explicit AudioItemFolderMaxi(QWidget *parent = nullptr);
    ~AudioItemFolderMaxi() override;
    void setFolderPath(const QString &path);
    bool setOrderedListPath(const QString &path, QString *error = nullptr);
    bool refreshOrderedList(QString *error = nullptr);
    QStringList orderedTrackPaths() const;
    QStringList indexedTrackPaths() const;
    double orderedListDurationSeconds(bool *known = nullptr) const;
    QPair<int, int> updateOrderedTrackDurations(const QHash<QString, double> &durations);
    bool isOrderedList() const { return m_orderedList; }
    QString orderedListPath() const { return m_orderedList ? m_orderedListPath : QString(); }
    void resetOrderedListPlayback();
    QString folderPath() const { return filePath(); }
    int fileCount() const { return m_tracks.size(); }
    bool isScanning() const { return m_scanning; }
    bool mixEnabled() const override { return m_mixEnabled; }
    double mixSeconds() const override { return m_mixSeconds; }
    void setMixSettings(bool enabled, double seconds) override;
    int songsPerTurn() const { return m_orderedList ? m_tracks.size() : m_songsPerTurn; }
    void setSongsPerTurn(int count) { m_songsPerTurn = qBound(1, count, 999); }
    bool avoidsArtistRepeats() const { return m_avoidArtistRepeats; }
    void setAvoidArtistRepeats(bool enabled) { m_avoidArtistRepeats=enabled; }
    int artistRepeatDistance() const { return m_artistRepeatDistance; }
    void setArtistRepeatDistance(int songs) { m_artistRepeatDistance=qBound(1,songs,50); }
    QString introAnnouncement() const { return m_introAnnouncement; }
    QString outroAnnouncement() const { return m_outroAnnouncement; }
    int announcementDelaySeconds() const { return m_announcementDelaySeconds; }
    void setAnnouncementTexts(const QString &intro, const QString &outro, int delaySeconds = 5);
    bool announcementsEnabled() const { return m_announcementsEnabled; }
    void setAnnouncementsEnabled(bool enabled);
    int announcementVolumePercent() const { return m_announcementVolumePercent; }
    void setAnnouncementVolumePercent(int percent);
    int announcementMusicVolumePercent() const { return m_announcementMusicVolumePercent; }
    void setAnnouncementMusicVolumePercent(int percent);
    double announcementRecoverySeconds() const { return m_announcementRecoverySeconds; }
    void setAnnouncementRecoverySeconds(double seconds);
    bool hasAnnouncements() const { return (m_announcementsEnabled
        && (!m_introAnnouncement.isEmpty() || !m_outroAnnouncement.isEmpty()))
        || m_sweeperEnabled; }
    QString introAnnouncementPath() const { return m_introAudioPath; }
    QString outroAnnouncementPath() const { return m_outroAudioPath; }
    QString resolvedIntroAnnouncement() const;
    QString resolvedOutroAnnouncement() const;
    bool sweeperEnabled() const { return m_sweeperEnabled; }
    QString sweeperIntroAudioPath() const { return m_sweeperIntroAudioPath; }
    QString sweeperOutroAudioPath() const { return m_sweeperOutroAudioPath; }
    bool sweeperAtEntry() const { return m_sweeperAtEntry; }
    bool sweeperAtExit() const { return m_sweeperAtExit; }
    int sweeperEveryNSongs() const { return m_sweeperEveryNSongs; }
    void setSweeper(bool enabled, bool atEntry = true, bool atExit = true);
    void setSweeperEveryNSongs(int songs);
    AudioItemMaxi *copy(QWidget *newParent) const override;
    bool preparePlayback() override;
    bool isPreparingPlayback() const override { return m_waitingForScan || m_announcementPreparing; }
    bool advancesOnLoop() const override { return true; }
    QString playbackPath() const override { return m_currentTrack; }
    QString playbackName() const override;
    bool loadPreparedPlayback(MediaManager *manager) override;
signals:
    void scanFinished();
protected:
    void copyFolderStateFrom(const AudioItemFolderMaxi &source);
private:
    friend class Player;
    struct Sequence {
        QHash<QString, Track> tracks;
        QStringList remaining;
        QSet<QString> played;
        QString lastTrack;
        QStringList recentArtists;
        QHash<QString,QString> artistByTrack;
        QHash<QString, QDateTime> directoryStamps;
        quint64 revision = 0;
        qint64 scannedAtMs = 0;
        qint64 lastChangeCheckMs = 0;
        bool scanComplete = false;
        bool changeCheckInProgress = false;
        bool refreshInProgress = false;
    };
    static std::shared_ptr<Sequence> sequenceForFolder(const QString &path);
    static QString trackKey(const QString &path);
    void updateSequence(const QVector<Track> &tracks);
    void checkFolderForChanges();
    void refreshFolderIndex();
    bool prepareAnnouncementAudio();
    QVector<Track> m_tracks;
    std::shared_ptr<Sequence> m_sequence;
    QString m_currentTrack;
    bool m_scanning = false;
    bool m_orderedList = false;
    QString m_orderedListPath;
    int m_nextOrderedTrack = 0;
    bool m_waitingForScan = false;
    bool m_mixEnabled = false;
    double m_mixSeconds = 3.0;
    int m_songsPerTurn = 1;
    bool m_avoidArtistRepeats = false;
    int m_artistRepeatDistance = 3;
    QString m_introAnnouncement = QStringLiteral("[excited] ¡Sigues en tu emisora favorita! Te dejamos con [artist] y su tema [title].");
    QString m_outroAnnouncement;
    int m_announcementDelaySeconds = 5;
    bool m_announcementsEnabled = false;
    int m_announcementVolumePercent = 140;
    int m_announcementMusicVolumePercent = 8;
    double m_announcementRecoverySeconds = 1.0;
    QString m_introAudioPath, m_outroAudioPath;
    bool m_sweeperEnabled = false;
    bool m_sweeperAtEntry = true;
    bool m_sweeperAtExit = true;
    int m_sweeperEveryNSongs = 1;
    int m_sweeperSongsSinceLast = 0;
    bool m_sweeperDueForCurrentTrack = false;
    QString m_sweeperIntroAudioPath, m_sweeperOutroAudioPath;
    bool m_announcementPreparing = false;
    bool m_trackPrepared = false;
    int m_announcementRequests = 0;
    quint64 m_displayedSequenceRevision = 0;
    QPointer<Frame> m_propertiesFrame;
    std::shared_ptr<std::atomic_bool> m_cancel;
    QTimer m_refreshTimer;
};
#endif
