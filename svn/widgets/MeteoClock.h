#ifndef METEOCLOCK_H
#define METEOCLOCK_H
#include "widgets/frame.h"
#include "core/WeatherService.h"
class QLabel;
class QLineEdit;
class QComboBox;
class QTimer;
class Button;
class SystemLocation;

class MeteoClock : public Frame
{
    Q_OBJECT
public:
    struct Readings {
        double temperature = 0;
        int humidity = 0;
        QString location;
        QDateTime updated;
        bool available = false;
    };
    static Readings currentReadings();
    explicit MeteoClock(QWidget *parent = nullptr);
    ~MeteoClock() override;
    static QString formatDate(const QDate &date);
    void setLocation(const QString &name, double latitude, double longitude);
protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
private:
    void updateClock();
    void updateClockFont();
    void refreshWeather();
    void searchCity();
    void loadSettings();
    void saveSettings();
    void setStatus(const QString &text);
    WeatherService *m_weather;
    SystemLocation *m_systemLocation;
    QTimer *m_clockTimer, *m_weatherTimer;
    QLabel *m_time, *m_date, *m_temperature, *m_humidity, *m_location, *m_status;
    QWidget *m_settings;
    Button *m_options, *m_search, *m_apply, *m_autoLocation;
    QLineEdit *m_city;
    QComboBox *m_places;
    QList<WeatherPlace> m_results;
    QString m_locationName;
    double m_latitude = 0, m_longitude = 0;
    bool m_hasLocation = false;
    QDateTime m_lastUpdate;
};
#endif
