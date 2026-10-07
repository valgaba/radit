/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "core/SystemLocation.h"
#include <QProcess>
#include <QTimer>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <cmath>

SystemLocation::SystemLocation(QObject *parent) : QObject(parent)
{
    m_process = new QProcess(this);
    m_timeout = new QTimer(this); m_timeout->setSingleShot(true); m_timeout->setInterval(20000);
#ifdef Q_OS_WIN
    m_process->setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *args) { args->flags |= 0x08000000; }); // CREATE_NO_WINDOW
#endif
    connect(m_timeout, &QTimer::timeout, this, [this]() {
        cancel(); emit locationFailed(tr("System location timed out. Choose a city manually."));
    });
    connect(m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (!m_active || error != QProcess::FailedToStart) return;
        m_active = false; m_timeout->stop();
        emit locationFailed(tr("System location is unavailable. Choose a city manually."));
    });
    connect(m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, [this](int code, QProcess::ExitStatus status) {
        if (m_restart) { m_restart = false; request(); return; }
        if (!m_active) return;
        m_active = false; m_timeout->stop();
        const auto object = QJsonDocument::fromJson(m_process->readAllStandardOutput()).object();
        const double latitude = object["latitude"].toDouble(1000), longitude = object["longitude"].toDouble(1000);
        if (status != QProcess::NormalExit || code != 0 || !object["latitude"].isDouble() || !object["longitude"].isDouble()
            || !std::isfinite(latitude) || !std::isfinite(longitude) || latitude < -90 || latitude > 90 || longitude < -180 || longitude > 180) {
            emit locationFailed(tr("Windows location is disabled or unavailable. Choose a city manually.")); return;
        }
        emit locationReady(latitude, longitude);
    });
}
SystemLocation::~SystemLocation()
{
    cancel();
    if (m_process->state() != QProcess::NotRunning) m_process->waitForFinished(1000);
}
void SystemLocation::cancel()
{
    m_active = false; m_restart = false; m_timeout->stop();
    if (m_process->state() != QProcess::NotRunning) m_process->kill();
}
void SystemLocation::request()
{
    if (m_process->state() != QProcess::NotRunning) {
        if (!m_active) m_restart = true;
        return;
    }
#ifdef Q_OS_WIN
    // Windows PowerShell provides WinRT interop with the built-in Windows location API.
    // This fixed script contains no user input and never changes location permissions.
    const QString script = QStringLiteral(R"PS(
$ErrorActionPreference='Stop'
try {
 Add-Type -AssemblyName System.Runtime.WindowsRuntime
 [Windows.Devices.Geolocation.Geolocator, Windows.Devices.Geolocation, ContentType=WindowsRuntime] > $null
 [Windows.Devices.Geolocation.Geoposition, Windows.Devices.Geolocation, ContentType=WindowsRuntime] > $null
 $meteoMethod=[System.WindowsRuntimeSystemExtensions].GetMethods() | Where-Object { $_.Name -eq 'AsTask' -and $_.IsGenericMethod -and $_.GetParameters().Count -eq 1 -and $_.GetParameters()[0].ParameterType.Name -eq 'IAsyncOperation`1' } | Select-Object -First 1
 $meteoLocator=New-Object Windows.Devices.Geolocation.Geolocator
 $meteoOperation=$meteoLocator.GetGeopositionAsync([TimeSpan]::FromMinutes(10),[TimeSpan]::FromSeconds(12))
 $meteoTask=$meteoMethod.MakeGenericMethod([Windows.Devices.Geolocation.Geoposition]).Invoke($null,@($meteoOperation))
 if (-not $meteoTask.Wait(15000)) { exit 2 }
 $meteoPosition=$meteoTask.Result.Coordinate.Point.Position
 @{latitude=$meteoPosition.Latitude;longitude=$meteoPosition.Longitude} | ConvertTo-Json -Compress
 exit 0
} catch { exit 1 }
)PS");
    const QByteArray encoded = QByteArray(reinterpret_cast<const char*>(script.utf16()), script.size()*2).toBase64();
    const QString executable = QDir(qEnvironmentVariable("WINDIR", "C:/Windows")).filePath("System32/WindowsPowerShell/v1.0/powershell.exe");
    m_active = true; m_timeout->start();
    m_process->start(executable, {"-NoLogo", "-NoProfile", "-NonInteractive", "-WindowStyle", "Hidden", "-EncodedCommand", QString::fromLatin1(encoded)});
#else
    emit locationFailed(tr("System location is unavailable. Choose a city manually."));
#endif
}
