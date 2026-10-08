#ifndef SCHEDULESLOTOPTIONSDIALOG_H
#define SCHEDULESLOTOPTIONSDIALOG_H

#include <QDialog>
#include <QTime>

class QTimeEdit;
class QCheckBox;

class ScheduleSlotOptionsDialog final : public QDialog
{
public:
    explicit ScheduleSlotOptionsDialog(const QTime &initialTime, QWidget *parent = nullptr,
                                       bool disabled = false, bool editing = false);
    QTime entryTime() const;
    bool isDisabled() const;

private:
    QTimeEdit *m_entryTime = nullptr;
    QCheckBox *m_disabled = nullptr;
};

#endif // SCHEDULESLOTOPTIONSDIALOG_H
