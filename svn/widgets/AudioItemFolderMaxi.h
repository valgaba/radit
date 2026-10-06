#ifndef AUDIOITEMFOLDERMAXI_H
#define AUDIOITEMFOLDERMAXI_H
#include "widgets/AudioItemMaxi.h"
#include <QVector>
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
    AudioItemMaxi *copy(QWidget *newParent) const override;
    bool preparePlayback() override;
    bool advancesOnLoop() const override { return true; }
    QString playbackPath() const override { return m_currentTrack; }
    QString playbackName() const override;
signals:
    void scanFinished();
private:
    QVector<Track> m_tracks;
    QVector<int> m_remaining;
    QString m_currentTrack, m_lastTrack;
    bool m_scanning = false;
    std::shared_ptr<std::atomic_bool> m_cancel;
};
#endif
