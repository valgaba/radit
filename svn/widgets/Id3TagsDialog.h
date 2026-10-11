#ifndef ID3TAGSDIALOG_H
#define ID3TAGSDIALOG_H

#include <QDialog>
#include <QPoint>
#include <QString>

struct AudioFileMetadata;
class QLineEdit;
class QLabel;
class Frame;

class Id3TagsDialog final : public QDialog
{
public:
    explicit Id3TagsDialog(const QString &filePath, QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void saveTags();

    QString m_filePath;
    QLineEdit *m_title = nullptr;
    QLineEdit *m_artist = nullptr;
    QLineEdit *m_album = nullptr;
    QLineEdit *m_year = nullptr;
    QLineEdit *m_genre = nullptr;
    QLineEdit *m_comment = nullptr;
    QLabel *m_status = nullptr;
    Frame *m_titleBar = nullptr;
    bool m_dragging = false;
    QPoint m_dragOffset;
};

#endif // ID3TAGSDIALOG_H
