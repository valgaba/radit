#ifndef FORMABOUT_H
#define FORMABOUT_H

#include "widgets/frame.h"

class QLabel;
class QNetworkAccessManager;

class FormAbout : public Frame
{
    Q_OBJECT
public:
    explicit FormAbout(QWidget *parent = nullptr);
    ~FormAbout() override = default;
protected:
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void checkLatestDownload();

    QLabel *m_buildDateLabel = nullptr;
    QLabel *m_downloadDateLabel = nullptr;
    QNetworkAccessManager *m_network = nullptr;
};

#endif // FORMABOUT_H
