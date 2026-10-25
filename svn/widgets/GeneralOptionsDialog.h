#ifndef GENERALOPTIONSDIALOG_H
#define GENERALOPTIONSDIALOG_H

#include <QDialog>
#include <QPoint>

class Frame;
class QLineEdit;
class QCheckBox;
class QSpinBox;

class GeneralOptionsDialog final : public QDialog
{
    Q_OBJECT
public:
    explicit GeneralOptionsDialog(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Frame *m_titleBar = nullptr;
    QLineEdit *m_sweeperFolder = nullptr;
    QCheckBox *m_scheduleTransitionsEnabled = nullptr;
    QLineEdit *m_scheduleTransitionFolder = nullptr;
    QSpinBox *m_minimumStandbyTrackSeconds = nullptr;
    QSpinBox *m_scheduleTransitionLateToleranceSeconds = nullptr;
    QPoint m_dragOffset;
};

#endif
