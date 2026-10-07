#ifndef WEATHERSERVICE_H
#define WEATHERSERVICE_H
#include <QObject>
#include <QDateTime>
#include <QPointer>
#include <QList>
class QNetworkAccessManager;
class QNetworkReply;
struct WeatherPlace {
    QString name;
    double latitude = 0, longitude = 0;
};
Q_DECLARE_METATYPE(WeatherPlace)
Q_DECLARE_METATYPE(QList<WeatherPlace>)

class WeatherService : public QObject
{
    Q_OBJECT
public:
    explicit WeatherService(QObject *parent = nullptr, QNetworkAccessManager *network = nullptr);
    ~WeatherService() override;
    void searchCity(const QString &name);
    void requestCurrent(double latitude, double longitude);
    void cancel();
    void cancelSearch();
signals:
    void placesReady(const QList<WeatherPlace> &places);
    void searchFailed(const QString &message);
    void currentReady(double temperature, int humidity, const QDateTime &observed);
    void weatherFailed(const QString &message);
private:
    QNetworkAccessManager *m_network;
    QPointer<QNetworkReply> m_searchReply, m_weatherReply;
};
#endif
