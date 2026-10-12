#ifndef TABPLAYER_H
#define TABPLAYER_H



//TabPlayer


#include "widgets/tab.h"
#include <QPointer>
class AudioItemMaxi;
class TapPlayerMenu;
class QMimeData;


class TabPlayer: public Tab
{

    Q_OBJECT


private:
        TapPlayerMenu *menu;
        QPointer<QWidget> m_colorTarget;
        QString m_playerFileName;
        bool m_modified = false;
        void markModified();
        void connectContentsModified(QWidget *container);
        bool savePlayerFile(bool saveAs);


public:



    explicit TabPlayer(QWidget *parent = 0);
    ~TabPlayer();

 void closeTab(int index);
 QString playerFileName() const { return m_playerFileName; }
 void setPlayerFileName(const QString &filename);
 bool isModified() const { return m_modified; }
 bool confirmSaveChanges();
 static QString droppedPlayerFile(const QMimeData *data);
 bool loadDroppedPlayer(const QMimeData *data);
 bool loadListFile(const QString &filename, QString *error = nullptr);



protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;


private slots:
    void loadPlayer();
    void loadPlayerFile(const QString &filename);
    void savePlayer();
    void savePlayerAs();

public slots:

    signals:
        void requestPlayItem(AudioItemMaxi* item);
        void playerFileNameChanged(const QString &filename);
        void modifiedChanged(bool modified);

};







#endif // TABPLAYER_H
