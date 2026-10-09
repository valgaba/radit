#ifndef SCHEDULESLOTOPTIONSDIALOG_H
#define SCHEDULESLOTOPTIONSDIALOG_H

#include <QDialog>
#include <QString>
#include <QTime>

class QTimeEdit;
class QCheckBox;
class QLineEdit;

class ScheduleSlotOptionsDialog final : public QDialog
{
public:
    explicit ScheduleSlotOptionsDialog(const QTime &initialTime, QWidget *parent = nullptr,
                                       bool disabled = false, bool editing = false,
                                       const QString &name = QString());
    QTime entryTime() const;
    QString name() const;
    bool isDisabled() const;

private:
    QTimeEdit *m_entryTime = nullptr;
    QCheckBox *m_disabled = nullptr;
    QLineEdit *m_name = nullptr;
};

#endif // SCHEDULESLOTOPTIONSDIALOG_H
