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
#include <QFileDialog>
#include <QScopedValueRollback>
#include <QStandardPaths>
#include <QDir>
#include <QKeySequence>
#include "widgets/QuitDialog.h"
#include "core/io.h"


//#include "widgets/TabPlayer.h"
//#include "widgets/container.h"
#include "widgets/TabAuto.h"
#include "widgets/FormAbout.h"
#include "core/config.h"
#include "widgets/fileexplore.h"
#include "widgets/Capture.h"
#include "widgets/Cast.h"
#include "widgets/Player.h"


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent){



    mediamanager = new MediaManager(this);
    mediamanager->initialize(); //iniciamos audio



    // Crear la barra de menú
    QMenuBar *menubar =new QMenuBar;
    menubar->setObjectName("menubar"); //para qss


       // Añadir los menús principales (sin submenús por ahora)
       QMenu *fileMenu = menubar->addMenu(tr("&File"));
       fileMenu->setObjectName("FileMenu");
       fileMenu->setSeparatorsCollapsible(false);
       fileMenu->addSeparator();
       QAction *quitAction = fileMenu->addAction(tr("Quit"));
       quitAction->setObjectName("QuitAction");
       quitAction->setShortcut(QKeySequence::Quit);
       quitAction->setMenuRole(QAction::QuitRole);
       connect(quitAction, &QAction::triggered, this, &QWidget::close);
       menubar->addMenu(tr("&Edit"));
       QMenu *vistasMenu = menubar->addMenu(tr("&View"));
       menubar->addMenu(tr("&Tools"));


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
    splittertop->addWidget(m_fileExplore);
    splittertop->addWidget(m_capture);
    splittertop->addWidget(m_cast);
    splittertop->addWidget(new TabAuto);

    splitterdown->addWidget(player2);
    splitterdown->addWidget(player3);
    splitterdown->addWidget(player4);


    // FileExplore, Capture y Cast comparten el ancho inicial.
    splittertop->setStretchFactor(0, 2);
    splittertop->setStretchFactor(1, 2);
    splittertop->setStretchFactor(2, 2);
    splittertop->setStretchFactor(3, 3);
    QList<int> sizesTop;
    sizesTop << 400 << 400 << 400 << 600;
    splittertop->setSizes(sizesTop);

    QAction *resetInterface = vistasMenu->addAction(tr("Reset User Interface"));
    connect(resetInterface, &QAction::triggered, this, &MainWindow::restoreInterface);
    vistasMenu->addSeparator();

    const auto addPanelAction = [this, vistasMenu](const QString &text, QWidget *panel) {
        QAction *action = vistasMenu->addAction(text);
        action->setCheckable(true);
        action->setChecked(!panel->isHidden());
        connect(action, &QAction::triggered, this, [panel](bool visible) {
            panel->setVisible(visible);
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


// cargar configuracion de los player*******************
    QString loadError;
    if (!Config::loadConfig("config.json", players, m_capture, &loadError))
        QMessageBox::warning(this, tr("Configuration"), loadError);

    const auto saveConfiguration = [this]() {
        QString error;
        if (!Config::saveConfig("config.json", players, m_capture, &error))
            QMessageBox::warning(this, tr("Configuration was not saved"), error);
    };
    for (Player *player : players)
        connect(player, &Player::configurationChanged, this, saveConfiguration);

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


}

void MainWindow::restoreInterface()
{
    // Recuperar los mismos paneles y su estado, sin crear nuevas instancias.
    for (int i = 0; i < splittertop->count(); ++i)
        splittertop->widget(i)->show();
    for (int i = 0; i < splitterdown->count(); ++i)
        splitterdown->widget(i)->show();
    splittertop->show();
    splitterdown->show();

    splittertop->setSizes({400, 400, 400, 600});
    splitterdown->setSizes({400, 400, 400});
    const int height = qMax(3, splitterprincipal->height() - splitterprincipal->handleWidth());
    splitterprincipal->setSizes({height / 3, height - height / 3});
}

MainWindow::~MainWindow(){
   mediamanager->shutdown();//libera recursos

}

bool MainWindow::saveBeforeQuit()
{
    Io io;
    for (Player *player : players) {
        if (!isAncestorOf(player)) continue;
        auto *tabs = player->findChild<TabPlayer*>();
        if (!tabs) continue;
        QString filename = tabs->playerFileName();
        if (filename.isEmpty()) {
            const QString suggestion = QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
                .filePath(player->objectName() + ".player");
            filename = QFileDialog::getSaveFileName(this, tr("Save player: %1").arg(player->title()),
                                                   suggestion, tr("Radit Player (*.player)"));
            if (filename.isEmpty()) return false;
            if (!filename.endsWith(".player", Qt::CaseInsensitive)) filename += ".player";
        }
        QString error;
        if (!io.SavePlayer(tabs, filename, &error)) {
            QMessageBox::warning(this, tr("Player was not saved"), error);
            return false;
        }
    }
    QString error;
    if (!Config::saveConfig("config.json", players, m_capture, &error)) {
        QMessageBox::warning(this, tr("Configuration was not saved"), error);
        return false;
    }
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_quitDialogOpen) {
        event->ignore();
        return;
    }
    QScopedValueRollback<bool> showing(m_quitDialogOpen, true);
    QuitDialog dialog(this);
    const int choice = dialog.exec();
    if (choice == QuitDialog::Cancel ||
        (choice == QuitDialog::SaveAndQuit && !saveBeforeQuit())) {
        event->ignore();
        return;
    }
    // Finalize recordings before the main audio context is released by the destructor.
    for (auto *manager : findChildren<MediaManager*>()) manager->stopInput();
    event->accept();
}
