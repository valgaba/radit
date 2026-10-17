/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org

   Radit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Radit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Radit. If not, see <http://www.gnu.org/licenses/>.
*/

#include "widgets/FormAbout.h"
#include <QCoreApplication>
#include <QDate>
#include <QDateTime>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLocale>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QScreen>
#include <QSvgRenderer>
#include <QRegularExpression>
#include <QVBoxLayout>

FormAbout::FormAbout(QWidget *parent) : Frame(parent)
{
    setObjectName("FormAbout");
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("About Radit"));
    setWindowIcon(QIcon(":/icons/radit.ico"));
    setMinimumSize(780, 320);
    resize(900, 350);

    auto *columns = new QHBoxLayout(this);
    columns->setContentsMargins(40, 28, 40, 26);
    columns->setSpacing(24);
    const auto label = [this](const QString &text, const QString &name) {
        auto *item = new QLabel(text, this);
        item->setObjectName(name);
        item->setAlignment(Qt::AlignCenter);
        item->setWordWrap(true);
        return item;
    };

    auto *left = new QVBoxLayout;
    left->setSpacing(12);
    left->addStretch();
    left->addWidget(label(tr("Audio playback and recording\nfor radio and theatre"), "AboutDescription"));
    left->addSpacing(10);
    left->addWidget(label(tr("Developed by"), "AboutMuted"));
    left->addWidget(label("Victor Algaba", "AboutAuthor"));
    left->addStretch();
    auto *website = label("<a href=\"https://www.radit.org\" style=\"color:#80A4AE;text-decoration:none\">www.radit.org</a>", "AboutWebsite");
    website->setOpenExternalLinks(true);
    website->setToolTip(tr("Visit the Radit website"));
    left->addWidget(website);
    columns->addLayout(left, 1);

    auto *center = new QVBoxLayout;
    center->setSpacing(5);
    center->addStretch();
    auto *logo = label(QString(), "AboutLogo");
    logo->setFixedSize(190, 180);
    QPixmap logoImage(QSize(380, 360));
    logoImage.fill(Qt::transparent);
    logoImage.setDevicePixelRatio(2);
    QPainter logoPainter(&logoImage);
    QSvgRenderer renderer(QString(":/icons/RaditAppIcon.svg"));
    // Keep the large letter's original size while reusing the app icon effects.
    renderer.setViewBox(QRectF(10, 8, 190, 180));
    renderer.render(&logoPainter, QRectF(0, 0, 190, 180));
    // Qt SVG does not apply the icon's clipPath; mask with the original letter.
    QPixmap letterMask(logoImage.size());
    letterMask.fill(Qt::transparent);
    letterMask.setDevicePixelRatio(2);
    QPainter maskPainter(&letterMask);
    QSvgRenderer letterRenderer(QString(":/icons/RaditLogo.svg"));
    letterRenderer.render(&maskPainter, QRectF(0, 0, 190, 180));
    maskPainter.end();
    logoPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    logoPainter.drawPixmap(0, 0, letterMask);
    logoPainter.end();
    logo->setPixmap(logoImage);
    logo->setAccessibleName(tr("Radit logo"));
    center->addWidget(logo, 0, Qt::AlignHCenter);
    center->addWidget(label("radit", "AboutName"));
    center->addStretch();
    const QString version = QCoreApplication::applicationVersion().isEmpty()
        ? QStringLiteral("1.0.0") : QCoreApplication::applicationVersion();
    center->addWidget(label(tr("Version %1").arg(version), "AboutVersion"));
    columns->addLayout(center, 1);

    auto *right = new QVBoxLayout;
    right->setSpacing(8);
    right->addStretch();
    right->addWidget(label(tr("Audio engine"), "AboutMuted"));
    right->addWidget(label("BASS · Un4seen Developments", "AboutCredits"));
    right->addSpacing(6);
    right->addWidget(label(tr("MP3 encoding"), "AboutMuted"));
    right->addWidget(label("FFmpeg · LAME", "AboutCredits"));
    right->addSpacing(6);
    right->addWidget(label("GNU GPL v3 or later", "AboutCredits"));
    right->addSpacing(8);
    const QDate buildDate = QLocale(QLocale::English).toDate(
        QString::fromLatin1(__DATE__).simplified(), QStringLiteral("MMM d yyyy"));
    m_buildDateLabel = label(tr("Installed binaries: %1").arg(
        buildDate.isValid() ? buildDate.toString("dd/MM/yyyy") : tr("Unknown")), "AboutBuildDate");
    right->addWidget(m_buildDateLabel);
    m_downloadDateLabel = label(tr("Latest downloadable binaries: %1").arg(tr("Checking…")), "AboutDownloadDate");
    right->addWidget(m_downloadDateLabel);
    right->addStretch();
    right->addWidget(label(QString("© %1 Victor Algaba").arg(QDate::currentDate().year()), "AboutCopyright"));
    columns->addLayout(right, 1);

    m_network = new QNetworkAccessManager(this);
    checkLatestDownload();
}

void FormAbout::checkLatestDownload()
{
    // The website currently replaces the binary asset on this stable release tag.
    // Read the asset's updated_at value so older installed builds can show when a
    // newer ZIP became available, without treating the tag's original release date
    // as the date of every subsequent binary upload.
    const QUrl url(QStringLiteral("https://api.github.com/repos/valgaba/radit/releases/tags/dev-2026-10-05"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Radit"));
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setTransferTimeout(10000);
    auto *reply = m_network->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (!m_downloadDateLabel)
            return;
        if (reply->error() != QNetworkReply::NoError) {
            m_downloadDateLabel->setText(tr("Latest downloadable binaries: %1").arg(tr("Unavailable")));
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            m_downloadDateLabel->setText(tr("Latest downloadable binaries: %1").arg(tr("Unavailable")));
            return;
        }

        const QJsonArray assets = document.object().value(QStringLiteral("assets")).toArray();
        QJsonObject selectedAsset;
        for (const QJsonValue &value : assets) {
            const QJsonObject asset = value.toObject();
            const QString name = asset.value(QStringLiteral("name")).toString();
            if (name.compare(QStringLiteral("radit_1_0_0.zip"), Qt::CaseInsensitive) == 0
                || name.compare(QStringLiteral("binaries.zip"), Qt::CaseInsensitive) == 0) {
                selectedAsset = asset;
                if (name.compare(QStringLiteral("radit_1_0_0.zip"), Qt::CaseInsensitive) == 0)
                    break;
            }
        }

        const QString uploadedAtText = selectedAsset.value(QStringLiteral("updated_at")).toString();
        QDateTime uploadedAt = QDateTime::fromString(uploadedAtText, Qt::ISODateWithMs);
        if (!uploadedAt.isValid())
            uploadedAt = QDateTime::fromString(uploadedAtText, Qt::ISODate);
        if (selectedAsset.isEmpty() || !uploadedAt.isValid()) {
            m_downloadDateLabel->setText(tr("Latest downloadable binaries: %1").arg(tr("Unavailable")));
            return;
        }

        QString downloadVersion = QStringLiteral("1.0.0");
        const QString assetName = selectedAsset.value(QStringLiteral("name")).toString();
        const QRegularExpression versionPattern(QStringLiteral("^radit_(\\d+)_(\\d+)_(\\d+)\\.zip$"),
                                                QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch match = versionPattern.match(assetName);
        if (match.hasMatch())
            downloadVersion = QStringLiteral("%1.%2.%3").arg(match.captured(1), match.captured(2), match.captured(3));

        const QString date = uploadedAt.toLocalTime().date().toString(QStringLiteral("dd/MM/yyyy"));
        m_downloadDateLabel->setText(tr("Latest downloadable binaries: %1").arg(
            QStringLiteral("%1 · %2").arg(downloadVersion, date)));
    });
}

void FormAbout::showEvent(QShowEvent *event)
{
    const QRect bounds = parentWidget() ? parentWidget()->window()->frameGeometry()
                                       : screen()->availableGeometry();
    move(bounds.center() - rect().center());
    Frame::showEvent(event);
}

void FormAbout::paintEvent(QPaintEvent *event)
{
    Frame::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const qreal middle = height() * 0.67;
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(56, 176, 209, 32));
    painter.drawRect(QRectF(0, middle, width() * 0.36, 5));
    painter.setBrush(QColor(56, 176, 209, 14));
    painter.drawRect(QRectF(width() * 0.61, middle, width() * 0.39, 2));
    painter.setBrush(QColor("#38b0d1"));
    painter.drawRect(QRectF(0, middle, 30, 5));
    QPolygonF slash;
    slash << QPointF(35, middle) << QPointF(42, middle)
          << QPointF(37, middle + 5) << QPointF(30, middle + 5);
    painter.drawPolygon(slash);
}

void FormAbout::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
        return;
    }
    Frame::keyPressEvent(event);
}
