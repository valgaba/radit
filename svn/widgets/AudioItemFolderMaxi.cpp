/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/AudioItemFolderMaxi.h"
#include "widgets/Player.h"
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
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
        const QString path = QFileDialog::getExistingDirectory(this, tr("Select audio folder"), folderPath());
        if (path.isEmpty()) return;
        if (isPlaying()) {
            QWidget *owner = parentWidget();
            while (owner && !qobject_cast<Player*>(owner)) owner = owner->parentWidget();
            if (auto *player = qobject_cast<Player*>(owner)) player->stopMain();
        }
        setFolderPath(path);
    });
}

AudioItemFolderMaxi::~AudioItemFolderMaxi() { if (m_cancel) m_cancel->store(true); }

void AudioItemFolderMaxi::setFolderPath(const QString &path)
{
    if (m_cancel) m_cancel->store(true);
    m_cancel = std::make_shared<std::atomic_bool>(false);
    const auto cancel = m_cancel;
    const QString folder = QDir::cleanPath(QFileInfo(path).absoluteFilePath());
    setFilePath(folder);
    const QString name = QDir(folder).dirName();
    setNameFile(name.isEmpty() ? QDir::toNativeSeparators(folder) : name);
    setToolTip(folder);
    m_tracks.clear(); m_remaining.clear(); m_currentTrack.clear(); m_lastTrack.clear();
    m_scanning = true;
    setSecond(0); setSecondStart(0);
    setFolderPresentation(tr("Loading..."), false);
    const QStringList filters = MediaManager::supportedAudioNameFilters();
    auto *watcher = new QFutureWatcher<QVector<Track>>(this);
    connect(watcher, &QFutureWatcher<QVector<Track>>::finished, this, [this, watcher, cancel, folder]() {
        const auto tracks = watcher->result();
        watcher->deleteLater();
        if (cancel != m_cancel || cancel->load()) return;
        m_tracks = tracks; m_scanning = false;
        setFolderPresentation(tr("%1 files").arg(m_tracks.size()), !m_tracks.isEmpty());
        setToolTip(tr("%1\n%2 audio files — random playback without repeats").arg(folder).arg(m_tracks.size()));
        emit scanFinished();
    });
    watcher->setFuture(QtConcurrent::run([folder, filters, cancel]() {
        QVector<Track> tracks;
        QDirIterator files(folder, filters, QDir::Files | QDir::Readable | QDir::NoSymLinks,
                           QDirIterator::Subdirectories);
        while (!cancel->load() && files.hasNext()) {
            const QString path = files.next();
            const double seconds = MediaManager::readFileDuration(path);
            if (seconds > 0) tracks.append({path, seconds});
        }
        return tracks;
    }));
}

bool AudioItemFolderMaxi::preparePlayback()
{
    if (m_scanning || m_tracks.isEmpty()) return false;
    if (m_remaining.isEmpty())
        for (int i = 0; i < m_tracks.size(); ++i) m_remaining.append(i);
    while (!m_remaining.isEmpty()) {
        int index = QRandomGenerator::global()->bounded(int(m_remaining.size()));
        // Also avoid an immediate repeat at the boundary between two cycles.
        if (m_remaining.size() > 1 && m_tracks[m_remaining[index]].path == m_lastTrack)
            index = (index + 1 + QRandomGenerator::global()->bounded(int(m_remaining.size()) - 1)) % m_remaining.size();
        const Track track = m_tracks[m_remaining.takeAt(index)];
        if (!QFileInfo(track.path).isFile()) continue;
        m_currentTrack = track.path; m_lastTrack = track.path;
        setSecond(track.seconds); setTiempoFile(track.seconds); setSecondStart(0);
        return true;
    }
    setFolderPresentation(tr("No files"), false);
    return false;
}

QString AudioItemFolderMaxi::playbackName() const
{
    return m_currentTrack.isEmpty() ? nameFile() :
        nameFile() + " — " + QFileInfo(m_currentTrack).completeBaseName();
}

AudioItemMaxi *AudioItemFolderMaxi::copy(QWidget *newParent) const
{
    auto *item = new AudioItemFolderMaxi(newParent);
    item->setIsPurge(isPurge()); item->setIsPlayNext(isPlayNext());
    item->setIsLoop(isLoop()); item->setIsSelect(isSelect()); item->setColor(color());
    if (m_scanning) item->setFolderPath(folderPath());
    else {
        item->setFilePath(folderPath()); item->setNameFile(nameFile());
        item->m_tracks = m_tracks; item->m_remaining = m_remaining; item->m_lastTrack = m_lastTrack;
        item->setToolTip(toolTip());
        item->setFolderPresentation(tr("%1 files").arg(m_tracks.size()), !m_tracks.isEmpty());
    }
    return item;
}
