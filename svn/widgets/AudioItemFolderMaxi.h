#ifndef AUDIOITEMFOLDERMAXI_H
#define AUDIOITEMFOLDERMAXI_H
#include "widgets/AudioItemMaxi.h"
#include <QVector>
#include <QPointer>
#include <QHash>
#include <QSet>
#include <atomic>
#include <memory>

class AudioItemFolderMaxi : public AudioItemMaxi
{
    Q_OBJECT
public:
    struct Track { QString path; double seconds = 0; };
    explicit AudioItemFolderMaxi(QWidget *parent = nullptr);
    ~AudioItemFolderMaxi() override;
    void setFolderPath(const QString &path);
    QString folderPath() const { return filePath(); }
    int fileCount() const { return m_tracks.size(); }
    bool isScanning() const { return m_scanning; }
    bool mixEnabled() const { return m_mixEnabled; }
    double mixSeconds() const { return m_mixSeconds; }
    void setMixSettings(bool enabled, double seconds);
    AudioItemMaxi *copy(QWidget *newParent) const override;
    bool preparePlayback() override;
    bool advancesOnLoop() const override { return true; }
    QString playbackPath() const override { return m_currentTrack; }
    QString playbackName() const override;
signals:
    void scanFinished();
private:
    friend class Player;
    struct Sequence {
        QHash<QString, Track> tracks;
        QStringList remaining;
        QSet<QString> played;
        QString lastTrack;
        quint64 revision = 0;
    };
    static std::shared_ptr<Sequence> sequenceForFolder(const QString &path);
    static QString trackKey(const QString &path);
    void updateSequence(const QVector<Track> &tracks);
    QVector<Track> m_tracks;
    std::shared_ptr<Sequence> m_sequence;
    QString m_currentTrack;
    bool m_scanning = false;
    bool m_mixEnabled = false;
    double m_mixSeconds = 3.0;
    QPointer<Frame> m_propertiesFrame;
    std::shared_ptr<std::atomic_bool> m_cancel;
};
#endif
