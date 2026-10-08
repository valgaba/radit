#ifndef CONTENTSPLAYER_H
#define CONTENTSPLAYER_H




#include <QWidget>
#include <QVBoxLayout>
#include <QList>
#include <QFrame>
#include <QDebug>


#include "widgets/contentsbase.h"
//#include "widgets/menu.h"
#include "core/Clipboard.h"
//#include "widgets/AudioItemMaxi.h"
#include "widgets/ContentsMenu.h"




class ContentsPlayer: public ContentsBase
{

    Q_OBJECT


private:

    QPoint mousePos;
    bool isCut;  //para las operaciones de cortar pegar
    ContentsMenu *contentsMenu = nullptr;
    QString m_listFileName;
    bool saveListFile(const QString &filename);
    //Player *m_player = nullptr;

public:

    Clipboard &clipboard=Clipboard::instance();
    explicit ContentsPlayer(QWidget *parent = 0);
    ~ContentsPlayer();

    void nextAllItems();
    void purgeAllItems();
    void loopAllItems();
    void selectItems();

    void selectAllItems();
    void unSelectAllItems();

    void deleteSelected();
    void copySelected();
    void cutSelected();
    void pasteClipboard();
    void applyColor(const QColor &color);

    void loadItems();
    void saveItems();
    void saveAsItems();
    QString listFileName() const { return m_listFileName; }
    void setListFileName(const QString &filename) { m_listFileName = filename; }

    void setPlayer(Player *player);




protected:

    void contextMenuEvent(QContextMenuEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void setContextMenuPosition(const QPoint &position) { mousePos=position; }
    virtual bool supportsPlaybackOptionsInContextMenu() const { return true; }
    virtual bool supportsListOptionsInContextMenu() const { return true; }

private slots:


public slots:





};





#endif // CONTENTSPLAYER_H
