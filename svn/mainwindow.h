#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QList>


#include "core/MediaManager.h"

class Player;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

   // QList<QWidget*>  clipboardlist; //para copiar multiples
     QList<QWidget*> *clipboardlist;



    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();



private:

    QWidget *centralwidget;
    QSplitter *splitterprincipal;
    QSplitter *splittertop;
    QSplitter *splitterdown;
    MediaManager * mediamanager;

    QList<Player*> players;


};
#endif // MAINWINDOW_H
