#ifndef LOADINGDIALOG_H
#define LOADINGDIALOG_H

#include "widgets/LoadingProgress.h"
#include "widgets/button.h"
#include <functional>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QElapsedTimer>
#include <QApplication>
#include <QEventLoop>
#include <QMouseEvent>
#include <QWindow>

// A scoped loading window: closing or returning on error removes it automatically.
// Supports modal document imports and non-modal asynchronous file imports.
class LoadingDialog : public QDialog
{
public:
    LoadingDialog(QWidget *parent, const QString &message,
                  Qt::WindowModality modality = Qt::ApplicationModal)
        : QDialog(parent, Qt::Dialog | Qt::FramelessWindowHint)
    {
        setObjectName("LoadingDialog");
        setWindowModality(modality);
        if (modality == Qt::NonModal) setAttribute(Qt::WA_ShowWithoutActivating);
        setWindowTitle(message);
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(10, 8, 10, 8);
        layout->setSpacing(4);
        m_label = new QLabel(message, this);
        m_label->setAttribute(Qt::WA_TransparentForMouseEvents);
        layout->addWidget(m_label);
        m_progress = new LoadingProgress(this);
        layout->addWidget(m_progress);
        setFixedSize(340, 64);
        m_progress->begin(message);
        if (parent) move(parent->window()->frameGeometry().center() - rect().center());
        show();
        m_refresh.start();
        refresh(true);
    }

    void setDetail(const QString &detail)
    {
        m_label->setText(m_label->fontMetrics().elidedText(detail, Qt::ElideMiddle, width() - 20));
        m_label->setToolTip(detail);
    }

    void setProgress(int completed, int total)
    {
        m_progress->setProgress(completed, total);
        refresh(completed == total);
    }

    void refresh(bool force = false)
    {
        // Asynchronous tasks return to the normal event loop themselves.
        if (windowModality() == Qt::NonModal) return;
        if (force || m_refresh.elapsed() >= 33) {
            QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            m_refresh.restart();
        }
    }

    void setCancelHandler(const std::function<void()> &handler)
    {
        m_cancelHandler = handler;
        if (!m_cancelButton) {
            m_cancelButton = new Button(this);
            m_cancelButton->setObjectName("LoadingCancel");
            m_cancelButton->setText(tr("Cancel"));
            m_cancelButton->setFixedSize(90, 24);
            m_cancelButton->setAccessibleName(tr("Cancel loading"));
            static_cast<QVBoxLayout*>(layout())->addWidget(m_cancelButton, 0, Qt::AlignRight);
            connect(m_cancelButton, &QPushButton::clicked, this, &LoadingDialog::reject);
            setFixedHeight(92);
        }
    }

    void reject() override
    {
        const auto handler = m_cancelHandler;
        if (handler) handler();
    }

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            if (windowHandle() && windowHandle()->startSystemMove()) {
                event->accept();
                return;
            }
            m_dragging = true;
            m_dragOffset = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
            return;
        }
        QDialog::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (m_dragging && (event->buttons() & Qt::LeftButton)) {
            move(event->globalPosition().toPoint() - m_dragOffset);
            event->accept();
            return;
        }
        QDialog::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (m_dragging && event->button() == Qt::LeftButton) {
            m_dragging = false;
            event->accept();
            return;
        }
        QDialog::mouseReleaseEvent(event);
    }

private:
    bool m_dragging = false;
    QPoint m_dragOffset;
    LoadingProgress *m_progress;
    QLabel *m_label;
    QElapsedTimer m_refresh;
    Button *m_cancelButton = nullptr;
    std::function<void()> m_cancelHandler;
};

#endif
