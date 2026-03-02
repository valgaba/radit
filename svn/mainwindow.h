#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>

#include "core/MediaManager.h"


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




};
#endif // MAINWINDOW_H
