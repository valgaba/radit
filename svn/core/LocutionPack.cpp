/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "core/LocutionPack.h"
#include <QProcess>
#include <QProcessEnvironment>
#include <QTimer>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QRegularExpression>
#include <QSet>
#include <QHash>
#include <QDateTime>
#ifdef Q_OS_LINUX
#include <minizip/unzip.h>
#endif

#ifdef Q_OS_LINUX
namespace {
QString extractLinuxVoicePack(const QString &path, const QString &destination)
{
    static const QRegularExpression accepted(
        QStringLiteral("(?:^|/)(?:time/(?:HRS[0-9]{2}(?:_O)?|MIN[0-9]{2})"
                       "|temperature/TMPN?[0-9]{3}|humidity/HUM[0-9]{3})\\.mp3$"),
        QRegularExpression::CaseInsensitiveOption);

    const QByteArray encodedPath = QFile::encodeName(path);
    unzFile archive = unzOpen64(encodedPath.constData());
    if (!archive)
        return QObject::tr("Invalid or unsupported ZIP voice pack.");

    unz_global_info64 globalInfo{};
    if (unzGetGlobalInfo64(archive, &globalInfo) != UNZ_OK || globalInfo.number_entry > 10000) {
        unzClose(archive);
        return QObject::tr("Invalid or unsupported ZIP voice pack.");
    }

    QSet<QString> seen;
    qint64 totalBytes = 0;
    int extracted = 0;
    int result = globalInfo.number_entry ? unzGoToFirstFile(archive) : UNZ_END_OF_LIST_OF_FILE;
    while (result == UNZ_OK) {
        unz_file_info64 entryInfo{};
        if (unzGetCurrentFileInfo64(archive, &entryInfo, nullptr, 0, nullptr, 0, nullptr, 0) != UNZ_OK
            || entryInfo.size_filename > 4096) {
            result = UNZ_BADZIPFILE;
            break;
        }

        QByteArray filename(int(entryInfo.size_filename) + 1, '\0');
        if (unzGetCurrentFileInfo64(archive, &entryInfo, filename.data(), unsigned(filename.size()),
                                    nullptr, 0, nullptr, 0) != UNZ_OK) {
            result = UNZ_BADZIPFILE;
            break;
        }

        const QString name = QString::fromUtf8(filename.constData(), int(entryInfo.size_filename));
        if (accepted.match(name).hasMatch()) {
            const QString baseName = QFileInfo(name).fileName().toLower();
            if (!entryInfo.uncompressed_size || entryInfo.uncompressed_size > 2 * 1024 * 1024
                || totalBytes + qint64(entryInfo.uncompressed_size) > 64 * 1024 * 1024
                || extracted >= 400 || seen.contains(baseName)) {
                result = UNZ_BADZIPFILE;
                break;
            }
            if (unzOpenCurrentFile(archive) != UNZ_OK) {
                result = UNZ_BADZIPFILE;
                break;
            }

            QByteArray data;
            data.reserve(int(entryInfo.uncompressed_size));
            char buffer[8192];
            int bytesRead = 0;
            do {
                bytesRead = unzReadCurrentFile(archive, buffer, sizeof(buffer));
                if (bytesRead > 0) {
                    data.append(buffer, bytesRead);
                    if (data.size() > int(entryInfo.uncompressed_size))
                        break;
                }
            } while (bytesRead > 0);

            const int closeResult = unzCloseCurrentFile(archive);
            if (bytesRead < 0 || data.size() != int(entryInfo.uncompressed_size)
                || closeResult != UNZ_OK) {
                result = UNZ_BADZIPFILE;
                break;
            }

            QSaveFile output(QDir(destination).filePath(baseName));
            if (!output.open(QIODevice::WriteOnly) || output.write(data) != data.size()
                || !output.commit()) {
                unzClose(archive);
                return QObject::tr("Unable to extract the ZIP voice pack.");
            }
            seen.insert(baseName);
            totalBytes += qint64(entryInfo.uncompressed_size);
            ++extracted;
        }
        result = unzGoToNextFile(archive);
    }

    const int closeResult = unzClose(archive);
    if ((result != UNZ_END_OF_LIST_OF_FILE && result != UNZ_OK) || closeResult != UNZ_OK)
        return QObject::tr("Invalid or unsupported ZIP voice pack.");
    return {};
}
}
#endif

std::shared_ptr<LocutionPack> LocutionPack::open(const QString &path)
{
    static QHash<QString, std::weak_ptr<LocutionPack>> packs;
    for (auto it=packs.begin();it!=packs.end();) {
        if (it.value().expired()) it=packs.erase(it); else ++it;
    }
    QFileInfo info(path);
    QString key=info.canonicalFilePath();
    if (key.isEmpty()) key=info.absoluteFilePath();
#ifdef Q_OS_WIN
    key=key.toCaseFolded();
#endif
    key+=QString("|%1|%2").arg(info.size()).arg(info.lastModified().toMSecsSinceEpoch());
    auto pack=packs.value(key).lock();
    if (!pack) {pack=std::shared_ptr<LocutionPack>(new LocutionPack(info.absoluteFilePath()));packs.insert(key,pack);}
    return pack;
}
LocutionPack::LocutionPack(const QString &path)
    : m_directory(QDir::tempPath()+"/radit-locution-XXXXXX"), m_process(new QProcess(this)), m_timeout(new QTimer(this))
{
    m_timeout->setSingleShot(true);m_timeout->setInterval(30000);
    connect(m_timeout,&QTimer::timeout,this,[this]() {m_process->kill();complete(tr("Voice pack loading timed out."));});
    connect(m_process,&QProcess::errorOccurred,this,[this](QProcess::ProcessError error) {
        if (error==QProcess::FailedToStart) complete(tr("Unable to open the ZIP voice pack."));
    });
    connect(m_process,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this](int code,QProcess::ExitStatus status) {
        if (!m_loading) return;
        if (code!=0 || status!=QProcess::NormalExit) {complete(tr("Invalid or unsupported ZIP voice pack."));return;}
        for (const char *name : {"hrs00.mp3","hrs00_o.mp3","min01.mp3","tmp000.mp3","hum000.mp3"}) {
            if (clip(name).isEmpty()) {complete(tr("The ZIP does not contain a MeteoClock voice pack."));return;}
        }
        m_ready=true;complete({});
    });
    QTimer::singleShot(0,this,[this,path]() {
        if (!m_directory.isValid() || !QFileInfo(path).isFile()) {complete(tr("Voice pack file is unavailable."));return;}
#ifdef Q_OS_WIN
        // Only recognized audio entries are extracted to generated, flat filenames.
        // Paths travel through process environment variables, never script interpolation.
        const QString script=QStringLiteral(R"PS(
$ErrorActionPreference='Stop'
$pack=$null
try {
 Add-Type -AssemblyName System.IO.Compression.FileSystem
 $pack=[IO.Compression.ZipFile]::OpenRead($env:RADIT_LOCUTION_ZIP)
 $seen=@{}; $total=0
 foreach ($entry in $pack.Entries) {
  $name=$entry.FullName.Replace('\','/')
  if ($name -notmatch '(?i)(?:^|/)(?:time/(?:HRS[0-9]{2}(?:_O)?|MIN[0-9]{2})|temperature/TMPN?[0-9]{3}|humidity/HUM[0-9]{3})\.mp3$') { continue }
  $base=[IO.Path]::GetFileName($name).ToLowerInvariant()
  $total+=$entry.Length
  if ($seen.ContainsKey($base) -or $entry.Length -le 0 -or $entry.Length -gt 2097152 -or $total -gt 67108864 -or $seen.Count -ge 400) { throw 'Invalid voice pack' }
  $seen[$base]=$true
  $dest=[IO.Path]::Combine($env:RADIT_LOCUTION_DEST,$base)
  [IO.Compression.ZipFileExtensions]::ExtractToFile($entry,$dest,$false)
 }
 $pack.Dispose(); exit 0
} catch { if($pack){$pack.Dispose()}; exit 1 }
)PS");
        auto environment=QProcessEnvironment::systemEnvironment();
        environment.insert("RADIT_LOCUTION_ZIP",path);environment.insert("RADIT_LOCUTION_DEST",m_directory.path());
        m_process->setProcessEnvironment(environment);
        m_process->setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *args){args->flags|=0x08000000;});
        const QByteArray encoded=QByteArray(reinterpret_cast<const char*>(script.utf16()),script.size()*2).toBase64();
        m_timeout->start();
        m_process->start(QDir(qEnvironmentVariable("WINDIR","C:/Windows")).filePath("System32/WindowsPowerShell/v1.0/powershell.exe"),
            {"-NoLogo","-NoProfile","-NonInteractive","-WindowStyle","Hidden","-EncodedCommand",QString::fromLatin1(encoded)});
#elif defined(Q_OS_LINUX)
        const QString extractionError = extractLinuxVoicePack(path, m_directory.path());
        if (!extractionError.isEmpty()) {
            complete(extractionError);
            return;
        }
        for (const char *name : {"hrs00.mp3", "hrs00_o.mp3", "min01.mp3", "tmp000.mp3", "hum000.mp3"}) {
            if (clip(name).isEmpty()) {
                complete(tr("The ZIP does not contain a MeteoClock voice pack."));
                return;
            }
        }
        m_ready = true;
        complete({});
#else
        complete(tr("ZIP voice packs are not supported on this platform."));
#endif
    });
}
LocutionPack::~LocutionPack()
{
    disconnect(m_process,nullptr,this,nullptr);
    m_timeout->stop();
    if (m_process->state()!=QProcess::NotRunning) {m_process->kill();m_process->waitForFinished(1000);}
}
void LocutionPack::complete(const QString &error)
{
    if (!m_loading) return;
    m_loading=false;m_error=error;m_timeout->stop();emit finished();
}
QString LocutionPack::clip(const QString &name) const
{
    const QString path=QDir(m_directory.path()).filePath(name.toLower());
    return QFileInfo(path).isFile() ? path : QString();
}
