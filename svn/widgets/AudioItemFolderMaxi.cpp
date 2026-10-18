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
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>
#include <utility>
#include <QDateTime>
#include <QFutureWatcher>
#include <QRandomGenerator>
#include <QSet>
#include <QtConcurrent/QtConcurrentRun>
#ifdef Q_OS_WIN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {
struct SweeperSequence {
    QStringList files;
    QStringList remaining;
    QString lastFile;
};

QString nextRandomSweeper(const QString &folderPath)
{
    const QFileInfo folderInfo(folderPath);
    QString key=folderInfo.canonicalFilePath();
    if (key.isEmpty())
        key=folderInfo.absoluteFilePath();
    key=QDir::cleanPath(QDir::fromNativeSeparators(key));
#ifdef Q_OS_WIN
    key=key.toCaseFolded();
#endif

    QStringList files;
    const QFileInfoList entries=QDir(folderPath).entryInfoList(
        MediaManager::supportedAudioNameFilters(),QDir::Files | QDir::Readable,QDir::Name);
    files.reserve(entries.size());
    for (const QFileInfo &entry : entries)
        files.append(entry.absoluteFilePath());
    if (files.isEmpty())
        return {};

    // Like folder-track selection, sweeper rotation is shared by copies of the
    // same configured directory and is accessed from the GUI thread.
    static QHash<QString,SweeperSequence> sequences;
    SweeperSequence &sequence=sequences[key];
    if (sequence.files!=files) {
        sequence.files=files;
        sequence.remaining=files;
        sequence.lastFile.clear();
    }
    if (sequence.remaining.isEmpty()) {
        sequence.remaining=sequence.files;
        for (int i=sequence.remaining.size()-1;i>0;--i) {
            const int j=QRandomGenerator::global()->bounded(i+1);
            sequence.remaining.swapItemsAt(i,j);
        }
        if (sequence.remaining.size()>1 && sequence.remaining.constFirst()==sequence.lastFile) {
            const int other=1+QRandomGenerator::global()->bounded(sequence.remaining.size()-1);
            sequence.remaining.swapItemsAt(0,other);
        }
    }

    const QString selected=sequence.remaining.takeFirst();
    sequence.lastFile=selected;
    return selected;
}

struct FolderScan {
    QVector<AudioItemFolderMaxi::Track> tracks;
    QHash<QString, QDateTime> directoryStamps;
    bool rootReadable = false;
};

#ifdef Q_OS_WIN
QDateTime fileTimeDateTime(const FILETIME &fileTime)
{
    ULARGE_INTEGER ticks{};
    ticks.LowPart = fileTime.dwLowDateTime;
    ticks.HighPart = fileTime.dwHighDateTime;
    constexpr quint64 windowsToUnixEpochMs = 11644473600000ULL;
    const quint64 fileTimeMs = ticks.QuadPart / 10000ULL;
    if (fileTimeMs < windowsToUnixEpochMs)
        return {};
    return QDateTime::fromMSecsSinceEpoch(qint64(fileTimeMs - windowsToUnixEpochMs), Qt::UTC);
}
#endif

FolderScan enumerateAudioTracks(
    const QString &folder, const QStringList &filters,
    const std::shared_ptr<std::atomic_bool> &cancel)
{
    FolderScan result;
    const QFileInfo rootInfo(folder);
    if (!rootInfo.isDir() || !rootInfo.isReadable())
        return result;
    result.rootReadable = true;
    const QDateTime rootStamp = rootInfo.lastModified();
    if (rootStamp.isValid())
        result.directoryStamps.insert(QDir::cleanPath(folder), rootStamp);
#ifdef Q_OS_WIN
    QSet<QString> extensions;
    for (const QString &filter : filters) {
        const qsizetype dot = filter.lastIndexOf(QLatin1Char('.'));
        if (dot >= 0)
            extensions.insert(filter.mid(dot).toCaseFolded());
    }

    // Enumerate each directory in batches. LARGE_FETCH can reduce round trips
    // to SMB servers for folders containing thousands of entries.
    QVector<QString> directories{folder};
    for (qsizetype directoryIndex = 0;
         directoryIndex < directories.size() && !cancel->load(); ++directoryIndex) {
        const QString searchPath = QDir::toNativeSeparators(
            QDir(directories.at(directoryIndex)).filePath(QStringLiteral("*")));
        const std::wstring nativeSearchPath = searchPath.toStdWString();
        WIN32_FIND_DATAW entry{};
        HANDLE search = FindFirstFileExW(nativeSearchPath.c_str(), FindExInfoBasic,
                                         &entry, FindExSearchNameMatch, nullptr,
                                         FIND_FIRST_EX_LARGE_FETCH);
        DWORD searchError = search == INVALID_HANDLE_VALUE ? GetLastError() : ERROR_SUCCESS;
        if (search == INVALID_HANDLE_VALUE
            && (searchError == ERROR_INVALID_PARAMETER || searchError == ERROR_NOT_SUPPORTED)) {
            search = FindFirstFileExW(nativeSearchPath.c_str(), FindExInfoBasic,
                                      &entry, FindExSearchNameMatch, nullptr, 0);
            searchError = search == INVALID_HANDLE_VALUE ? GetLastError() : ERROR_SUCCESS;
        }
        if (search == INVALID_HANDLE_VALUE
            && (searchError == ERROR_INVALID_PARAMETER || searchError == ERROR_NOT_SUPPORTED))
            search = FindFirstFileExW(nativeSearchPath.c_str(), FindExInfoStandard,
                                      &entry, FindExSearchNameMatch, nullptr, 0);
        if (search == INVALID_HANDLE_VALUE)
            continue;

        do {
            if (cancel->load())
                break;
            const QString name = QString::fromWCharArray(entry.cFileName);
            if (name == QLatin1String(".") || name == QLatin1String(".."))
                continue;
            if (entry.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
                continue;

            const QString path = QDir::cleanPath(
                QDir(directories.at(directoryIndex)).filePath(name));
            if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                const QDateTime stamp = fileTimeDateTime(entry.ftLastWriteTime);
                if (stamp.isValid())
                    result.directoryStamps.insert(path, stamp);
                directories.append(path);
            } else {
                const QString suffix = QLatin1Char('.')
                    + QFileInfo(name).suffix().toCaseFolded();
                if (extensions.contains(suffix))
                    result.tracks.append({path, 0.0});
            }
        } while (FindNextFileW(search, &entry));
        FindClose(search);
    }
#else
    QDirIterator files(folder, filters, QDir::Files | QDir::Readable | QDir::NoSymLinks,
                       QDirIterator::Subdirectories);
    while (!cancel->load() && files.hasNext()) {
        files.next();
        const QString path = QDir::cleanPath(files.filePath());
        if (path.isEmpty()) continue;
        const QFileInfo info = files.fileInfo();
        if (info.isDir()) {
            const QDateTime stamp = info.lastModified();
            if (stamp.isValid())
                result.directoryStamps.insert(path, stamp);
        } else {
            result.tracks.append({path, 0.0});
        }
    }
#endif
    return result;
}
}

AudioItemFolderMaxi::AudioItemFolderMaxi(QWidget *parent) : AudioItemMaxi(parent)
{
    setObjectName("AudioItemFolderMaxi");
    setSecond(0);
    setNameFile(tr("Audio folder"));
    setFolderPresentation(tr("FOLDER"), false);
    m_refreshTimer.setInterval(45000);
    connect(&m_refreshTimer, &QTimer::timeout, this, [this]() { checkFolderForChanges(); });
    m_refreshTimer.start();
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

void AudioItemFolderMaxi::setSweeper(bool enabled, const QString &folderPath,
                                     bool atEntry, bool atExit)
{
    m_sweeperFolderPath = folderPath.trimmed().isEmpty()
        ? QString() : QDir::cleanPath(QFileInfo(folderPath).absoluteFilePath());
    m_sweeperEnabled = enabled;
    m_sweeperAtEntry = atEntry;
    m_sweeperAtExit = atExit;
    m_sweeperIntroAudioPath.clear();
    m_sweeperOutroAudioPath.clear();
}

bool AudioItemFolderMaxi::prepareAnnouncementAudio()
{
    if (m_announcementPreparing) return false;
    m_introAudioPath.clear();
    m_outroAudioPath.clear();
    const QString intro = m_announcementsEnabled ? resolvedIntroAnnouncement() : QString();
    const QString outro = m_announcementsEnabled ? resolvedOutroAnnouncement() : QString();
    m_sweeperIntroAudioPath.clear();
    m_sweeperOutroAudioPath.clear();
    if (m_sweeperEnabled && !m_sweeperFolderPath.isEmpty()) {
        if (m_sweeperAtEntry)
            m_sweeperIntroAudioPath=nextRandomSweeper(m_sweeperFolderPath);
        if (m_sweeperAtExit)
            m_sweeperOutroAudioPath=nextRandomSweeper(m_sweeperFolderPath);
    }
    m_announcementRequests = int(!intro.isEmpty()) + int(!outro.isEmpty());
    if (m_announcementRequests==0) return true;
    m_announcementPreparing=true;
    const auto finished=[this](int type,const QString &path,const QString &error) {
        if (!m_announcementsEnabled) {
            m_introAudioPath.clear();
            m_outroAudioPath.clear();
        }
        if (!path.isEmpty()) {
            if (type == 0 && m_announcementsEnabled) m_introAudioPath = path;
            else if (type == 1 && m_announcementsEnabled) m_outroAudioPath = path;
        } else if (!error.isEmpty()) {
            setToolTip(folderPath()+QStringLiteral("\n")+error);
        }
        if (--m_announcementRequests<=0) {
            m_announcementPreparing=false;
            emit playbackPreparationReady(true); // Keep the music going if TTS is unavailable.
        }
    };
    if (!intro.isEmpty()) ElevenLabsTts::instance()->generate(intro,this,
        [finished](const QString &path,const QString &error) { finished(0,path,error); });
    if (!outro.isEmpty()) ElevenLabsTts::instance()->generate(outro,this,
        [finished](const QString &path,const QString &error) { finished(1,path,error); });
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
    m_orderedList = false;
    m_orderedListPath.clear();
    m_nextOrderedTrack = 0;
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

    // Keep a completed in-memory index available. A lightweight background
    // timestamp check below schedules a refresh if the directory tree changes.
    if (m_sequence && m_sequence->scanComplete) {
        m_tracks = m_sequence->tracks.values().toVector();
        m_displayedSequenceRevision = m_sequence->revision;
        m_scanning = false;
        setFolderPresentation(tr("%1 files").arg(m_tracks.size()), !m_tracks.isEmpty());
        setToolTip(tr("%1\n%2 audio files — random playback without repeats")
                       .arg(folder).arg(m_tracks.size()));
        emit scanFinished();
        if (m_waitingForScan) {
            m_waitingForScan = false;
            const bool ready = preparePlayback();
            if (ready)
                emit playbackPreparationReady(true);
            else if (!isPreparingPlayback())
                emit playbackPreparationReady(false);
        }
        return;
    }

    m_scanning = true;
    setFolderPresentation(QString(), false);
    setLoadingPresentation(true);
    const QStringList filters = MediaManager::supportedAudioNameFilters();
    auto *watcher = new QFutureWatcher<FolderScan>(this);
    connect(watcher, &QFutureWatcher<FolderScan>::finished, this, [this, watcher, cancel, folder]() {
        const FolderScan scan = watcher->result();
        watcher->deleteLater();
        if (cancel != m_cancel || cancel->load()) return;
        m_scanning = false;
        if (scan.rootReadable) {
            updateSequence(scan.tracks);
            m_sequence->directoryStamps = scan.directoryStamps;
            m_sequence->scannedAtMs = QDateTime::currentMSecsSinceEpoch();
            m_sequence->scanComplete = true;
        }
        m_tracks = m_sequence->tracks.values().toVector();
        m_displayedSequenceRevision = m_sequence->revision;
        setFolderPresentation(tr("%1 files").arg(m_tracks.size()), !m_tracks.isEmpty());
        setToolTip(tr("%1\n%2 audio files — random playback without repeats").arg(folder).arg(m_tracks.size()));
        emit scanFinished();
        if (m_waitingForScan) {
            m_waitingForScan = false;
            const bool ready = preparePlayback();
            if (ready)
                emit playbackPreparationReady(true);
            else if (!isPreparingPlayback())
                emit playbackPreparationReady(false);
        }
    });
    watcher->setFuture(QtConcurrent::run([folder, filters, cancel]() {
        return enumerateAudioTracks(folder, filters, cancel);
    }));
}

bool AudioItemFolderMaxi::setOrderedListPath(const QString &path, QString *error)
{
    if (error) error->clear();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) *error = tr("Cannot read list: %1").arg(file.errorString());
        return false;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()
        || !document.object().value("items").isArray()) {
        if (error) *error = tr("This Radit list does not contain a playable item sequence.");
        return false;
    }

    QVector<Track> tracks;
    const QJsonArray items = document.object().value("items").toArray();
    for (const QJsonValue &value : items) {
        if (!value.isObject()) continue;
        const QJsonObject object = value.toObject();
        if (object.value("type").toString() != QLatin1String("file")) {
            if (error) *error = tr("Ordered Radit lists can contain audio files only.");
            return false;
        }
        QString audioPath = object.value("url").toString();
        if (audioPath.isEmpty()) continue;
        if (QDir::isRelativePath(audioPath))
            audioPath = QFileInfo(path).dir().absoluteFilePath(audioPath);
        audioPath = QDir::cleanPath(QFileInfo(audioPath).absoluteFilePath());
        if (!QFileInfo(audioPath).isFile()) continue;
        const double seconds = object.value("second").toDouble();
        tracks.append({audioPath, std::isfinite(seconds) && seconds > 0.0 ? seconds : 0.0});
    }
    if (tracks.isEmpty()) {
        if (error) *error = tr("The Radit list contains no available audio files.");
        return false;
    }

    if (m_cancel) m_cancel->store(true);
    m_cancel.reset();
    m_sequence.reset();
    m_scanning = false;
    m_waitingForScan = false;
    m_trackPrepared = false;
    m_currentTrack.clear();
    m_orderedList = true;
    m_orderedListPath = QDir::cleanPath(QFileInfo(path).absoluteFilePath());
    m_nextOrderedTrack = 0;
    m_tracks = std::move(tracks);
    double totalSeconds = 0.0;
    for (const Track &track : m_tracks) totalSeconds += track.seconds;
    setFilePath(m_orderedListPath);
    setNameFile(QFileInfo(path).completeBaseName());
    setSecond(totalSeconds);
    setTiempoFile(totalSeconds);
    setSecondStart(0);
    setFolderPresentation(tr("%1 files").arg(m_tracks.size()), true);
    setToolTip(tr("%1\n%2 audio files — ordered Radit list")
                   .arg(m_orderedListPath).arg(m_tracks.size()));
    emit scanFinished();
    return true;
}

bool AudioItemFolderMaxi::refreshOrderedList(QString *error)
{
    if (!m_orderedList || m_orderedListPath.isEmpty()) {
        if (error) *error=tr("This item is not a Radit list.");
        return false;
    }

    const QString path=m_orderedListPath;
    const QVector<Track> oldTracks=m_tracks;
    const int oldNextTrack=m_nextOrderedTrack;
    const QString currentTrack=m_currentTrack;
    const bool currentTrackActive=isPlaying() || m_trackPrepared;
    const bool trackPrepared=m_trackPrepared;
    QString nextTrack;
    if (oldNextTrack>=0 && oldNextTrack<oldTracks.size())
        nextTrack=oldTracks.at(oldNextTrack).path;

    if (!setOrderedListPath(path,error))
        return false;

    if (currentTrackActive) {
        const auto current=std::find_if(m_tracks.cbegin(),m_tracks.cend(),
            [&currentTrack](const Track &track) { return track.path==currentTrack; });
        if (current!=m_tracks.cend()) {
            m_nextOrderedTrack=int(current-m_tracks.cbegin())+1;
            setSecond(current->seconds);
            setTiempoFile(current->seconds);
        } else {
            const auto next=std::find_if(m_tracks.cbegin(),m_tracks.cend(),
                [&nextTrack](const Track &track) { return track.path==nextTrack; });
            m_nextOrderedTrack=next!=m_tracks.cend() ? int(next-m_tracks.cbegin())
                : qBound(0,oldNextTrack,m_tracks.size());
        }
        m_currentTrack=currentTrack;
        m_trackPrepared=trackPrepared;
    } else {
        const auto next=std::find_if(m_tracks.cbegin(),m_tracks.cend(),
            [&nextTrack](const Track &track) { return track.path==nextTrack; });
        m_nextOrderedTrack=next!=m_tracks.cend() ? int(next-m_tracks.cbegin())
            : qBound(0,oldNextTrack,m_tracks.size());
        m_currentTrack.clear();
        m_trackPrepared=false;
    }
    return true;
}

QStringList AudioItemFolderMaxi::orderedTrackPaths() const
{
    QStringList paths;
    if (!m_orderedList)
        return paths;
    paths.reserve(m_tracks.size());
    for (const Track &track : m_tracks)
        paths.append(track.path);
    return paths;
}

QPair<int, int> AudioItemFolderMaxi::updateOrderedTrackDurations(
    const QHash<QString, double> &durations)
{
    if (!m_orderedList)
        return {0,0};

    double totalSeconds=0.0;
    int refreshed=0;
    int failed=0;
    for (Track &track : m_tracks) {
        const auto duration=durations.constFind(track.path);
        if (duration!=durations.cend() && std::isfinite(duration.value())
            && duration.value()>0.0) {
            track.seconds=duration.value();
            ++refreshed;
        } else {
            ++failed;
        }
        totalSeconds+=track.seconds;
    }

    const bool currentTrackActive=isPlaying() || m_trackPrepared;
    if (currentTrackActive) {
        const auto current=std::find_if(m_tracks.cbegin(),m_tracks.cend(),
            [this](const Track &track) { return track.path==m_currentTrack; });
        if (current!=m_tracks.cend()) {
            setSecond(current->seconds);
            setTiempoFile(current->seconds);
        }
    } else {
        setSecond(totalSeconds);
        setTiempoFile(totalSeconds);
    }
    setFolderPresentation(tr("%1 files").arg(m_tracks.size()),!m_tracks.isEmpty());
    return {refreshed,failed};
}

void AudioItemFolderMaxi::resetOrderedListPlayback()
{
    if (!m_orderedList) return;
    m_nextOrderedTrack = 0;
    m_trackPrepared = false;
    m_currentTrack.clear();
}

void AudioItemFolderMaxi::checkFolderForChanges()
{
    if (!m_sequence || !m_sequence->scanComplete || m_scanning
        || m_trackPrepared || m_announcementPreparing)
        return;

    if (m_displayedSequenceRevision != m_sequence->revision) {
        m_tracks = m_sequence->tracks.values().toVector();
        m_displayedSequenceRevision = m_sequence->revision;
        setFolderPresentation(tr("%1 files").arg(m_tracks.size()), !m_tracks.isEmpty());
        setToolTip(tr("%1\n%2 audio files — random playback without repeats")
                       .arg(folderPath()).arg(m_tracks.size()));
    }

    const auto sequence = m_sequence;
    if (sequence->directoryStamps.isEmpty() || sequence->refreshInProgress
        || sequence->changeCheckInProgress)
        return;

    constexpr qint64 checkIntervalMs = 45000;
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    if (nowMs - sequence->lastChangeCheckMs < checkIntervalMs)
        return;
    sequence->lastChangeCheckMs = nowMs;
    sequence->changeCheckInProgress = true;

    const auto stamps = sequence->directoryStamps;
    const std::weak_ptr<Sequence> weakSequence = sequence;
    auto *watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QObject::destroyed, [weakSequence]() {
        if (const auto current = weakSequence.lock())
            current->changeCheckInProgress = false;
    });
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, sequence]() {
        const bool changed = watcher->result();
        sequence->changeCheckInProgress = false;
        watcher->deleteLater();
        if (changed && m_sequence == sequence && !sequence->refreshInProgress)
            refreshFolderIndex();
    });
    watcher->setFuture(QtConcurrent::run([stamps]() {
        for (auto it = stamps.cbegin(); it != stamps.cend(); ++it) {
            const QFileInfo directory(it.key());
            if (!directory.isDir())
                return true;
            const QDateTime currentStamp = directory.lastModified();
            if (!currentStamp.isValid() || currentStamp != it.value())
                return true;
        }
        return false;
    }));
}

void AudioItemFolderMaxi::refreshFolderIndex()
{
    const auto sequence = m_sequence;
    if (!sequence || sequence->refreshInProgress)
        return;

    sequence->refreshInProgress = true;
    const QString folder = folderPath();
    const QStringList filters = MediaManager::supportedAudioNameFilters();
    const auto cancel = std::make_shared<std::atomic_bool>(false);
    const std::weak_ptr<Sequence> weakSequence = sequence;
    auto *watcher = new QFutureWatcher<FolderScan>(this);
    connect(watcher, &QObject::destroyed, [weakSequence, cancel]() {
        cancel->store(true);
        if (const auto current = weakSequence.lock())
            current->refreshInProgress = false;
    });
    connect(watcher, &QFutureWatcher<FolderScan>::finished, this,
            [this, watcher, sequence, folder]() {
        const FolderScan scan = watcher->result();
        sequence->refreshInProgress = false;
        watcher->deleteLater();
        // If a network share is temporarily unavailable, keep the playable
        // index and try again on a later timestamp check.
        if (m_sequence != sequence || !scan.rootReadable)
            return;

        updateSequence(scan.tracks);
        sequence->directoryStamps = scan.directoryStamps;
        sequence->scannedAtMs = QDateTime::currentMSecsSinceEpoch();
        m_tracks = sequence->tracks.values().toVector();
        m_displayedSequenceRevision = sequence->revision;
        setFolderPresentation(tr("%1 files").arg(m_tracks.size()), !m_tracks.isEmpty());
        setToolTip(tr("%1\n%2 audio files — random playback without repeats")
                       .arg(folder).arg(m_tracks.size()));
        emit scanFinished();
    });
    watcher->setFuture(QtConcurrent::run([folder, filters, cancel]() {
        return enumerateAudioTracks(folder, filters, cancel);
    }));
}

bool AudioItemFolderMaxi::preparePlayback()
{
    if (m_trackPrepared) return !m_announcementPreparing;
    if (m_scanning) {
        m_waitingForScan = true;
        return false;
    }
    if (m_orderedList) {
        if (m_nextOrderedTrack >= m_tracks.size()) {
            if (!isLoop()) return false;
            m_nextOrderedTrack = 0;
        }
        for (int checked = 0; checked < m_tracks.size(); ++checked) {
            if (m_nextOrderedTrack >= m_tracks.size()) {
                if (!isLoop()) break;
                m_nextOrderedTrack = 0;
            }
            const Track track = m_tracks.at(m_nextOrderedTrack++);
            if (!QFileInfo(track.path).isFile()) continue;
            m_currentTrack = track.path;
            setSecond(track.seconds);
            setTiempoFile(track.seconds);
            setSecondStart(0);
            m_trackPrepared = true;
            return prepareAnnouncementAudio();
        }
        return false;
    }
    if (!m_sequence || m_sequence->tracks.isEmpty()) return false;
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
    if (!m_orderedList) {
        setSecond(manager->getDuration());
        setTiempoFile(second());
    }
    return true;
}

QString AudioItemFolderMaxi::playbackName() const
{
    return m_currentTrack.isEmpty() ? nameFile() :
        nameFile() + " — " + QFileInfo(m_currentTrack).completeBaseName();
}

void AudioItemFolderMaxi::copyFolderStateFrom(const AudioItemFolderMaxi &source)
{
    if (source.m_orderedList) {
        if (m_cancel) m_cancel->store(true);
        m_cancel.reset();
        m_sequence.reset();
        m_scanning = false;
        m_waitingForScan = false;
        m_trackPrepared = false;
        m_orderedList = true;
        m_orderedListPath = source.m_orderedListPath;
        m_nextOrderedTrack = 0;
        m_tracks = source.m_tracks;
        m_currentTrack.clear();
        setFilePath(m_orderedListPath);
        setNameFile(source.nameFile());
        setSecond(0);
        setSecondStart(0);
        setToolTip(source.toolTip());
        setFolderPresentation(tr("%1 files").arg(m_tracks.size()), !m_tracks.isEmpty());
        return;
    }
    m_orderedList = false;
    m_orderedListPath.clear();
    m_nextOrderedTrack = 0;
    // A loaded folder already owns the complete index. Share it directly when
    // duplicating an item so network directories are not enumerated again.
    if (source.m_scanning) {
        setFolderPath(source.folderPath());
        return;
    }

    if (m_cancel) m_cancel->store(true);
    m_cancel.reset();
    m_sequence = source.m_sequence;
    m_tracks = m_sequence ? m_sequence->tracks.values().toVector() : source.m_tracks;
    m_displayedSequenceRevision = m_sequence ? m_sequence->revision : 0;
    m_currentTrack.clear();
    m_trackPrepared = false;
    m_scanning = false;
    m_waitingForScan = false;
    setFilePath(source.folderPath());
    setNameFile(source.nameFile());
    setSecond(0);
    setSecondStart(0);
    setToolTip(source.toolTip());
    setFolderPresentation(tr("%1 files").arg(m_tracks.size()), !m_tracks.isEmpty());
}

AudioItemMaxi *AudioItemFolderMaxi::copy(QWidget *newParent) const
{
    auto *item = new AudioItemFolderMaxi(newParent);
    item->setMixSettings(m_mixEnabled, m_mixSeconds);
    item->setSongsPerTurn(m_songsPerTurn);
    item->setAnnouncementTexts(m_introAnnouncement, m_outroAnnouncement,
                               m_announcementDelaySeconds);
    item->setAnnouncementsEnabled(m_announcementsEnabled);
    item->setAnnouncementVolumePercent(m_announcementVolumePercent);
    item->setAnnouncementMusicVolumePercent(m_announcementMusicVolumePercent);
    item->setAnnouncementRecoverySeconds(m_announcementRecoverySeconds);
    item->setSweeper(m_sweeperEnabled, m_sweeperFolderPath,
                     m_sweeperAtEntry, m_sweeperAtExit);
    item->setIsPurge(isPurge()); item->setIsPlayNext(isPlayNext());
    item->setIsLoop(isLoop()); item->setIsSelect(isSelect()); item->setColor(color());
    item->copyFolderStateFrom(*this);
    return item;
}
