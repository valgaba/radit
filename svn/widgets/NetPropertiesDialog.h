#ifndef NETPROPERTIESDIALOG_H
#define NETPROPERTIESDIALOG_H

#include <QDialog>
#include <QPoint>

class Frame;
class QLineEdit;
class QTimeEdit;

class NetPropertiesDialog final : public QDialog
{
    Q_OBJECT

public:
    NetPropertiesDialog(const QString &name, const QString &url,
                        int connectionDurationSeconds = -1,
                        QWidget *parent = nullptr);

    QString stationName() const;
    QString streamUrl() const;
    bool hasConnectionDuration() const;
    int connectionDurationSeconds() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QLineEdit *m_name = nullptr;
    QLineEdit *m_url = nullptr;
    QTimeEdit *m_connectionDuration = nullptr;
    Frame *m_titleBar = nullptr;
    bool m_dragging = false;
    QPoint m_dragOffset;
};

#endif // NETPROPERTIESDIALOG_H
