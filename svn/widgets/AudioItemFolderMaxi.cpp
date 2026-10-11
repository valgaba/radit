/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/AudioItemFolderMaxi.h"
#include "widgets/Player.h"
#include "widgets/FolderPropertiesFrame.h"
#include "core/ElevenLabsTts.h"
#include <cmath>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QDateTime>
#include <QFutureWatcher>
#include <QRandomGenerator>
#include <QtConcurrent/QtConcurrentRun>

AudioItemFolderMaxi::AudioItemFolderMaxi(QWidget *parent) : AudioItemMaxi(parent)
{
    setObjectName("AudioItemFolderMaxi");
    setSecond(0);
    setNameFile(tr("Audio folder"));
    setFolderPresentation(tr("FOLDER"), false);
    connect(propertiesButton(), &Button::clicked, this, [this]() {
        if (!m_propertiesFrame) {
            m_propertiesFrame = new FolderPropertiesFrame(this);
        }
        m_propertiesFrame->show();
        m_propertiesFrame->raise();
        m_propertiesFrame->activateWindow();
    });
}

AudioItemFolderMaxi::~AudioItemFolderMaxi() { if (m_cancel) m_cancel->store(true); }

void AudioItemFolderMaxi::setMixSettings(bool enabled, double seconds)
{
    if (!std::isfinite(seconds)) return;
    m_mixEnabled = enabled;
    m_mixSeconds = qBound(0.1, seconds, 30.0);
}

void AudioItemFolderMaxi::setAnnouncementTexts(const QString &intro, const QString &outro,
                                               int delaySeconds)
{
    const QString nextIntro = intro.trimmed();
    const QString nextOutro = outro.trimmed();
    if (nextIntro != m_introAnnouncement || nextOutro != m_outroAnnouncement) {
        m_introAudioPath.clear();
        m_outroAudioPath.clear();
    }
    m_introAnnouncement = nextIntro;
    m_outroAnnouncement = nextOutro;
    m_announcementDelaySeconds = qBound(0, delaySeconds, 30);
}

void AudioItemFolderMaxi::setAnnouncementsEnabled(bool enabled)
{
    m_announcementsEnabled = enabled;
    if (!enabled) {
        m_introAudioPath.clear();
        m_outroAudioPath.clear();
    }
}

void AudioItemFolderMaxi::setAnnouncementVolumePercent(int percent)
{
    m_announcementVolumePercent = qBound(0, percent, 200);
}

void AudioItemFolderMaxi::setAnnouncementMusicVolumePercent(int percent)
{
    m_announcementMusicVolumePercent = qBound(0, percent, 100);
}

void AudioItemFolderMaxi::setAnnouncementRecoverySeconds(double seconds)
{
    if (std::isfinite(seconds))
        m_announcementRecoverySeconds = qBound(0.1, seconds, 5.0);
}

QString AudioItemFolderMaxi::resolvedIntroAnnouncement() const
{
    if (m_currentTrack.isEmpty()) return {};
    const AudioFileMetadata metadata=MediaManager::readAudioFileMetadata(m_currentTrack);
    const QString group=metadata.artist.trimmed().isEmpty() ? QDir(folderPath()).dirName()
                                                            : metadata.artist.trimmed();
    const QString title=metadata.title.trimmed().isEmpty()
        ? QFileInfo(m_currentTrack).completeBaseName() : metadata.title.trimmed();
    QString text=m_introAnnouncement;
    text.replace(QStringLiteral("[artist]"),group,Qt::CaseInsensitive);
    text.replace(QStringLiteral("[title]"),title,Qt::CaseInsensitive);
    text.replace(QStringLiteral("[album]"),metadata.album.trimmed(),Qt::CaseInsensitive);
    // Keep templates from older saved lists working.
    text.replace(QStringLiteral("@grupo"),group,Qt::CaseInsensitive);
    text.replace(QStringLiteral("@nombrecancion"),title,Qt::CaseInsensitive);
    return text.trimmed();
}

QString AudioItemFolderMaxi::resolvedOutroAnnouncement() const
{
    if (m_currentTrack.isEmpty()) return {};
    const AudioFileMetadata metadata=MediaManager::readAudioFileMetadata(m_currentTrack);
    const QString group=metadata.artist.trimmed().isEmpty() ? QDir(folderPath()).dirName()
                                                            : metadata.artist.trimmed();
    const QString title=metadata.title.trimmed().isEmpty()
        ? QFileInfo(m_currentTrack).completeBaseName() : metadata.title.trimmed();
    QString text=m_outroAnnouncement;
    text.replace(QStringLiteral("[artist]"),group,Qt::CaseInsensitive);
    text.replace(QStringLiteral("[title]"),title,Qt::CaseInsensitive);
    text.replace(QStringLiteral("[album]"),metadata.album.trimmed(),Qt::CaseInsensitive);
    text.replace(QStringLiteral("@grupo"),group,Qt::CaseInsensitive);
    text.replace(QStringLiteral("@nombrecancion"),title,Qt::CaseInsensitive);
    return text.trimmed();
}

bool AudioItemFolderMaxi::prepareAnnouncementAudio()
{
    if (!m_announcementsEnabled) {
        m_introAudioPath.clear();
        m_outroAudioPath.clear();
        return true;
    }
    if (m_announcementPreparing) return false;
    m_introAudioPath.clear();
    m_outroAudioPath.clear();
    const QString intro=resolvedIntroAnnouncement();
    const QString outro=resolvedOutroAnnouncement();
    m_announcementRequests=int(!intro.isEmpty())+int(!outro.isEmpty());
    if (m_announcementRequests==0) return true;
    m_announcementPreparing=true;
    const auto finished=[this](bool isIntro,const QString &path,const QString &error) {
        if (!m_announcementsEnabled) {
            m_introAudioPath.clear();
            m_outroAudioPath.clear();
            if (--m_announcementRequests <= 0) {
                m_announcementPreparing = false;
                emit playbackPreparationReady(true);
            }
            return;
        }
        if (!path.isEmpty()) {
            if (isIntro) m_introAudioPath=path;
            else m_outroAudioPath=path;
        } else if (!error.isEmpty()) {
            setToolTip(folderPath()+QStringLiteral("\n")+error);
        }
        if (--m_announcementRequests<=0) {
            m_announcementPreparing=false;
            emit playbackPreparationReady(true); // Keep the music going if TTS is unavailable.
        }
    };
    if (!intro.isEmpty()) ElevenLabsTts::instance()->generate(intro,this,
        [finished](const QString &path,const QString &error) { finished(true,path,error); });
    if (!outro.isEmpty()) ElevenLabsTts::instance()->generate(outro,this,
        [finished](const QString &path,const QString &error) { finished(false,path,error); });
    return false;
}

QString AudioItemFolderMaxi::trackKey(const QString &path)
{
    QString key = QDir::cleanPath(QDir::fromNativeSeparators(path));
#ifdef Q_OS_WIN
    key = key.toCaseFolded();
#endif
    return key;
}

std::shared_ptr<AudioItemFolderMaxi::Sequence> AudioItemFolderMaxi::sequenceForFolder(const QString &path)
{
    // Accessed only by widgets on the GUI thread; scans publish their results there.
    static QHash<QString, std::weak_ptr<Sequence>> sequences;
    for (auto it = sequences.begin(); it != sequences.end(); ) {
        if (it.value().expired()) it = sequences.erase(it);
        else ++it;
    }
    const QFileInfo info(path);
    const QString canonical = info.canonicalFilePath();
    const QString key = trackKey(canonical.isEmpty() ? info.absoluteFilePath() : canonical);
    auto sequence = sequences.value(key).lock();
    if (!sequence) {
        sequence = std::make_shared<Sequence>();
        sequences.insert(key, sequence);
    }
    return sequence;
}

void AudioItemFolderMaxi::updateSequence(const QVector<Track> &tracks)
{
    QHash<QString, Track> available;
    for (const auto &track : tracks) available.insert(trackKey(track.path), track);
    QSet<QString> played;
    for (const auto &key : m_sequence->played)
        if (available.contains(key)) played.insert(key);
    QStringList remaining;
    QSet<QString> queued;
    for (const auto &key : m_sequence->remaining) {
        if (available.contains(key) && !played.contains(key) && !queued.contains(key)) {
            remaining.append(key); queued.insert(key);
        }
    }
    for (auto it = available.cbegin(); it != available.cend(); ++it) {
        if (!played.contains(it.key()) && !queued.contains(it.key())) remaining.append(it.key());
    }
    m_sequence->tracks = available;
    m_sequence->played = played;
    m_sequence->remaining = remaining;
    ++m_sequence->revision;
}

void AudioItemFolderMaxi::setFolderPath(const QString &path)
{
    if (m_cancel) m_cancel->store(true);
    m_cancel = std::make_shared<std::atomic_bool>(false);
    const auto cancel = m_cancel;
    const QString folder = QDir::cleanPath(QFileInfo(path).absoluteFilePath());
    m_sequence = sequenceForFolder(folder);
    setFilePath(folder);
    const QString name = QDir(folder).dirName();
    setNameFile(name.isEmpty() ? QDir::toNativeSeparators(folder) : name);
    setToolTip(folder);
    m_tracks.clear(); m_currentTrack.clear();
    setSecond(0); setSecondStart(0);

    // Reuse a recent in-memory index when another item points at the same folder.
    // Missing files are discarded lazily by preparePlayback().
    constexpr qint64 cacheLifetimeMs = 2 * 60 * 1000;
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    const qint64 cacheAgeMs = nowMs - (m_sequence ? m_sequence->scannedAtMs : 0);
    if (m_sequence && m_sequence->scanComplete
        && cacheAgeMs >= 0 && cacheAgeMs < cacheLifetimeMs) {
        m_tracks = m_sequence->tracks.values().toVector();
        m_scanning = false;
        setFolderPresentation(tr("%1 files").arg(m_tracks.size()), !m_tracks.isEmpty());
        setToolTip(tr("%1\n%2 audio files — random playback without repeats")
                       .arg(folder).arg(m_tracks.size()));
        emit scanFinished();
        return;
    }

    m_scanning = true;
    setFolderPresentation(tr("Loading..."), false);
    const QStringList filters = MediaManager::supportedAudioNameFilters();
    auto *watcher = new QFutureWatcher<QVector<Track>>(this);
    connect(watcher, &QFutureWatcher<QVector<Track>>::finished, this, [this, watcher, cancel, folder]() {
        const auto tracks = watcher->result();
        watcher->deleteLater();
        if (cancel != m_cancel || cancel->load()) return;
        m_tracks = tracks; m_scanning = false;
        updateSequence(tracks);
        m_sequence->scannedAtMs = QDateTime::currentMSecsSinceEpoch();
        m_sequence->scanComplete = true;
        setFolderPresentation(tr("%1 files").arg(m_tracks.size()), !m_tracks.isEmpty());
        setToolTip(tr("%1\n%2 audio files — random playback without repeats").arg(folder).arg(m_tracks.size()));
        emit scanFinished();
    });
    watcher->setFuture(QtConcurrent::run([folder, filters, cancel]() {
        QVector<Track> tracks;
        QDirIterator files(folder, filters, QDir::Files | QDir::Readable | QDir::NoSymLinks,
                           QDirIterator::Subdirectories);
        while (!cancel->load() && files.hasNext()) {
            files.next();
            // Enumerate only. Opening every file with BASS to read its duration
            // makes network folders with thousands of entries take minutes.
            const QString path = QDir::cleanPath(files.filePath());
            if (!path.isEmpty()) tracks.append({path, 0.0});
        }
        return tracks;
    }));
}

bool AudioItemFolderMaxi::preparePlayback()
{
    if (m_trackPrepared) return !m_announcementPreparing;
    if (m_scanning || !m_sequence || m_sequence->tracks.isEmpty()) return false;
    auto &sequence = *m_sequence;
    if (sequence.remaining.isEmpty()) {
        sequence.played.clear();
        sequence.remaining = sequence.tracks.keys();
    }
    ++sequence.revision;
    while (!sequence.remaining.isEmpty()) {
        int index = QRandomGenerator::global()->bounded(int(sequence.remaining.size()));
        // Also avoid an immediate repeat at the boundary between two cycles.
        if (sequence.remaining.size() > 1 && sequence.remaining[index] == sequence.lastTrack)
            index = (index + 1 + QRandomGenerator::global()->bounded(int(sequence.remaining.size()) - 1)) % sequence.remaining.size();
        const QString key = sequence.remaining.takeAt(index);
        const Track track = sequence.tracks.value(key);
        if (!QFileInfo(track.path).isFile()) {
            sequence.tracks.remove(key); sequence.played.remove(key);
            continue;
        }
        sequence.played.insert(key);
        m_currentTrack = track.path; sequence.lastTrack = key;
        setSecond(track.seconds); setTiempoFile(track.seconds); setSecondStart(0);
        m_trackPrepared=true;
        return prepareAnnouncementAudio();
    }
    setFolderPresentation(tr("No files"), false);
    return false;
}

bool AudioItemFolderMaxi::loadPreparedPlayback(MediaManager *manager)
{
    const bool loaded=manager->loadFile(m_currentTrack);
    m_trackPrepared=false;
    if (!loaded) return false;
    setSecond(manager->getDuration());
    setTiempoFile(second());
    return true;
}

QString AudioItemFolderMaxi::playbackName() const
{
    return m_currentTrack.isEmpty() ? nameFile() :
        nameFile() + " — " + QFileInfo(m_currentTrack).completeBaseName();
}

AudioItemMaxi *AudioItemFolderMaxi::copy(QWidget *newParent) const
{
    auto *item = new AudioItemFolderMaxi(newParent);
    item->setMixSettings(m_mixEnabled, m_mixSeconds);
    item->setAnnouncementTexts(m_introAnnouncement, m_outroAnnouncement,
                               m_announcementDelaySeconds);
    item->setAnnouncementsEnabled(m_announcementsEnabled);
    item->setAnnouncementVolumePercent(m_announcementVolumePercent);
    item->setAnnouncementRecoverySeconds(m_announcementRecoverySeconds);
    item->setIsPurge(isPurge()); item->setIsPlayNext(isPlayNext());
    item->setIsLoop(isLoop()); item->setIsSelect(isSelect()); item->setColor(color());
    if (m_scanning) item->setFolderPath(folderPath());
    else {
        item->setFilePath(folderPath()); item->setNameFile(nameFile());
        item->m_tracks = m_tracks; item->m_sequence = m_sequence;
        item->setToolTip(toolTip());
        item->setFolderPresentation(tr("%1 files").arg(m_tracks.size()), !m_tracks.isEmpty());
    }
    return item;
}
