#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QList>


#include "core/MediaManager.h"

class Player;
class FileExplore;
class Capture;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

   // QList<QWidget*>  clipboardlist; //para copiar multiples
     QList<QWidget*> *clipboardlist;



    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();



protected:
    void closeEvent(QCloseEvent *event) override;

private:
    bool saveBeforeQuit();
    bool m_quitDialogOpen = false;
    void restoreInterface();

    QWidget *centralwidget;
    QSplitter *splitterprincipal;
    QSplitter *splittertop;
    QSplitter *splitterdown;
    MediaManager * mediamanager;

    QList<Player*> players;
    FileExplore *m_fileExplore = nullptr;
    Capture *m_capture = nullptr;


};
#endif // MAINWINDOW_H
