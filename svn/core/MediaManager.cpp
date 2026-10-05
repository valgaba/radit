/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org

   Radit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   radit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with radit.  If not, see <http://www.gnu.org/licenses/>.
*/



#include <QDebug>
#include <QCoreApplication>
#include <QHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QProcess>
#include <algorithm>
#include <cmath>
#include "core/MediaManager.h"
#include <bass.h>

// Los tipos y callbacks del motor quedan fuera de la interfaz pública.
struct MediaManager::Backend
{
    HSTREAM stream = 0;
    HRECORD inputStream = 0;
    int inputDevice = -1;
    DWORD inputRate = 0;
    DWORD inputChannels = 0;
    QMutex inputMutex;
    bool collectRecording = false;
    bool recording = false;
    bool overflow = false;
    float inputGain = 1.0f;
    QByteArray recordingData;
    qint64 recordingBytes = 0;
    QProcess *encoder = nullptr;
    QString recordingPath;
    static BOOL CALLBACK InputCallback(HRECORD, const void *buffer, DWORD length, void *user)
    {
        auto *backend = static_cast<Backend *>(user);
        const QMutexLocker lock(&backend->inputMutex);
        if (!backend->collectRecording)
            return TRUE;
        // El callback de BASS no accede a QProcess ni a los widgets de Qt.
        if (backend->recordingData.size() + length > 4 * 1024 * 1024) {
            backend->overflow = true;
            return TRUE;
        }
        const qsizetype offset = backend->recordingData.size();
        backend->recordingData.append(static_cast<const char *>(buffer), length);
        auto *samples = reinterpret_cast<float *>(backend->recordingData.data() + offset);
        for (DWORD i = 0; i < length / sizeof(float); ++i)
            samples[i] *= backend->inputGain;
        backend->recordingBytes += length;
        return TRUE;
    }
    static void CALLBACK EndSyncCallback(HSYNC handle, DWORD channel, DWORD data, void *user);
    static void CALLBACK FadeOutSyncCallback(HSYNC handle, DWORD channel, DWORD data, void *user);
    static void CALLBACK DeviceFailedSyncProc(HSYNC handle, DWORD channel, DWORD data, void *user);
};

namespace {
struct InputDeviceUse
{
    int users = 0;
    bool owned = false;
};

QHash<int, InputDeviceUse> &inputDeviceUses()
{
    // Acceso desde el hilo de Qt; los callbacks no modifican esta tabla.
    static QHash<int, InputDeviceUse> devices;
    return devices;
}

void releaseInputDevice(int device)
{
    auto &devices = inputDeviceUses();
    auto it = devices.find(device);
    if (it == devices.end() || --it->users > 0)
        return;
    const bool owned = it->owned;
    devices.erase(it);
    const DWORD previous = BASS_RecordGetDevice();
    if (owned && BASS_RecordSetDevice(device))
        BASS_RecordFree();
    if (previous != static_cast<DWORD>(-1) && previous != static_cast<DWORD>(device))
        BASS_RecordSetDevice(previous);
}

QList<HPLUGIN> &loadedAudioPlugins()
{
    // Los plugins de BASS son globales, compartidos por todos los players.
    static QList<HPLUGIN> plugins;
    return plugins;
}

void registerAudioPlugin(HPLUGIN handle)
{
    if (handle && !loadedAudioPlugins().contains(handle))
        loadedAudioPlugins().append(handle);
}

QString audioDeviceName(const char *name)
{
#ifdef Q_OS_WIN
    if (!BASS_GetConfig(BASS_CONFIG_UNICODE))
        return QString::fromLocal8Bit(name);
#endif
    return QString::fromUtf8(name);
}
}

QList<AudioDevice> MediaManager::inputDevices()
{
    QList<AudioDevice> devices;
    BASS_DEVICEINFO info = {};
    for (DWORD id = 0; BASS_RecordGetDeviceInfo(id, &info); ++id) {
        if ((info.flags & BASS_DEVICE_ENABLED) && !(info.flags & BASS_DEVICE_LOOPBACK)) {
            const DWORD type = info.flags & BASS_DEVICE_TYPE_MASK;
            const bool microphone = !(info.flags & BASS_DEVICE_LOOPBACK) &&
                (type == BASS_DEVICE_TYPE_MICROPHONE ||
                 type == BASS_DEVICE_TYPE_HEADSET || type == BASS_DEVICE_TYPE_HANDSET);
            devices.append({static_cast<int>(id), audioDeviceName(info.name),
                            (info.flags & BASS_DEVICE_DEFAULT) != 0, microphone, info.driver ? QString::fromUtf8(info.driver) : QString()});
        }
    }
    return devices;
}

QList<AudioDevice> MediaManager::outputDevices()
{
    QList<AudioDevice> devices;
    BASS_DEVICEINFO info = {};
    for (DWORD id = 0; BASS_GetDeviceInfo(id, &info); ++id) {
        if (info.flags & BASS_DEVICE_ENABLED)
            devices.append({static_cast<int>(id), audioDeviceName(info.name),
                            (info.flags & BASS_DEVICE_DEFAULT) != 0, false, info.driver ? QString::fromUtf8(info.driver) : QString()});
    }
    return devices;
}

AudioWaveform MediaManager::readWaveform(const QString &filePath,
                                         const std::shared_ptr<std::atomic_bool> &cancel,
                                         const std::shared_ptr<std::atomic_int> &progress)
{
    if (progress) progress->store(-1);
    AudioWaveform waveform;
    if (cancel->load())
        return waveform;
    // Contexto de decodificación independiente, sin abrir una salida audible.
    static QMutex decoderInitMutex;
    {
        const QMutexLocker lock(&decoderInitMutex);
        if (!BASS_SetDevice(0) && !BASS_Init(0, 44100, 0, nullptr, nullptr)) {
            waveform.error = tr("Unable to initialize waveform decoding (error %1).")
                .arg(BASS_ErrorGetCode());
            return waveform;
        }
    }
#ifdef Q_OS_WIN
    const HSTREAM stream = BASS_StreamCreateFile(FALSE, filePath.utf16(), 0, 0,
        BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT | BASS_STREAM_PRESCAN | BASS_UNICODE);
#else
    const QByteArray path = filePath.toUtf8();
    const HSTREAM stream = BASS_StreamCreateFile(FALSE, path.constData(), 0, 0,
        BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT | BASS_STREAM_PRESCAN);
#endif
    if (!stream) {
        waveform.error = tr("Unable to read the waveform (error %1).")
            .arg(BASS_ErrorGetCode());
        return waveform;
    }
    struct StreamGuard {
        HSTREAM stream;
        ~StreamGuard() { BASS_StreamFree(stream); }
    } guard{stream};
    BASS_CHANNELINFO info = {};
    if (!BASS_ChannelGetInfo(stream, &info) || !info.freq || !info.chans || info.chans > 32) {
        waveform.error = tr("Unsupported waveform audio format.");
        return waveform;
    }
    const QWORD length = BASS_ChannelGetLength(stream, BASS_POS_BYTE);
    const double duration = length == QWORD(-1) ? 0 : BASS_ChannelBytes2Seconds(stream, length);
    // 100 puntos por segundo; limitar la memoria para grabaciones largas.
    const qint64 framesPerPeak = std::max(std::max(qint64(1), qint64(info.freq / 100)),
        qint64(std::ceil(duration * info.freq / 1000000.0)));
    waveform.secondsPerPeak = double(framesPerPeak) / info.freq;
    QVector<float> samples(8192 * info.chans);
    WaveformPeak peak;
    qint64 frames = 0, binFrames = 0;
    QWORD decodedBytes = 0;
    const bool knownLength = length != QWORD(-1) && length > 0;
    int lastProgress = -1;
    if (progress && knownLength) progress->store(0);
    while (!cancel->load()) {
        const DWORD bytes = BASS_ChannelGetData(stream, samples.data(), samples.size() * sizeof(float));
        if (bytes == DWORD(-1)) {
            if (BASS_ErrorGetCode() != BASS_ERROR_ENDED)
                waveform.error = tr("Waveform decoding failed (error %1).")
                    .arg(BASS_ErrorGetCode());
            break;
        }
        if (!bytes)
            break;
        const DWORD count = bytes / (sizeof(float) * info.chans);
        for (DWORD frame = 0; frame < count; ++frame) {
            for (DWORD channel = 0; channel < info.chans; ++channel) {
                const float value = samples[frame * info.chans + channel];
                if (std::isfinite(value)) {
                    peak.minimum = std::min(peak.minimum, std::clamp(value, -1.0f, 1.0f));
                    peak.maximum = std::max(peak.maximum, std::clamp(value, -1.0f, 1.0f));
                }
            }
            ++frames;
            if (++binFrames == framesPerPeak) {
                waveform.peaks.append(peak);
                peak = {};
                binFrames = 0;
            }
        }
        decodedBytes += bytes;
        if (progress && knownLength) {
            const int percent = int(std::min(99.0, double(decodedBytes) / double(length) * 100.0));
            if (percent != lastProgress) {
                progress->store(percent);
                lastProgress = percent;
            }
        }
        if (waveform.peaks.size() > 1000000) {
            waveform.error = tr("The audio file is too long to display its waveform.");
            break;
        }
    }
    if (cancel->load())
        return {};
    if (binFrames)
        waveform.peaks.append(peak);
    waveform.duration = double(frames) / info.freq;
    if (progress && waveform.error.isEmpty()) progress->store(100);
    return waveform;
}

QStringList MediaManager::supportedAudioNameFilters()
{
    // Formatos nativos de BASS_StreamCreateFile. No incluir módulos musicales:
    // necesitan BASS_MusicLoad, que el reproductor de Radit no utiliza.
    QStringList filters = {
        "*.mp3", "*.mp2", "*.mp1", "*.ogg", "*.wav", "*.aif", "*.aiff"
    };

    for (HPLUGIN handle : loadedAudioPlugins()) {
        const BASS_PLUGININFO *info = BASS_PluginGetInfo(handle);
        if (!info)
            continue;

        for (DWORD i = 0; i < info->formatc; ++i) {
            if (!info->formats[i].exts)
                continue;
            const QString extensions = QString::fromUtf8(info->formats[i].exts);
            for (const QString &extension : extensions.split(';', Qt::SkipEmptyParts)) {
                const QString filter = extension.trimmed().toLower();
                if (filter.startsWith("*.") && filter.size() > 2)
                    filters.append(filter);
            }
        }
    }
    filters.removeDuplicates();
    return filters;
}



MediaManager::MediaManager(QObject *parent)
    : QObject(parent), m_backend(std::make_unique<Backend>())
{
    qRegisterMetaType<AudioFrame>("AudioFrame");

    m_backend->encoder = new QProcess(this);
#ifdef Q_OS_WIN
    m_backend->encoder->setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *args) {
        args->flags |= CREATE_NO_WINDOW;
    });
#endif
    connect(m_backend->encoder, &QProcess::readyReadStandardError, this, [this]() {
        // Evitar que los mensajes del codificador crezcan sin límite.
        const QByteArray error = m_backend->encoder->readAllStandardError();
        if (!error.isEmpty())
            qWarning().noquote() << "Capture:" << QString::fromUtf8(error).trimmed();
    });
    connect(m_backend->encoder, &QProcess::finished, this, [this]() {
        if (isRecording())
            stopRecording();
    });

    m_inputTimer = new QTimer(this);
    m_inputTimer->setTimerType(Qt::PreciseTimer);
    m_inputTimer->setInterval(25);
    connect(m_inputTimer, &QTimer::timeout, this, [this]() {
        if (!m_backend->inputStream)
            return;
        const DWORD level = BASS_ChannelGetLevel(m_backend->inputStream);
        if (level == static_cast<DWORD>(-1) ||
            BASS_ChannelIsActive(m_backend->inputStream) != BASS_ACTIVE_PLAYING) {
            stopInput();
            emit inputError(tr("Audio input disconnected or unavailable. Select the input device again."));
            return;
        }
        const auto toDb = [this](float amplitude) {
            return 20.0f * std::log10(std::max(amplitude * m_inputVolume, 0.000001f));
        };
        emit inputLevelsChanged(toDb(LOWORD(level) / 32768.0f),
                                toDb(HIWORD(level) / 32768.0f));
        flushRecordingData();
    });

    m_deviceRecoveryTimer = new QTimer(this);
    m_deviceRecoveryTimer->setInterval(500);
    connect(m_deviceRecoveryTimer, &QTimer::timeout,
            this, &MediaManager::recoverDevice);

       m_timer = new QTimer(this);

       connect(m_timer, &QTimer::timeout, this, [this]() {

           if (!m_backend->stream) return;

           DWORD state = BASS_ChannelIsActive(m_backend->stream);

           if (state == BASS_ACTIVE_PLAYING)
           {
               AudioFrame frame;

               frame.position = BASS_ChannelBytes2Seconds(
                   m_backend->stream,
                   BASS_ChannelGetPosition(m_backend->stream, BASS_POS_BYTE)
               );

               DWORD level = BASS_ChannelGetLevel(m_backend->stream);
               // Una lectura fallida no es un pico de audio.
               if (level == static_cast<DWORD>(-1))
                   level = 0;

               // El nivel de BASS se mide antes del volumen: aplicarlo al vúmetro.
               float volume = 1.0f;
               if (!BASS_ChannelGetAttribute(m_backend->stream, BASS_ATTRIB_VOL, &volume))
                   volume = 1.0f;
               float leftLinear  = (LOWORD(level) / 32768.0f) * volume;
               float rightLinear = (HIWORD(level) / 32768.0f) * volume;

               leftLinear  = std::max(leftLinear,  0.000001f);
               rightLinear = std::max(rightLinear, 0.000001f);

               frame.left  = 20.0f * log10f(leftLinear);
               frame.right = 20.0f * log10f(rightLinear);

               //  detección de finales
                    /*  if (shouldStopBySilence(frame)) //cuidado corta estrevistas etc.
                      {
                          BASS_ChannelStop(m_backend->stream);
                          m_timer->stop();

                          emit playbackFinished();
                          return;
                      }*/



               emit audioFrameUpdated(frame);
           }

       });

       //m_timer->start(50); // 20 FPS
}


MediaManager::~MediaManager(){

    stopInput();

    if (m_backend->stream){
        BASS_StreamFree(m_backend->stream);
        m_backend->stream = 0;
       }


}

bool MediaManager::startInput(int deviceId)
{
    if (deviceId >= 0 && m_backend->inputDevice == deviceId &&
        m_backend->inputStream &&
        BASS_ChannelIsActive(m_backend->inputStream) == BASS_ACTIVE_PLAYING)
        return true;

    stopInput();
    if (deviceId < 0)
        return false;

    const DWORD previous = BASS_RecordGetDevice();
    auto &devices = inputDeviceUses();
    bool ready = false;
    if (devices.contains(deviceId)) {
        ready = BASS_RecordSetDevice(deviceId);
        if (ready)
            ++devices[deviceId].users;
    } else {
        const bool owned = BASS_RecordInit(deviceId);
        ready = owned || (BASS_ErrorGetCode() == BASS_ERROR_ALREADY &&
                          BASS_RecordSetDevice(deviceId));
        if (ready)
            devices.insert(deviceId, {1, owned});
    }
    int error = ready ? 0 : BASS_ErrorGetCode();
    if (ready) {
        // Formato nativo: también admite micrófonos mono. Callback cada 20 ms.
        m_backend->inputStream = BASS_RecordStart(0, 0, MAKELONG(BASS_SAMPLE_FLOAT, 20),
                                                Backend::InputCallback, m_backend.get());
        if (!m_backend->inputStream) {
            error = BASS_ErrorGetCode();
            releaseInputDevice(deviceId);
        }
    }
    if (previous != static_cast<DWORD>(-1))
        BASS_RecordSetDevice(previous);
    if (!m_backend->inputStream) {
        emit inputError(tr("Unable to open the audio input (error %1).").arg(error));
        return false;
    }
    m_backend->inputDevice = deviceId;
    BASS_CHANNELINFO info = {};
    BASS_ChannelGetInfo(m_backend->inputStream, &info);
    m_backend->inputRate = info.freq;
    m_backend->inputChannels = info.chans;
    m_inputTimer->start();
    return true;
}

void MediaManager::stopInput()
{
    stopRecording();
    m_inputTimer->stop();
    if (m_backend->inputStream) {
        BASS_ChannelStop(m_backend->inputStream);
        m_backend->inputStream = 0;
    }
    if (m_backend->inputDevice >= 0) {
        releaseInputDevice(m_backend->inputDevice);
        m_backend->inputDevice = -1;
    }
    emit inputLevelsChanged(-120.0f, -120.0f);
}

void MediaManager::setInputVolume(float volume)
{
    if (std::isfinite(volume))
        m_inputVolume = std::clamp(volume, 0.0f, 1.0f);
    {
        const QMutexLocker lock(&m_backend->inputMutex);
        m_backend->inputGain = m_inputVolume;
    }
    if (m_inputVolume == 0.0f)
        emit inputLevelsChanged(-120.0f, -120.0f);
}

float MediaManager::inputVolume() const
{
    return m_inputVolume;
}

bool MediaManager::isRecording() const
{
    return m_backend->recording;
}

QList<Mp3RecordingMode> MediaManager::mp3RecordingModes()
{
    return {
        {"mp3-44100-stereo-128", tr("MP3 44.1 kHz / Stereo / 128 kbps"), 44100, 2, 128},
        {"mp3-44100-stereo-192", tr("MP3 44.1 kHz / Stereo / 192 kbps"), 44100, 2, 192},
        {"mp3-44100-stereo-320", tr("MP3 44.1 kHz / Stereo / 320 kbps"), 44100, 2, 320},
        {"mp3-48000-stereo-192", tr("MP3 48 kHz / Stereo / 192 kbps"), 48000, 2, 192},
        {"mp3-48000-stereo-320", tr("MP3 48 kHz / Stereo / 320 kbps"), 48000, 2, 320},
        {"mp3-44100-mono-96", tr("MP3 44.1 kHz / Mono / 96 kbps"), 44100, 1, 96}
    };
}

bool MediaManager::setRecordingMode(const QString &id)
{
    if (isRecording()) return false;
    for (const auto &mode : mp3RecordingModes()) {
        if (mode.id == id) {
            m_recordingMode = id;
            return true;
        }
    }
    return false;
}

QString MediaManager::recordingMode() const
{
    return m_recordingMode;
}

QStringList MediaManager::mp3EncodingOptions() const
{
    for (const auto &mode : mp3RecordingModes()) {
        if (mode.id == m_recordingMode) {
            return {"-ar", QString::number(mode.sampleRate), "-ac", QString::number(mode.channels),
                    "-c:a", "libmp3lame", "-b:a", QString::number(mode.bitrateKbps) + "k"};
        }
    }
    return {};
}

bool MediaManager::startRecording()
{
    if (isRecording())
        return true;
    if (!m_backend->inputStream || !m_backend->inputRate || !m_backend->inputChannels ||
        BASS_ChannelIsActive(m_backend->inputStream) != BASS_ACTIVE_PLAYING) {
        emit recordingError(tr("Select an available audio input before recording."));
        return false;
    }

    const QDir applicationDir(QCoreApplication::applicationDirPath());
    const QString program = applicationDir.filePath("ffmpeg/ffmpeg.exe");
    if (!QFileInfo::exists(program)) {
        emit recordingError(tr("The MP3 encoder is missing: %1").arg(program));
        return false;
    }
    if (!applicationDir.mkpath("capture")) {
        emit recordingError(tr("Unable to create the capture folder."));
        return false;
    }
    const QString baseName = "capture_" +
        QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    QFile output;
    for (int suffix = 1; ; ++suffix) {
        const QString name = baseName + (suffix == 1 ? QString() : "_" + QString::number(suffix));
        output.setFileName(applicationDir.filePath("capture/" + name + ".mp3"));
        // Reservar el archivo de forma exclusiva, incluso con dos instancias de Radit.
        if (output.open(QIODevice::WriteOnly | QIODevice::NewOnly))
            break;
        if (!QFileInfo::exists(output.fileName())) {
            emit recordingError(tr("Unable to write to the capture folder: %1").arg(output.errorString()));
            return false;
        }
    }
    const QString path = output.fileName();
    output.close();
    QStringList arguments{
        "-hide_banner", "-loglevel", "error", "-nostdin", "-y",
        "-f", "f32le", "-ar", QString::number(m_backend->inputRate),
        "-ac", QString::number(m_backend->inputChannels), "-i", "pipe:0"
    };
    arguments << mp3EncodingOptions() << "-f" << "mp3" << path;
    m_backend->encoder->start(program, arguments);
    if (!m_backend->encoder->waitForStarted(3000)) {
        const QString error = m_backend->encoder->errorString();
        m_backend->encoder->kill();
        m_backend->encoder->waitForFinished(1000);
        output.remove();
        emit recordingError(tr("Unable to start the MP3 encoder: %1")
                            .arg(error));
        return false;
    }
    m_backend->recordingPath = path;
    {
        const QMutexLocker lock(&m_backend->inputMutex);
        m_backend->recordingData.clear();
        m_backend->recordingBytes = 0;
        m_backend->overflow = false;
        m_backend->collectRecording = true;
    }
    m_backend->recording = true;
    emit recordingTimeChanged(0);
    emit recordingChanged(true);
    return true;
}

void MediaManager::flushRecordingData()
{
    if (!isRecording())
        return;
    QByteArray data;
    qint64 bytes = 0;
    bool overflow = false;
    {
        const QMutexLocker lock(&m_backend->inputMutex);
        data.swap(m_backend->recordingData);
        bytes = m_backend->recordingBytes;
        overflow = m_backend->overflow;
    }
    if (overflow || m_backend->encoder->bytesToWrite() > 4 * 1024 * 1024 ||
        m_backend->encoder->state() != QProcess::Running ||
        (!data.isEmpty() && m_backend->encoder->write(data) != data.size())) {
        if (stopRecording())
            emit recordingError(tr("Recording stopped because the MP3 encoder could not keep up or failed."));
        return;
    }
    emit recordingTimeChanged(bytes * 1000 /
                              (m_backend->inputRate * m_backend->inputChannels * sizeof(float)));
}

bool MediaManager::stopRecording()
{
    if (!isRecording())
        return true;
    QByteArray data;
    qint64 bytes = 0;
    bool overflow = false;
    {
        const QMutexLocker lock(&m_backend->inputMutex);
        m_backend->collectRecording = false;
        data.swap(m_backend->recordingData);
        bytes = m_backend->recordingBytes;
        overflow = m_backend->overflow;
    }
    // Marcar antes de esperar: finished no debe cerrar dos veces el codificador.
    m_backend->recording = false;
    bool success = !overflow;
    if (m_backend->encoder->state() == QProcess::Running) {
        if (!data.isEmpty())
            success &= m_backend->encoder->write(data) == data.size();
        m_backend->encoder->closeWriteChannel();
        if (!m_backend->encoder->waitForFinished(10000)) {
            m_backend->encoder->kill();
            m_backend->encoder->waitForFinished(1000);
            success = false;
        }
    } else if (!data.isEmpty()) {
        success = false;
    }
    success &= m_backend->encoder->exitStatus() == QProcess::NormalExit &&
               m_backend->encoder->exitCode() == 0 && bytes > 0 &&
               QFileInfo(m_backend->recordingPath).size() > 0;
    emit recordingTimeChanged(bytes * 1000 /
                              (m_backend->inputRate * m_backend->inputChannels * sizeof(float)));
    emit recordingChanged(false);
    if (success)
        emit recordingFinished(m_backend->recordingPath);
    else
        emit recordingError(tr("The recording could not be completed. Check the file: %1")
                            .arg(m_backend->recordingPath));
    return success;
}


//*****************************************

bool MediaManager::initialize()
{
    int device = -1; // default Windows

#ifdef Q_OS_WIN
    // Configurar los nombres antes de inicializar o enumerar dispositivos.
    // Capture y FrameOptionsPlayer los convierten con QString::fromUtf8.
    if (!BASS_GetConfig(BASS_CONFIG_UNICODE) &&
        !BASS_SetConfig(BASS_CONFIG_UNICODE, TRUE)) {
        qWarning() << "BASS: no se pudo activar UTF-8 para los dispositivos, error"
                   << BASS_ErrorGetCode();
    }
#endif

    BASS_SetConfig(BASS_CONFIG_BUFFER, 5000);
    BASS_SetConfig(BASS_CONFIG_NET_PLAYLIST, 1);

    if (!BASS_Init(device, 44100, 0, nullptr, nullptr))
    {
        qDebug() << "BASS_Init error:" << BASS_ErrorGetCode();
        return false;
    }

#ifdef Q_OS_WIN

    QString basePath = QCoreApplication::applicationDirPath() + "/bassplugin";

    QStringList plugins = {
        "bass_aac.dll",
        "bassflac.dll",
        "basswma.dll"
    };

    for (const QString &plugin : plugins)
    {
        QString fullPath = basePath + "/" + plugin;

        HPLUGIN handle = BASS_PluginLoad(
            fullPath.toUtf8().constData(), 0
        );

        registerAudioPlugin(handle);

        if (!handle){

            qDebug() << "Error cargando plugin:"
                     << fullPath
                     << "Error code:"
                     << BASS_ErrorGetCode();

        }
    }

#endif

#ifdef Q_OS_UNIX
    QString basePath = QCoreApplication::applicationDirPath() + "/Plugin";
    registerAudioPlugin(BASS_PluginLoad((basePath + "/libbass_aac.so").toUtf8(), 0));
    registerAudioPlugin(BASS_PluginLoad((basePath + "/libbassflac.so").toUtf8(), 0));
#endif

    return true;
}


void MediaManager::shutdown(){

    stopInput();
    m_timer->stop();
    m_deviceRecoveryTimer->stop();

    if (m_backend->stream)
    {
        BASS_StreamFree(m_backend->stream);
        m_backend->stream = 0;
    }

    BASS_Free();
}



bool MediaManager::loadFile(const QString &filePath){

    // La selección de BASS es por hilo y puede haberla cambiado otro player.
    if (m_currentDevice >= 0 && !startDevice(m_currentDevice))
        return false;

    m_deviceRecoveryTimer->stop();

    if (m_backend->stream) {
          BASS_StreamFree(m_backend->stream);
          m_backend->stream = 0;
      }

  #ifdef Q_OS_WIN
      m_backend->stream = BASS_StreamCreateFile(
          FALSE,
          filePath.utf16(),
          0,
          0,
          BASS_UNICODE
      );
  #else
      QByteArray path = filePath.toUtf8();
      m_backend->stream = BASS_StreamCreateFile(
          FALSE,
          path.constData(),
          0,
          0,
          0
      );
  #endif

      if (!m_backend->stream) {
           return false;
         }

         // Volumen normal de reproducción, sin amplificación.
         if (!BASS_ChannelSetAttribute(m_backend->stream, BASS_ATTRIB_VOL, m_volume)) {
             BASS_StreamFree(m_backend->stream);
             m_backend->stream = 0;
             return false;
         }


         BASS_ChannelSetSync(
             m_backend->stream,
             BASS_SYNC_END,
             0,
             &MediaManager::Backend::EndSyncCallback,
             this
         );


         // Fallo/desconexión del dispositivo de salida
         BASS_ChannelSetSync(
             m_backend->stream,
             BASS_SYNC_DEV_FAIL,
             0,
             &MediaManager::Backend::DeviceFailedSyncProc,
             this
         );




         return true;
}

//******************************************************************


double MediaManager::getDurationSecond(const QString &filePath){

#ifdef Q_OS_WIN
    HSTREAM stream = BASS_StreamCreateFile(
        FALSE,
        filePath.utf16(),
        0,
        0,
        BASS_STREAM_DECODE | BASS_UNICODE
    );
#else
    QByteArray path = filePath.toUtf8();

    HSTREAM stream = BASS_StreamCreateFile(
        FALSE,
        path.constData(),
        0,
        0,
        BASS_STREAM_DECODE
    );
#endif

    if (!stream) {
        qDebug() << "BASS error:" << BASS_ErrorGetCode()
                 << "File:" << filePath;
        return -1.0;
    }

    QWORD length = BASS_ChannelGetLength(stream, BASS_POS_BYTE);
    double seconds = BASS_ChannelBytes2Seconds(stream, length);

    BASS_StreamFree(stream);
    return seconds;
}



//****************************************************


void MediaManager::play()
{
    if (!m_backend->stream) return;

    const DWORD device = BASS_ChannelGetDevice(m_backend->stream);
    if (device == static_cast<DWORD>(-1))
        return;

    if (!startDevice(static_cast<int>(device)))
        m_deviceRecoveryTimer->start();

    // BASS conserva el canal mientras la salida está desconectada.
    if (BASS_ChannelPlay(m_backend->stream, FALSE)) {
        m_timer->start(50);
        float volume = 0.0f;
        if (BASS_ChannelGetAttribute(m_backend->stream, BASS_ATTRIB_VOL, &volume)) {
            qDebug() << "BASS: canal" << m_backend->stream
                     << "dispositivo" << device
                     << "volumen real" << volume
                     << "esperado" << m_volume;
        } else {
            qWarning() << "BASS: no se pudo leer el volumen del canal" << m_backend->stream
                       << "error" << BASS_ErrorGetCode();
        }
    }
}

bool MediaManager::setVolume(float volume)
{
    if (!std::isfinite(volume))
        return false;
    const float value = std::clamp(volume, 0.0f, 1.0f);
    if (m_backend->stream && !BASS_ChannelSetAttribute(m_backend->stream, BASS_ATTRIB_VOL, value)) {
        qWarning() << "BASS: no se pudo ajustar el volumen, error" << BASS_ErrorGetCode();
        return false;
    }
    m_volume = value;
    return true;
}

float MediaManager::volume() const
{
    return m_volume;
}

void MediaManager::pause()
{
    if (!m_backend->stream) return;

        BASS_ChannelPause(m_backend->stream);
        m_timer->stop();
}

void MediaManager::stop()
{
    if (!m_backend->stream)
         return;


        BASS_ChannelStop(m_backend->stream);
        m_timer->stop();

        emit positionChanged(0.0);
}


bool MediaManager::isPlaying() const
{
    if (!m_backend->stream) return false;
    return BASS_ChannelIsActive(m_backend->stream) == BASS_ACTIVE_PLAYING;
}

bool MediaManager::isPaused() const
{
    if (!m_backend->stream) return false;
    return BASS_ChannelIsActive(m_backend->stream) == BASS_ACTIVE_PAUSED;
}

void MediaManager::rewind()
{
    seekRelative(-1.0); // esta a 1 segundo
}

void MediaManager::forward()
{
    seekRelative(1.0);
}

void MediaManager::seek(double seconds)
{
    if (!m_backend->stream) return;

       // Duración total
       double duration = BASS_ChannelBytes2Seconds(
           m_backend->stream,
           BASS_ChannelGetLength(m_backend->stream, BASS_POS_BYTE)
       );

       if (seconds < 0.0)
           seconds = 0.0;

       if (seconds > duration)
           seconds = duration;

       // Convertir y aplicar
       QWORD bytePos = BASS_ChannelSeconds2Bytes(m_backend->stream, seconds);
       BASS_ChannelSetPosition(m_backend->stream, bytePos, BASS_POS_BYTE);

       //  Actualizar UI inmediatamente
       DWORD level = BASS_ChannelGetLevel(m_backend->stream);
       // Mantener la actualización de posición aunque falle la lectura de nivel.
       if (level == static_cast<DWORD>(-1))
           level = 0;

       AudioFrame frame;

       frame.position = seconds;

       // Mantener la misma lectura al cambiar la posición de reproducción.
       float volume = 1.0f;
       if (!BASS_ChannelGetAttribute(m_backend->stream, BASS_ATTRIB_VOL, &volume))
           volume = 1.0f;
       float leftLinear  = (LOWORD(level) / 32768.0f) * volume;
       float rightLinear = (HIWORD(level) / 32768.0f) * volume;

       leftLinear  = std::max(leftLinear,  0.000001f);
       rightLinear = std::max(rightLinear, 0.000001f);

       frame.left  = 20.0f * log10f(leftLinear);
       frame.right = 20.0f * log10f(rightLinear);

       emit audioFrameUpdated(frame);
}




double MediaManager::getDuration() const
{
    if (!m_backend->stream)
        return 0;
    const QWORD length = BASS_ChannelGetLength(m_backend->stream, BASS_POS_BYTE);
    if (length == QWORD(-1))
        return 0;
    return std::max(0.0, BASS_ChannelBytes2Seconds(m_backend->stream, length));
}

double MediaManager::getPosition() const
{
    if (!m_backend->stream)
        return 0;
    const QWORD position = BASS_ChannelGetPosition(m_backend->stream, BASS_POS_BYTE);
    if (position == QWORD(-1))
        return 0;
    return std::max(0.0, BASS_ChannelBytes2Seconds(m_backend->stream, position));
}

void MediaManager::seekRelative(double deltaSeconds)
{
    if (!m_backend->stream) return;

      double current = BASS_ChannelBytes2Seconds(
          m_backend->stream,
          BASS_ChannelGetPosition(m_backend->stream, BASS_POS_BYTE)
      );

      seek(current + deltaSeconds);
}


//**************************
bool MediaManager::setDevice(int deviceId){

    if (deviceId == -1) {
        BASS_DEVICEINFO info = {};
        for (int i = 1; BASS_GetDeviceInfo(i, &info); ++i) {
            if ((info.flags & BASS_DEVICE_ENABLED) &&
                (info.flags & BASS_DEVICE_DEFAULT)) {
                deviceId = i;
                break;
            }
        }
    }

    if (deviceId < 0 || !startDevice(deviceId))
        return false;

    m_currentDevice = deviceId;
    return true;
}

int MediaManager::currentDevice() const
{
    return m_currentDevice;
}

bool MediaManager::startDevice(int deviceId)
{
    BASS_DEVICEINFO info = {};
    if (!BASS_GetDeviceInfo(deviceId, &info) ||
        (deviceId != 0 && !(info.flags & BASS_DEVICE_ENABLED)))
        return false;

    if (!(info.flags & BASS_DEVICE_INIT) &&
        !BASS_Init(deviceId, 44100, 0, nullptr, nullptr))
        return false;

    if (!BASS_SetDevice(deviceId))
        return false;

    // Un dispositivo puede seguir inicializado aunque su salida esté detenida.
    return BASS_Start();
}

void MediaManager::recoverDevice()
{
    if (!m_backend->stream) {
        m_deviceRecoveryTimer->stop();
        return;
    }

    const DWORD device = BASS_ChannelGetDevice(m_backend->stream);
    if (device == static_cast<DWORD>(-1)) {
        m_deviceRecoveryTimer->stop();
        return;
    }

    // No cambiar la selección de salida de otros reproductores en este hilo.
    const DWORD previousDevice = BASS_GetDevice();
    const bool recovered = startDevice(static_cast<int>(device));
    if (previousDevice != static_cast<DWORD>(-1))
        BASS_SetDevice(previousDevice);

    if (recovered) {
        m_deviceRecoveryTimer->stop();
        qDebug() << "Dispositivo de audio recuperado:" << device;
    }
}


//******************************
void MediaManager::fadeOut(int durationMs)
{
    if (!m_backend->stream)
        return;

    // Deslizar volumen hasta 0
    BASS_ChannelSlideAttribute(
        m_backend->stream,
        BASS_ATTRIB_VOL,
        0.0f,
        durationMs
    );

    // Sync cuando termina el slide
    BASS_ChannelSetSync(
        m_backend->stream,
        BASS_SYNC_SLIDE,
        0,
        &MediaManager::Backend::FadeOutSyncCallback,
        this
    );
}




//****************************************************

void CALLBACK MediaManager::Backend::EndSyncCallback(
        HSYNC,
        DWORD,
        DWORD,
        void *user)
{
    MediaManager* self = static_cast<MediaManager*>(user);

    if (!self) return;

    QMetaObject::invokeMethod(
        self,
        [self]()
        {
            if (self->m_timer)
                self->m_timer->stop();

            emit self->playbackFinished();
        },
        Qt::QueuedConnection
    );
}


void CALLBACK MediaManager::Backend::DeviceFailedSyncProc(
        HSYNC,
        DWORD channel,
        DWORD,
        void *user){

    MediaManager* self = static_cast<MediaManager*>(user);
    if (!self) return;

    QMetaObject::invokeMethod(self, [self, channel]() {
        if (channel != self->m_backend->stream)
            return;
        qDebug() << "Dispositivo de audio desconectado; esperando reconexión";
        self->m_deviceRecoveryTimer->start();
        self->recoverDevice();
    }, Qt::QueuedConnection);

}


//*****************************************
void CALLBACK MediaManager::Backend::FadeOutSyncCallback(
        HSYNC,
        DWORD channel,
        DWORD,
        void *user)
        {
            MediaManager* self = static_cast<MediaManager*>(user);
            if (!self) return;

            QMetaObject::invokeMethod(
                self,
                [self, channel]()
                {
                    // Restaurar volumen
                    BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, self->m_volume);

                    // Stop
                    BASS_ChannelStop(channel);

                    // Posición al inicio
                    BASS_ChannelSetPosition(channel, 0, BASS_POS_BYTE);

                    // Si es el canal actual, mantener coherencia
                    if (channel == self->m_backend->stream)
                    {
                        if (self->m_timer)
                            self->m_timer->stop();

                        emit self->playbackFinished();
                    }
                },
                Qt::QueuedConnection
            );
}

bool MediaManager::shouldStopBySilence(const AudioFrame& frame)
{
    float maxDb = std::max(frame.left, frame.right);

    // 1. Detectar si hubo audio real
    if (maxDb > m_soundThresholdDb)
    {
        m_hadSound = true;
    }

    // 2. Duración total
    double duration = BASS_ChannelBytes2Seconds(
        m_backend->stream,
        BASS_ChannelGetLength(m_backend->stream, BASS_POS_BYTE)
    );

    double current = frame.position;

    // margen dinámico
    double tailMargin = std::max(m_tailSeconds, duration * m_tailPercent);

    bool nearEnd = (duration - current) < tailMargin;

    // 3. Contador de silencio
    if (maxDb < m_silenceThresholdDb)
    {
        m_silenceCounter += 50;
    }
    else
    {
        m_silenceCounter = 0;
    }


    // 4. Decisión final
    return (nearEnd &&
            m_hadSound &&
            m_silenceCounter >= m_silenceDurationMs);
}
