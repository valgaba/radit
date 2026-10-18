#ifndef EMISSIONLOGFRAME_H
#define EMISSIONLOGFRAME_H

#include "widgets/frame.h"

class EmissionLog;
class QDateEdit;
class QLineEdit;
class QTableWidget;
class QShowEvent;

class EmissionLogFrame final : public Frame
{
    Q_OBJECT
public:
    explicit EmissionLogFrame(EmissionLog *log, QWidget *parent = nullptr);

private:
    EmissionLog *m_log = nullptr;
    QDateEdit *m_fromDate = nullptr;
    QDateEdit *m_toDate = nullptr;
    QLineEdit *m_search = nullptr;
    QTableWidget *m_table = nullptr;
    void refresh();
    void exportCsv();
    void clearHistory();

protected:
    void showEvent(QShowEvent *event) override;
};

#endif // EMISSIONLOGFRAME_H
