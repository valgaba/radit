#ifndef ELEVENLABSTTS_H
#define ELEVENLABSTTS_H

#include <QObject>
#include <functional>

class ElevenLabsTts final : public QObject
{
public:
    using Callback = std::function<void(const QString &audioPath, const QString &error)>;

    static ElevenLabsTts *instance();
    void generate(const QString &text, QObject *context, Callback callback);

private:
    explicit ElevenLabsTts(QObject *parent = nullptr);
};

#endif // ELEVENLABSTTS_H
