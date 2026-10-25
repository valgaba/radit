#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QList>
#include <QString>


#include "core/MediaManager.h"

class Player;
class FileExplore;
class Capture;
class Cast;
class MeteoClock;
class Planner;
class EmissionLog;
class EmissionLogFrame;
class QAction;

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
    void saveWindowInterface() const;
    void saveWindowInterfacePreset(int preset) const;
    void saveWindowInterfaceToGroup(const QString &groupName) const;
    void restoreWindowInterface();
    bool restoreWindowInterfacePreset(int preset);
    bool restoreWindowInterfaceFromGroup(const QString &groupName);
    bool m_quitDialogOpen = false;
    void restoreInterface();
    void applyWorkspacePreset(bool plannerView);
    void clearWorkspacePresetSelection();

    QWidget *centralwidget;
    QSplitter *splitterprincipal;
    QSplitter *splittertop;
    QSplitter *splitterdown;
    MediaManager * mediamanager;

    QList<Player*> players;
    FileExplore *m_fileExplore = nullptr;
    Capture *m_capture = nullptr;
    Cast *m_cast = nullptr;
    MeteoClock *m_meteoClock = nullptr;
    Planner *m_planner = nullptr;
    EmissionLog *m_emissionLog = nullptr;
    EmissionLogFrame *m_emissionLogFrame = nullptr;
    QAction *m_studioViewAction = nullptr;
    QAction *m_plannerViewAction = nullptr;


};
#endif // MAINWINDOW_H
