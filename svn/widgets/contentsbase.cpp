/* This file is part of Radit.
   Copyright 2022, Victor Algaba <victorengine@gmail.com> www.radit.org

   Radit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Iradit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with radit.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <QDrag>
#include <QDragEnterEvent>
#include <QDebug>
#include <QLayoutItem>
#include <QMimeData>
#include <QStyleOption>
#include <QPainter>
#include <QAction>
#include <QIcon>
#include <QUrl>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QMessageBox>
#include <QFutureWatcher>
#include <QEventLoop>
#include <QPointer>
#include <QTimer>
#include <QtConcurrent/QtConcurrentRun>
#include "widgets/LoadingDialog.h"

//#include <QtConcurrent/QtConcurrentMap>
//#include <QFuture>
//#include <QFutureWatcher>

#include "widgets/contentsbase.h"
#include "widgets/AudioItemFolderMaxi.h"
#include "widgets/AudioItemMeteoClockMaxi.h"
//#include "widgets/AudioItemFileMini.h"
#include "widgets/AudioItemFilemaxi.h"
#include "core/io.h"
//#include "bass.h"

ContentsBase::ContentsBase(QWidget *parent):QWidget(parent){
  // setContextMenuPolicy(Qt::DefaultContextMenu); // Habilitar la política de menú contextual predeterminada

    this->setObjectName("Contents"); // para el archivo qss
    this->setAcceptDrops(true);



    layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop); // pone los item en la parte alta
    layout->setSpacing(5); // espacios entre  item dentro del contenedor
    this->setLayout(layout);

    mediamanager = new MediaManager(this);


}



ContentsBase::~ContentsBase()
{
    if (m_dropCancel) m_dropCancel->store(true);
}

//***********************************


//entra el evento decide si elevento es valido
void ContentsBase::dragEnterEvent(QDragEnterEvent *event){
    // Verificar si el arrastre contiene URIs de archivos

    //  Drag interno (widgets)
       if (qobject_cast<QWidget*>(event->source())) {
           event->acceptProposedAction();
           return;
       }

       //  Archivos externos
       if (event->mimeData()->hasUrls()) {

           const QList<QUrl> urls = event->mimeData()->urls();

           for (const QUrl &url : urls) {
               if (url.isLocalFile()) {
                   event->acceptProposedAction();
                   return;
               }
           }
       }

       // Si no cumple ninguna condición
       event->ignore();
}



// miestras se arrastra
void ContentsBase::dragMoveEvent(QDragMoveEvent *event){

    event->acceptProposedAction();

}


// suelta evento
void ContentsBase::dropEvent(QDropEvent *event){
    if (m_importingFiles) { event->ignore(); return; }

    QWidget *owner = this;
    while (owner && !qobject_cast<TabPlayer*>(owner)) owner = owner->parentWidget();
    auto *tabs = qobject_cast<TabPlayer*>(owner);
    if (tabs) {
        if (tabs->loadDroppedPlayer(event->mimeData())) {
            event->acceptProposedAction();
            return;
        }
    }

    if (!event->mimeData()->hasUrls() || m_importingFiles) {
        event->ignore();
        return;
    }
    QStringList paths;
    for (const QUrl &url : event->mimeData()->urls())
        if (url.isLocalFile()) paths.append(url.toLocalFile());
    if (paths.isEmpty()) {
        event->ignore();
        return;
    }
    // Return from the native drop before starting work, so its drag image disappears.
    m_importingFiles = true;
    event->acceptProposedAction();
    QTimer::singleShot(0, this, [this, paths]() {
        importDroppedFiles(paths);
    });
}

void ContentsBase::importDroppedFiles(const QStringList &paths)
{
    m_dropPaths = paths;
    m_dropIndex = 0;
    m_dropCancel = std::make_shared<std::atomic_bool>(false);
    // Owned by the destination: deleting a tab closes its progress window too.
    m_dropLoading = new LoadingDialog(this, tr("Loading files..."), Qt::NonModal);
    m_dropLoading->setCancelHandler([this]() { cancelDroppedFiles(); });
    m_dropLoading->setProgress(0, paths.size());
    importNextDroppedFile();
}

void ContentsBase::importNextDroppedFile()
{
    if (!m_importingFiles || !m_dropCancel || m_dropCancel->load()) return;
    if (m_dropIndex >= m_dropPaths.size()) {
        cancelDroppedFiles();
        return;
    }
    const QString path = m_dropPaths.at(m_dropIndex);
    const QFileInfo info(path);
    if (info.suffix().compare("zip", Qt::CaseInsensitive)==0) {
        auto *item=new AudioItemMeteoClockMaxi(this);
        item->setVoicePackPath(path);createItem(item);finishDroppedFile();return;
    }
    if (info.isDir()) {
        auto *item = new AudioItemFolderMaxi(this);
        item->setFolderPath(path);
        createItem(item);
        finishDroppedFile();
        return;
    }
    if (m_dropLoading)
        m_dropLoading->setDetail(tr("Reading: %1 (%2/%3)")
            .arg(info.fileName()).arg(m_dropIndex + 1).arg(m_dropPaths.size()));
    if (info.suffix().compare("list", Qt::CaseInsensitive) == 0) {
        QWidget *owner = this;
        while (owner && !qobject_cast<TabPlayer*>(owner)) owner = owner->parentWidget();
        auto *tabs = qobject_cast<TabPlayer*>(owner);
        if (m_dropLoading) m_dropLoading->hide();
        QPointer<ContentsBase> target(this);
        Io io;
        QString error;
        const bool loaded = tabs ? tabs->loadListFile(path, &error)
                                 : io.LoadListPlayer(this, path, &error);
        if (!target) return;
        if (!loaded) QMessageBox::warning(this, tr("Load list"), error);
        if (m_dropLoading) m_dropLoading->show();
        finishDroppedFile();
        return;
    }

    const auto cancel = m_dropCancel;
    auto *watcher = new QFutureWatcher<double>(this);
    // The connection belongs to the destination. If it is removed while a network
    // read is pending, no completion callback can touch deleted widgets.
    connect(watcher, &QFutureWatcher<double>::finished, this, [this, watcher, path, cancel]() {
        if (cancel != m_dropCancel || cancel->load()) {
            watcher->deleteLater();
            return;
        }
        const double duration = watcher->result();
        watcher->deleteLater();
        if (duration > 0) {
            auto *item = new AudioItemFileMaxi(this);
            item->setFilePath(path);
            item->setSecond(duration);
            item->setNameFile(QFileInfo(path).completeBaseName());
            item->setTiempoFile(duration);
            item->setToolTip(path);
            createItem(item);
        } else {
            qWarning() << "Invalid or unavailable audio file:" << path;
        }
        finishDroppedFile();
    });
    watcher->setFuture(QtConcurrent::run([path, cancel]() {
        if (cancel->load()) return -1.0;
        const double duration = MediaManager::readFileDuration(path);
        return cancel->load() ? -1.0 : duration;
    }));
}

void ContentsBase::finishDroppedFile()
{
    ++m_dropIndex;
    if (m_dropLoading) m_dropLoading->setProgress(m_dropIndex, m_dropPaths.size());
    // Yield after every item: playback controls, painting and user input keep working.
    const auto cancel = m_dropCancel;
    QTimer::singleShot(0, this, [this, cancel]() {
        if (cancel == m_dropCancel && !cancel->load()) importNextDroppedFile();
    });
}

void ContentsBase::cancelDroppedFiles()
{
    if (m_dropCancel) m_dropCancel->store(true);
    m_importingFiles = false;
    m_dropPaths.clear();
    if (m_dropLoading) {
        m_dropLoading->hide();
        m_dropLoading->deleteLater();
        m_dropLoading.clear();
    }
}

AudioItemMaxi* ContentsBase::createItem(AudioItemMaxi* item){

    layout->addWidget(item);

    connect(item, &AudioItemMaxi::requestDelete, //viene de pulsar boton de borrado de audioitemMaxi
            this, [this](AudioItemMaxi* item){

      //  if(item->isPlaying())
        //    return;

        QString nombre = item->nameFile();

          QMessageBox::StandardButton reply = QMessageBox::question(
              this,
              "Borrar Item",
              QString("¿Seguro que quieres borrar \"%1\"?").arg(nombre),
              QMessageBox::Yes | QMessageBox::No
          );

          if (reply == QMessageBox::Yes) {
              deleteItem(item);
          }

    });

    //borrado para el purge
   connect(item, &AudioItemMaxi::requestAutoDelete,
           this, [this](AudioItemMaxi* item){
             //if(item->isPlayNext()) // si esta en playnext no borra el item
                // return;

             deleteItem(item);
   });

    //  NUEVO: conectar play grande del item
      connect(item, &AudioItemMaxi::requestPlay,
              this, [this](AudioItemMaxi* item) {

          if (Player* player = findPlayer()) {
              player->playItem(item);
          }

      });

    return item;
}


void ContentsBase::deleteItem(AudioItemMaxi* item){

    if (!item) return;
    clipboard.lista.removeAll(item);


        //  Buscar el player REAL del item
        Player* itemPlayer = nullptr;
        QWidget* w = item;

        while (w) {
            if (Player* p = qobject_cast<Player*>(w)) {
                itemPlayer = p;
                break;
            }
            w = w->parentWidget();
        }

        if (itemPlayer && item->isPlaying()) {
            itemPlayer->stopMain();
        }

        if (item->parentWidget() && item->parentWidget()->layout()) {
            item->parentWidget()->layout()->removeWidget(item);
        }

        item->deleteLater();
}

//*****************************************
Player* ContentsBase::findPlayer() const{

    QWidget* w = const_cast<ContentsBase*>(this);

    while (w) {
        if (Player* player = qobject_cast<Player*>(w)) {
            return player;
        }
        w = w->parentWidget();
    }

    return nullptr;
}

AudioItemMaxi* ContentsBase::findNextPlayItem(AudioItemMaxi* current)
{
    if (!current || !layout)
            return nullptr;

        int index = layout->indexOf(current);

        //  1. Buscar hacia abajo
        for (int i = index; i < layout->count(); ++i) {
            QWidget* w = layout->itemAt(i)->widget();
            if (auto *item = qobject_cast<AudioItemMaxi*>(w)) {
                if (item->isPlayNext())
                    return item;
            }
        }

        //  2. Si no encuentra, buscar desde arriba
        for (int i = 0; i < index; ++i) {
            QWidget* w = layout->itemAt(i)->widget();
            if (auto *item = qobject_cast<AudioItemMaxi*>(w)) {
                if (item->isPlayNext())
                    return item;
            }
        }

        return nullptr;
}

//***************** boorrar antes de cargar la lista nueva
void ContentsBase::clearItems()
{
           while (layout->count() > 0) {

            QLayoutItem *layoutItem = layout->takeAt(0);

            if (!layoutItem)
                continue;

            QWidget *widget = layoutItem->widget();

            if (widget) {
                widget->setParent(nullptr);
                delete widget;
            }

            delete layoutItem;
        }


}


void ContentsBase::setTabName(const QString &filename)
{
    // 1. Extraemos el nombre limpio del archivo y manejamos si viene vacío
    QFileInfo fileInfo(filename);
    QString tabName = fileInfo.completeBaseName();

    if (tabName.isEmpty()) {
        tabName = "noname";
    }

    // 2. Subimos por la jerarquía de widgets buscando el TabPlayer
    QWidget *parentWidget = this->parentWidget();
    TabPlayer *tabPlayer = nullptr;

    while (parentWidget != nullptr) {
        tabPlayer = qobject_cast<TabPlayer*>(parentWidget);
        if (tabPlayer) {
            break; // ¡Encontrado!
        }
        parentWidget = parentWidget->parentWidget();
    }

    // 3. Si lo encontramos, localizamos la pestaña que contiene a este ContentsPlayer
    if (tabPlayer) {
        int tabIndex = -1;

        for (int i = 0; i < tabPlayer->count(); ++i) {
            // Comprobamos cuál de las pestañas es el "antepasado" de este widget
            if (tabPlayer->widget(i)->isAncestorOf(this)) {
                tabIndex = i;
                break;
            }
        }

        // 4. Cambiamos el texto y el ToolTip de la pestaña asignada
        if (tabIndex != -1) {
            tabPlayer->setTabText(tabIndex, tabName);
            tabPlayer->setTabToolTip(tabIndex, tabName); // Muestra la ruta completa como ToolTip
        }
    }
}



