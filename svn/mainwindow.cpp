/* This file is part of Coundown
   Copyright 2024, Victor Algaba <victorengine@gmail.com>

   Coundown is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Iradit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Coundown.  If not, see <http://www.gnu.org/licenses/>.
*/


// release/application_res.o] Error 1 puede ser el puto icono radit.ico que no esta ruta




#include "mainwindow.h"
#include <QHBoxLayout>
#include <QDebug>
#include <QLabel>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QApplication>
#include <QMessageBox>
#include <QCloseEvent>
#include <QScopedValueRollback>
#include <QDir>
#include <QSettings>
#include <QKeySequence>
#include <QActionGroup>
#include "widgets/QuitDialog.h"


//#include "widgets/TabPlayer.h"
//#include "widgets/container.h"
//#include "widgets/TabAuto.h"
#include "widgets/FormAbout.h"
#include "core/config.h"
#include "core/EmissionLog.h"
#include "widgets/fileexplore.h"
#include "widgets/Capture.h"
#include "widgets/Cast.h"
#include "widgets/MeteoClock.h"
#include "widgets/Planner.h"
#include "widgets/TabPlayer.h"
#include "widgets/Player.h"
#include "widgets/ElevenLabsSettingsDialog.h"
#include "widgets/EmissionLogFrame.h"


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent){



    mediamanager = new MediaManager(this);
    mediamanager->initialize(); //iniciamos audio
    m_emissionLog=new EmissionLog(this);



    // Crear la barra de menú
    QMenuBar *menubar =new QMenuBar;
    menubar->setObjectName("menubar"); //para qss


       // Añadir los menús principales (sin submenús por ahora)
       QMenu *fileMenu = menubar->addMenu(tr("&File"));
       fileMenu->setObjectName("FileMenu");
       fileMenu->setSeparatorsCollapsible(false);
       const QString settingsFile=QDir(QCoreApplication::applicationDirPath())
                                      .filePath(QStringLiteral("interface.ini"));
       QSettings languageSettings(settingsFile,QSettings::IniFormat);
       const QString uiLanguage=languageSettings.value(QStringLiteral("Interface/language"),
                                                         QStringLiteral("en")).toString();
       QMenu *languageMenu=fileMenu->addMenu(tr("Language"));
       auto *languageGroup=new QActionGroup(languageMenu);
       languageGroup->setExclusive(true);
       QAction *englishAction=languageMenu->addAction(tr("English"));
       englishAction->setCheckable(true);
       englishAction->setChecked(uiLanguage!=QLatin1String("es"));
       languageGroup->addAction(englishAction);
       QAction *spanishAction=languageMenu->addAction(tr("Spanish"));
       spanishAction->setCheckable(true);
       spanishAction->setChecked(uiLanguage==QLatin1String("es"));
       languageGroup->addAction(spanishAction);
       const auto setLanguage=[this,settingsFile](const QString &language) {
           QSettings settings(settingsFile,QSettings::IniFormat);
           if (settings.value(QStringLiteral("Interface/language"),QStringLiteral("en")).toString()
               ==language)
               return;
           settings.setValue(QStringLiteral("Interface/language"),language);
           settings.sync();
           QMessageBox::information(this,tr("Language"),
                                    tr("Restart Radit to apply the language change."));
       };
       connect(englishAction,&QAction::triggered,this,[setLanguage]() { setLanguage(QStringLiteral("en")); });
       connect(spanishAction,&QAction::triggered,this,[setLanguage]() { setLanguage(QStringLiteral("es")); });
       fileMenu->addSeparator();
       QAction *quitAction = fileMenu->addAction(tr("Quit"));
       quitAction->setObjectName("QuitAction");
       quitAction->setShortcut(QKeySequence::Quit);
       quitAction->setMenuRole(QAction::QuitRole);
       connect(quitAction, &QAction::triggered, this, &QWidget::close);
       QMenu *vistasMenu = menubar->addMenu(tr("&View"));

       QMenu *toolsMenu = menubar->addMenu(tr("&Tools"));
       QMenu *aiVoicesMenu = toolsMenu->addMenu(tr("AI voices (experimental)"));
       QAction *aiVoiceSettings = aiVoicesMenu->addAction(tr("Configure AI voices..."));
       aiVoiceSettings->setObjectName("ConfigureAIVoicesAction");
       connect(aiVoiceSettings, &QAction::triggered, this, [this]() {
           ElevenLabsSettingsDialog dialog(this);
           dialog.exec();
       });


       // Crear submenús
       QMenu *ayudaMenu = menubar->addMenu(tr("&Help"));

       // Añadir acciones al submenú de ayuda
       QAction *accionAyuda = new QAction(tr("Help"), this);
       QAction *accionWeb = new QAction(tr("Visit website"), this);

       QAction *accionAcercaDe = new QAction(tr("About"), this);

       ayudaMenu->addAction(accionAyuda);
       ayudaMenu->addAction(accionWeb);
       ayudaMenu->addSeparator();

       ayudaMenu->addAction(accionAcercaDe);



       connect(accionWeb, &QAction::triggered, this, []() {
           QDesktopServices::openUrl(QUrl("http://www.radit.org"));
       });

       connect(accionAcercaDe, &QAction::triggered, this, [this]() {
           FormAbout *frmabout = new FormAbout(this);
           frmabout->show();
       });




        this->setMenuBar(menubar);





     //Clipboard &clipboard = Clipboard::instance();


    this->setWindowTitle("Radit[]");



    this->setMinimumSize(200, 200); // si no pongo esto  no se maximiza al pincipio, una puta mierda
   // clipboardlist = new QList<QWidget*>; // crea un objeto grobal para la gestion de un porta papeles

    centralwidget = new QWidget; //widget principal
    this->setCentralWidget(centralwidget);

    splitterprincipal = new QSplitter(centralwidget); // general
    splitterprincipal->setGeometry(QRect(80, 60, 561, 391));
    splitterprincipal->setOrientation(Qt::Vertical);
    splitterprincipal->setHandleWidth(1); //grosor linea separador




    splittertop = new QSplitter(splitterprincipal); //cuadricula parte alta
    splittertop->setOrientation(Qt::Horizontal);
    splittertop->setHandleWidth(1);







    //splitterprincipal->addWidget(splittertop);

    splitterdown = new QSplitter(splitterprincipal); //cuadricula parte baja
    splitterdown->setOrientation(Qt::Horizontal);
    splitterdown->setHandleWidth(1);


    QList<int> sizes;
    sizes << 1 << 2; // permite especificar los tamaños de los widgets secundarios del divisor.
    splitterprincipal->setSizes(sizes);



    QVBoxLayout  *layoutprincipal = new QVBoxLayout(centralwidget); //layout espacio global vertical
    layoutprincipal->setSpacing(0);
    layoutprincipal->setContentsMargins(0, 0, 0, 0);
    layoutprincipal->addWidget(splitterprincipal);


    // ----------------------------------------
    // Crear Players
    // ----------------------------------------

    Player *player1 = new Player;
    Player *player2 = new Player;
    Player *player3 = new Player;
    Player *player4 = new Player;



    player1->setObjectName("Player1");
    player2->setObjectName("Player2");
    player3->setObjectName("Player3");
    player4->setObjectName("Player4");





    // Guardar referencias
    players << player1
            << player2
            << player3
            << player4;



    // ----------------------------------------
    // Añadir Players a la interfaz
    // ----------------------------------------

   // splittertop->addWidget(player1);
    m_fileExplore = new FileExplore;
    m_capture = new Capture;
    m_cast = new Cast;
    m_meteoClock = new MeteoClock;
    m_planner = new Planner;
    m_emissionLogFrame=new EmissionLogFrame(m_emissionLog);
    m_emissionLogFrame->hide();

    splittertop->addWidget(m_fileExplore);
    splittertop->addWidget(m_capture);
    splittertop->addWidget(m_cast);
    splittertop->addWidget(m_meteoClock);

    splittertop->addWidget(m_planner);
    splittertop->addWidget(m_emissionLogFrame);

    splitterdown->addWidget(player2);
    splitterdown->addWidget(player3);
    splitterdown->addWidget(player4);


    // Los paneles superiores comparten el ancho inicial.
    splittertop->setStretchFactor(0, 2);
    splittertop->setStretchFactor(1, 2);
    splittertop->setStretchFactor(2, 2);
    splittertop->setStretchFactor(3, 2);
    splittertop->setStretchFactor(4, 3);
    splittertop->setStretchFactor(5, 3);
    QList<int> sizesTop;
    sizesTop << 400 << 400 << 400 << 400 << 600 << 0;
    splittertop->setSizes(sizesTop);

    QAction *resetInterface = vistasMenu->addAction(tr("Reset User Interface"));
    connect(resetInterface, &QAction::triggered, this, &MainWindow::restoreInterface);
    vistasMenu->addSeparator();

    m_studioViewAction=vistasMenu->addAction(tr("Studio view"));
    m_studioViewAction->setCheckable(true);
    connect(m_studioViewAction,&QAction::triggered,this,[this]() {
        applyWorkspacePreset(false);
    });
    m_plannerViewAction=vistasMenu->addAction(tr("Planner view"));
    m_plannerViewAction->setCheckable(true);
    connect(m_plannerViewAction,&QAction::triggered,this,[this]() {
        applyWorkspacePreset(true);
    });
    vistasMenu->addSeparator();

    const auto addPanelAction = [this, vistasMenu](const QString &text, QWidget *panel) {
        QAction *action = vistasMenu->addAction(text);
        action->setCheckable(true);
        action->setChecked(!panel->isHidden());
        connect(action, &QAction::triggered, this, [this,panel](bool visible) {
            clearWorkspacePresetSelection();
            panel->setVisible(visible);
            if (panel->parentWidget()==splittertop) {
                QList<int> sizes=splittertop->sizes();
                const int panelIndex=splittertop->indexOf(panel);
                if (panelIndex>=0 && panelIndex<sizes.size()) {
                    if (visible && sizes[panelIndex]==0) {
                        int donor=-1;
                        for (int index=0;index<sizes.size();++index)
                            if (index!=panelIndex && sizes[index]>(donor<0 ? 0 : sizes[donor]))
                                donor=index;
                        const int extra=qMin(280,donor>=0 ? sizes[donor]/2 : 0);
                        if (donor>=0 && extra>0) {
                            sizes[donor]-=extra;
                            sizes[panelIndex]=extra;
                        }
                    } else if (!visible) {
                        sizes[panelIndex]=0;
                    }
                    splittertop->setSizes(sizes);
                }
            }
            if (panel->parentWidget()==splitterdown) {
                bool anyPlayerVisible=false;
                for (int index=0; index<splitterdown->count(); ++index)
                    anyPlayerVisible=anyPlayerVisible || !splitterdown->widget(index)->isHidden();
                const int height=qMax(2,splitterprincipal->height()-splitterprincipal->handleWidth());
                if (anyPlayerVisible)
                    splitterprincipal->setSizes({height/3,height-height/3});
                else
                    splitterprincipal->setSizes({height,0});
            }
        });
        // Sincronizar también después de ocultar el panel con su botón X.
        connect(vistasMenu, &QMenu::aboutToShow, this, [action, panel]() {
            action->setChecked(!panel->isHidden());
        });
    };

    // Los tres Player del menú corresponden a la fila inferior.
    addPanelAction(tr("Show Player1"), player2);
    addPanelAction(tr("Show Player2"), player3);
    addPanelAction(tr("Show Player3"), player4);
    addPanelAction(tr("Show File Browser"), m_fileExplore);
    addPanelAction(tr("Show Capture"), m_capture);
    addPanelAction(tr("Show Cast"), m_cast);
    addPanelAction(tr("Show MeteoClock"), m_meteoClock);
    addPanelAction(tr("Show Planner"), m_planner);
    addPanelAction(tr("Show Emission Log"),m_emissionLogFrame);

    const auto connectEmissionLog=[this](Player *player) {
        connect(player,&Player::emissionPlaybackStarted,m_emissionLog,
                [this,player](const QString &source,const QString &name,const QString &path) {
            m_emissionLog->trackStarted(player,source,name,path);
        });
        connect(player,&Player::emissionPlaybackFinished,m_emissionLog,
                [this,player](const QString &result,const QString &details) {
            m_emissionLog->trackFinished(player,result,details);
        });
    };
    for (Player *player : players)
        connectEmissionLog(player);
    connect(m_planner,&Planner::emissionPlaybackStarted,m_emissionLog,
            [this](QObject *engine,const QString &source,const QString &name,const QString &path) {
        m_emissionLog->trackStarted(engine,source,name,path);
    });
    connect(m_planner,&Planner::emissionPlaybackFinished,m_emissionLog,
            [this](QObject *engine,const QString &result,const QString &details) {
        m_emissionLog->trackFinished(engine,result,details);
    });


// cargar configuracion de los player*******************
    QString loadError;
    if (!Config::loadConfig("config.json", players, m_capture, &loadError, m_planner))
        QMessageBox::warning(this, tr("Configuration"), loadError);

    const auto saveConfiguration = [this]() {
        QString error;
        if (!Config::saveConfig("config.json", players, m_capture, &error, m_planner))
            QMessageBox::warning(this, tr("Configuration was not saved"), error);
    };
    for (Player *player : players)
        connect(player, &Player::configurationChanged, this, saveConfiguration);
    connect(m_planner, &Planner::configurationChanged, this, saveConfiguration);

    // Coalesce input-volume slider changes, then flush on normal exit.
    auto *captureSaveTimer = new QTimer(this);
    captureSaveTimer->setSingleShot(true);
    captureSaveTimer->setInterval(350);
    connect(m_capture, &Capture::configurationChanged, this, [captureSaveTimer]() {
        captureSaveTimer->start();
    });
    connect(captureSaveTimer, &QTimer::timeout, this, saveConfiguration);
    connect(qApp, &QCoreApplication::aboutToQuit, this, [captureSaveTimer, saveConfiguration]() {
        if (captureSaveTimer->isActive()) {
            captureSaveTimer->stop();
            saveConfiguration();
        }
    });

    restoreWindowInterface();

}

void MainWindow::restoreInterface()
{
    clearWorkspacePresetSelection();
    // Recuperar los mismos paneles y su estado, sin crear nuevas instancias.
    for (int i = 0; i < splittertop->count(); ++i)
        splittertop->widget(i)->show();
    for (int i = 0; i < splitterdown->count(); ++i)
        splitterdown->widget(i)->show();
    splittertop->show();
    splitterdown->show();
    if (m_emissionLogFrame)
        m_emissionLogFrame->hide();

    splittertop->setSizes({400, 400, 400, 400, 600, 0});
    splitterdown->setSizes({400, 400, 400});
    const int height = qMax(3, splitterprincipal->height() - splitterprincipal->handleWidth());
    splitterprincipal->setSizes({height / 3, height - height / 3});
}

void MainWindow::clearWorkspacePresetSelection()
{
    if (m_studioViewAction) m_studioViewAction->setChecked(false);
    if (m_plannerViewAction) m_plannerViewAction->setChecked(false);
}

void MainWindow::applyWorkspacePreset(bool plannerView)
{
    if (m_emissionLogFrame)
        m_emissionLogFrame->hide();
    m_studioViewAction->setChecked(!plannerView);
    m_plannerViewAction->setChecked(plannerView);

    m_fileExplore->show();
    m_meteoClock->show();
    m_cast->hide();
    m_capture->setVisible(!plannerView);
    m_planner->setVisible(plannerView);

    for (int index=0; index<splitterdown->count(); ++index)
        splitterdown->widget(index)->setVisible(!plannerView);

    splittertop->show();
    splitterdown->show();
    const int width=qMax(5,splittertop->width());
    const int height=qMax(2,splitterprincipal->height()-splitterprincipal->handleWidth());
    if (plannerView) {
        const int fileExploreWidth=m_fileExplore->minimumWidth();
        const int meteoClockWidth=m_meteoClock->minimumWidth();
        const int plannerWidth=qMax(1,width-fileExploreWidth-meteoClockWidth);
        splittertop->setSizes({fileExploreWidth,0,0,meteoClockWidth,plannerWidth,0});
        splitterprincipal->setSizes({height,0});
    } else {
        splittertop->setSizes({width/3,width/3,0,width/3,0,0});
        splitterdown->setSizes({1,1,1});
        splitterprincipal->setSizes({height/3,height-height/3});
    }
}

void MainWindow::saveWindowInterface() const
{
    const QString filename = QDir(QCoreApplication::applicationDirPath()).filePath("interface.ini");
    QSettings settings(filename, QSettings::IniFormat);
    settings.beginGroup("Interface");
    settings.setValue("splitterPrincipal", splitterprincipal->saveState());
    settings.setValue("splitterTop", splittertop->saveState());
    settings.setValue("splitterDown", splitterdown->saveState());
    settings.setValue("workspacePreset",m_plannerViewAction && m_plannerViewAction->isChecked()
                      ? QStringLiteral("planner")
                      : m_studioViewAction && m_studioViewAction->isChecked()
                            ? QStringLiteral("studio") : QString());

    const auto saveVisibility = [&settings,this](const QSplitter *splitter) {
        for (int index = 0; index < splitter->count(); ++index) {
            const QWidget *panel = splitter->widget(index);
            if (panel==m_emissionLogFrame)
                continue;
            if (!panel->objectName().isEmpty())
                settings.setValue("visible/" + panel->objectName(), !panel->isHidden());
        }
    };
    saveVisibility(splittertop);
    saveVisibility(splitterdown);
    settings.endGroup();
    settings.sync();
}

void MainWindow::restoreWindowInterface()
{
    const QString filename = QDir(QCoreApplication::applicationDirPath()).filePath("interface.ini");
    QSettings settings(filename, QSettings::IniFormat);
    settings.beginGroup("Interface");
    if (settings.contains("splitterPrincipal"))
        splitterprincipal->restoreState(settings.value("splitterPrincipal").toByteArray());
    if (settings.contains("splitterTop"))
        splittertop->restoreState(settings.value("splitterTop").toByteArray());
    if (settings.contains("splitterDown"))
        splitterdown->restoreState(settings.value("splitterDown").toByteArray());

    const auto restoreVisibility = [&settings,this](QSplitter *splitter) {
        for (int index = 0; index < splitter->count(); ++index) {
            QWidget *panel = splitter->widget(index);
            if (panel==m_emissionLogFrame)
                continue;
            const QString key = "visible/" + panel->objectName();
            if (!panel->objectName().isEmpty() && settings.contains(key))
                panel->setVisible(settings.value(key).toBool());
        }
    };
    restoreVisibility(splittertop);
    restoreVisibility(splitterdown);
    const QString workspacePreset=settings.value("workspacePreset").toString();
    if (workspacePreset==QLatin1String("planner"))
        m_plannerViewAction->setChecked(true);
    else if (workspacePreset==QLatin1String("studio"))
        m_studioViewAction->setChecked(true);
    settings.endGroup();
}

MainWindow::~MainWindow(){
   mediamanager->shutdown();//libera recursos

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_quitDialogOpen) {
        event->ignore();
        return;
    }
    QScopedValueRollback<bool> showing(m_quitDialogOpen, true);
    for (Planner *planner : findChildren<Planner*>()) {
        if (!planner->confirmSaveDocumentChanges()) {
            event->ignore();
            return;
        }
    }
    for (TabPlayer *player : findChildren<TabPlayer*>()) {
        if (!player->confirmSaveChanges()) {
            event->ignore();
            return;
        }
    }
    QuitDialog dialog(this);
    const int choice = dialog.exec();
    if (choice == QuitDialog::Cancel) {
        event->ignore();
        return;
    }
    if (choice == QuitDialog::SaveAndQuit)
        saveWindowInterface();
    // Finalize recordings before the main audio context is released by the destructor.
    for (auto *manager : findChildren<MediaManager*>()) manager->stopInput();
    event->accept();
}
