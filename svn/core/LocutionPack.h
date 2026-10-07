#ifndef LOCUTIONPACK_H
#define LOCUTIONPACK_H
#include <QObject>
#include <QTemporaryDir>
#include <memory>
class QProcess;
class QTimer;

class LocutionPack : public QObject
{
    Q_OBJECT
public:
    static std::shared_ptr<LocutionPack> open(const QString &zipPath);
    ~LocutionPack() override;
    bool isReady() const { return m_ready; }
    bool isLoading() const { return m_loading; }
    QString error() const { return m_error; }
    QString clip(const QString &name) const;
signals:
    void finished();
private:
    explicit LocutionPack(const QString &zipPath);
    void complete(const QString &error);
    QTemporaryDir m_directory;
    QProcess *m_process;
    QTimer *m_timeout;
    bool m_ready = false, m_loading = true;
    QString m_error;
};
#endif
