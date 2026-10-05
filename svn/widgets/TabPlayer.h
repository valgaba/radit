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


public:



    explicit TabPlayer(QWidget *parent = 0);
    ~TabPlayer();

 void closeTab(int index);
 QString playerFileName() const { return m_playerFileName; }
 void setPlayerFileName(const QString &filename);
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

};







#endif // TABPLAYER_H
