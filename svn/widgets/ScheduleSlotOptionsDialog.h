#ifndef SCHEDULESLOTOPTIONSDIALOG_H
#define SCHEDULESLOTOPTIONSDIALOG_H

#include <QDialog>
#include <QTime>

class QTimeEdit;

class ScheduleSlotOptionsDialog final : public QDialog
{
public:
    explicit ScheduleSlotOptionsDialog(const QTime &initialTime, QWidget *parent = nullptr);
    QTime entryTime() const;

private:
    QTimeEdit *m_entryTime = nullptr;
};

#endif // SCHEDULESLOTOPTIONSDIALOG_H
