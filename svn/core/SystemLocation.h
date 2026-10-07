#ifndef SYSTEMLOCATION_H
#define SYSTEMLOCATION_H
#include <QObject>
class QProcess;
class QTimer;
class SystemLocation : public QObject
{
    Q_OBJECT
public:
    explicit SystemLocation(QObject *parent = nullptr);
    ~SystemLocation() override;
    void request();
    void cancel();
signals:
    void locationReady(double latitude, double longitude);
    void locationFailed(const QString &message);
private:
    QProcess *m_process;
    QTimer *m_timeout;
    bool m_active = false;
    bool m_restart = false;
};
#endif
