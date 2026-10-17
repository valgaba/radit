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
#include <QWidgetAction>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QPointer>
#include <QVector>
#include <QHash>
#include <QSet>
#include <QDialog>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrentRun>
#include <cmath>


#include "widgets/ContentsPlayer.h"
#include "widgets/button.h"
#include "widgets/frame.h"
#include "widgets/label.h"
#include "widgets/AudioItem.h"
#include "widgets/AudioItemFileMaxi.h"
#include "widgets/AudioItemFilePlanner.h"
#include "widgets/AudioItemFolderPlanner.h"
#include "widgets/AudioItemListMaxi.h"
#include "widgets/AudioItemListPlanner.h"
#include "widgets/AudioItemMeteoClockPlanner.h"
#include "widgets/AudioItemNetPlanner.h"
#include "core/io.h"
#include "core/MediaManager.h"

namespace {
AudioItemMaxi *makeMaxiItem(AudioItemMaxi *source, QWidget *parent)
{
    AudioItemMaxi *item=nullptr;
    if (auto *list=qobject_cast<AudioItemListPlanner*>(source))
        item=list->AudioItemListMaxi::copy(parent);
    else if (auto *file=qobject_cast<AudioItemFilePlanner*>(source))
        item=file->AudioItemFileMaxi::copy(parent);
    else if (auto *folder=qobject_cast<AudioItemFolderPlanner*>(source))
        item=folder->AudioItemFolderMaxi::copy(parent);
    else if (auto *meteo=qobject_cast<AudioItemMeteoClockPlanner*>(source))
        item=meteo->AudioItemMeteoClockMaxi::copy(parent);
    else if (auto *net=qobject_cast<AudioItemNetPlanner*>(source))
        item=net->AudioItemNetMaxi::copy(parent);

    if (item) {
        item->setIsLoop(source->isLoop());
        item->setIsPurge(source->isPurge());
        item->setIsPlayNext(source->isPlayNext());
        item->setEnabledForPlayback(source->isEnabledForPlayback());
        item->setColor(source->color());
        item->setToolTip(source->toolTip());
    }
    return item;
}
}


ContentsPlayer::ContentsPlayer(QWidget *parent):ContentsBase(parent){

    setContextMenuPolicy(Qt::DefaultContextMenu); // Habilitar la política de menú contextual predeterminada
    this->setObjectName("Contents"); // para el archivo qss
    this->isCut=false;


 contentsMenu = new ContentsMenu(this);

}



ContentsPlayer::~ContentsPlayer(){}

void ContentsPlayer::refreshAudioDurations()
{
    if (m_refreshingAudioDurations)
        return;

    const QList<AudioItemFileMaxi*> fileItems=findChildren<AudioItemFileMaxi*>();
    QVector<QPointer<AudioItemFileMaxi>> targets;
    QStringList paths;
    QSet<QString> uniquePaths;
    for (AudioItemFileMaxi *item : fileItems) {
        if (!item || item->filePath().isEmpty())
            continue;
        targets.append(item);
        if (!uniquePaths.contains(item->filePath())) {
            uniquePaths.insert(item->filePath());
            paths.append(item->filePath());
        }
    }
    if (targets.isEmpty()) {
        QMessageBox::information(this,tr("Update audio times"),
                                 tr("There are no audio files to update."));
        return;
    }

    m_refreshingAudioDurations=true;
    auto *progress=new QProgressDialog(tr("Reading audio file durations..."),QString(),0,0,this);
    progress->setWindowTitle(tr("Update audio times"));
    progress->setWindowModality(Qt::WindowModal);
    progress->setCancelButton(nullptr);
    progress->setMinimumDuration(0);
    progress->show();

    auto *watcher=new QFutureWatcher<QVector<QPair<QString,double>>>(this);
    connect(watcher,&QFutureWatcher<QVector<QPair<QString,double>>>::finished,
            this,[this,watcher,progress,targets]() {
        const auto results=watcher->result();
        QHash<QString,double> durations;
        for (const auto &result : results)
            durations.insert(result.first,result.second);

        int updated=0;
        int refreshed=0;
        int failed=0;
        for (const QPointer<AudioItemFileMaxi> &item : targets) {
            if (!item)
                continue;
            const auto found=durations.constFind(item->filePath());
            if (found==durations.cend() || found.value()<=0.0) {
                ++failed;
                continue;
            }
            const double duration=found.value();
            ++refreshed;
            if (std::abs(item->second()-duration)>0.01) {
                item->setSecond(duration);
                item->setTiempoFile(duration);
                ++updated;
            }
        }

        m_refreshingAudioDurations=false;
        progress->close();
        progress->deleteLater();
        watcher->deleteLater();
        if (updated>0)
            emit contentsModified();
        emit audioDurationsRefreshed(updated,failed);
        QMessageBox::information(this,tr("Update audio times"),
            tr("Updated %1 audio item(s). Could not read %2.").arg(refreshed).arg(failed));
    });

    watcher->setFuture(QtConcurrent::run([paths]() {
        QVector<QPair<QString,double>> results;
        results.reserve(paths.size());
        for (const QString &path : paths)
            results.append({path,MediaManager::readFileDuration(path)});
        return results;
    }));
}

AudioItemMaxi *ContentsPlayer::createItem(AudioItemMaxi *item)
{
    AudioItemMaxi *created=ContentsBase::createItem(item);
    if (created) {
        connect(created,&AudioItemMaxi::propertiesOpened,this,[this]() {
            emit contentsModified();
        });
        connect(created,&AudioItemMaxi::itemStateChanged,this,[this]() {
            emit contentsModified();
        });
        emit contentsModified();
    }
    return created;
}

void ContentsPlayer::deleteItem(AudioItemMaxi *item)
{
    if (!item)
        return;
    ContentsBase::deleteItem(item);
    emit contentsModified();
}




void ContentsPlayer::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-audioitems")) {

           QByteArray data = event->mimeData()->data("application/x-audioitems");
           QDataStream stream(&data, QIODevice::ReadOnly);

           QList<AudioItem*> draggedItems;
           QList<AudioItemMaxi*> itemsToDeleteFromSource;

           while (!stream.atEnd()) {
               quintptr ptr;
               stream >> ptr;

               AudioItem *itemBase = reinterpret_cast<AudioItem*>(ptr);
               if (!itemBase)
                   continue;

               draggedItems.append(itemBase);
           }

           // Buscar item destino bajo el cursor
  /* #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
           QPoint dropPos = event->position().toPoint();
   #else
           QPoint dropPos = event->pos();
   #endif*/
           QPoint dropPos = event->position().toPoint();
           QWidget *targetWidget = childAt(dropPos);

           while (targetWidget && !qobject_cast<AudioItemMaxi*>(targetWidget)) {
               targetWidget = targetWidget->parentWidget();
           }

           int insertIndex = -1;
           if (targetWidget) {
               insertIndex = layout->indexOf(targetWidget);
           }

           // Si no cae sobre un item, añadir al final
           if (insertIndex < 0) {
               insertIndex = layout->count();
           }

           for (AudioItem *itemBase : std::as_const(draggedItems)) {

               ContentsPlayer* sourceContents = nullptr;

               QWidget *p = itemBase->parentWidget();
               while (p) {
                   sourceContents = qobject_cast<ContentsPlayer*>(p);
                   if (sourceContents)
                       break;
                   p = p->parentWidget();
               }

               if (sourceContents == this) {
                   // MISMO PLAYER -> mover
                   if (itemBase->parentWidget() && itemBase->parentWidget()->layout()) {
                       itemBase->parentWidget()->layout()->removeWidget(itemBase);
                   }

                   layout->insertWidget(insertIndex, itemBase);
                   itemBase->setParent(this);
                   itemBase->setIsSelect(false);

                   insertIndex++;
               }
               else {
                   // OTRO PLAYER -> copiar
                   AudioItemMaxi* itemMaxi = qobject_cast<AudioItemMaxi*>(itemBase);
                   if (!itemMaxi)
                       continue;

                   AudioItemMaxi* newItem = makeMaxiItem(itemMaxi,this);
                   if (!newItem)
                       newItem = itemMaxi->copy(this);

                   // createItem(newItem) hace addWidget al final,
                   // así que aquí NO lo usamos directamente
                   layout->insertWidget(insertIndex, newItem);

                   // reconectar señales manualmente
                   connect(newItem, &AudioItemMaxi::requestDelete,
                           this, [this](AudioItemMaxi* item){

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
                   connect(newItem, &AudioItemMaxi::requestAutoDelete,
                           this, [this](AudioItemMaxi* item){
                             deleteItem(item);
                   });


                   connect(newItem, &AudioItemMaxi::requestPlay,
                           this, [this](AudioItemMaxi* item) {
                       if (Player* player = findPlayer()) {
                           player->playItem(item);
                       }
                   });
                   connect(newItem,&AudioItemMaxi::propertiesOpened,this,[this]() {
                       emit contentsModified();
                   });
                   connect(newItem,&AudioItemMaxi::itemStateChanged,this,[this]() {
                       emit contentsModified();
                   });

                   newItem->setIsSelect(false);

                   itemsToDeleteFromSource.append(itemMaxi);
                   insertIndex++;
               }
           }

           // Borrar originales si vienen de otro player (cut/move)
           for (AudioItemMaxi* oldItem : std::as_const(itemsToDeleteFromSource)) {
               if (!oldItem)
                   continue;

               if (oldItem->parentWidget() && oldItem->parentWidget()->layout()) {
                   oldItem->parentWidget()->layout()->removeWidget(oldItem);
               }

              // oldItem->deleteLater();
                deleteItem(oldItem);   //
           }

           if (!draggedItems.isEmpty())
               emit contentsModified();

           event->acceptProposedAction();
           return;
       }

       // Drag externo (archivos) -> lo maneja ContentsBase
       ContentsBase::dropEvent(event);
}



void ContentsPlayer::contextMenuEvent(QContextMenuEvent *event){
    mousePos = event->pos();

        QWidget *widget = childAt(mousePos);

        // Si no hay widget, ocultar acciones específicas
        if (!widget) {
            const bool showListOptions=supportsListOptionsInContextMenu();
            const bool hasAudioFiles=!findChildren<AudioItemFileMaxi*>().isEmpty();
            contentsMenu->setColorVisible(false);
            contentsMenu->setSelectAllVisible(false);
            contentsMenu->setUnselectAllVisible(false);
            contentsMenu->setCopyVisible(false);
            contentsMenu->setCutVisible(false);
            contentsMenu->setDeleteVisible(false);
            contentsMenu->setPropertiesVisible(false);
            contentsMenu->setSumDurationsVisible(false);

            contentsMenu->setnextVisible(false);
            contentsMenu->setpurgeVisible(false);
            contentsMenu->setloopVisible(false);
            contentsMenu->setselectVisible(false);

            contentsMenu->setPasteVisible(!this->clipboard.lista.isEmpty());

            contentsMenu->setLoadVisible(showListOptions);
            contentsMenu->setSaveVisible(showListOptions);
            contentsMenu->setSaveAsVisible(showListOptions);
            contentsMenu->setRefreshDurationsVisible(showListOptions && hasAudioFiles);

            contentsMenu->exec(mapToGlobal(mousePos));
            return;
        }

        // Mostrar acciones por defecto
        const bool showPlaybackOptions=supportsPlaybackOptionsInContextMenu();
        contentsMenu->setColorVisible(showPlaybackOptions);
        contentsMenu->setSelectAllVisible(true);
        contentsMenu->setUnselectAllVisible(true);
        contentsMenu->setCopyVisible(true);
        contentsMenu->setCutVisible(true);
        contentsMenu->setDeleteVisible(true);
        contentsMenu->setPropertiesVisible(showPlaybackOptions);

        contentsMenu->setnextVisible(showPlaybackOptions);
        contentsMenu->setpurgeVisible(showPlaybackOptions);
        contentsMenu->setloopVisible(showPlaybackOptions);
        contentsMenu->setselectVisible(true);

        bool hasSelection = false;
        int selectedCount = 0;
        bool canPurge = false;
        for (auto *item : findChildren<AudioItemMaxi*>()) {
            if (!item->isSelect()) continue;
            hasSelection = true;
            ++selectedCount;
            canPurge |= !item->isLiveStream();
        }
        contentsMenu->setSumDurationsVisible(selectedCount>0);
        if (!hasSelection) {
            QWidget *target = widget;
            while (target && !qobject_cast<AudioItemMaxi*>(target))
                target = target->parentWidget();
            if (auto *item = qobject_cast<AudioItemMaxi*>(target))
                canPurge = !item->isLiveStream();
        }
        contentsMenu->setpurgeVisible(showPlaybackOptions && canPurge);

        const bool showListOptions=supportsListOptionsInContextMenu();
        const bool hasAudioFiles=!findChildren<AudioItemFileMaxi*>().isEmpty();
        contentsMenu->setLoadVisible(showListOptions);
        contentsMenu->setSaveVisible(showListOptions);
        contentsMenu->setSaveAsVisible(showListOptions);
        contentsMenu->setRefreshDurationsVisible(showListOptions && hasAudioFiles);



        // Mostrar u ocultar Paste según clipboard
        contentsMenu->setPasteVisible(!this->clipboard.lista.isEmpty());

        // Mostrar menú nuevo
        contentsMenu->exec(mapToGlobal(mousePos));

}

//************************ menus ************
void ContentsPlayer::nextAllItems(){

    QList<AudioItemMaxi*> selectedItems;

        // Recoger seleccionados
        QList<AudioItemMaxi*> items = findChildren<AudioItemMaxi*>();
        for (AudioItemMaxi* item : std::as_const(items)) {
            if (item->isSelect()) {
                selectedItems.append(item);
            }
        }

        // Caso 1: hay selección
        if (!selectedItems.isEmpty()) {
            for (AudioItemMaxi* item : std::as_const(selectedItems)) {
                item->setIsPlayNext(!item->isPlayNext());
                // quitar selección
                 item->setIsSelect(false);
            }
            emit contentsModified();
            return;
        }

        // Caso 2: no hay selección → item bajo el cursor
        QWidget *widget = childAt(mousePos);

        while (widget && !qobject_cast<AudioItemMaxi*>(widget)) {
            widget = widget->parentWidget();
        }

        if (auto *item = qobject_cast<AudioItemMaxi*>(widget)) {
            item->setIsPlayNext(!item->isPlayNext());
            emit contentsModified();
        }

}

void ContentsPlayer::purgeAllItems(){

    QList<AudioItemMaxi*> selectedItems;

        // Recoger seleccionados
        QList<AudioItemMaxi*> items = findChildren<AudioItemMaxi*>();
        for (AudioItemMaxi* item : std::as_const(items)) {
            if (item->isSelect()) {
                selectedItems.append(item);
            }
        }

        // Caso 1: hay selección
        if (!selectedItems.isEmpty()) {
            for (AudioItemMaxi* item : std::as_const(selectedItems)) {
                item->setIsPurge(!item->isPurge());
                // quitar selección
                 item->setIsSelect(false);
            }
            emit contentsModified();
            return;
        }

        // Caso 2: no hay selección → item bajo el cursor
        QWidget *widget = childAt(mousePos);

        while (widget && !qobject_cast<AudioItemMaxi*>(widget)) {
            widget = widget->parentWidget();
        }

        if (auto *item = qobject_cast<AudioItemMaxi*>(widget)) {
            item->setIsPurge(!item->isPurge());
            emit contentsModified();
        }

}


void ContentsPlayer::loopAllItems(){

    QList<AudioItemMaxi*> selectedItems;

        // Recoger seleccionados
        QList<AudioItemMaxi*> items = findChildren<AudioItemMaxi*>();
        for (AudioItemMaxi* item : std::as_const(items)) {
            if (item->isSelect()) {
                selectedItems.append(item);
            }
        }

        // Caso 1: hay selección
        if (!selectedItems.isEmpty()) {
            for (AudioItemMaxi* item : std::as_const(selectedItems)) {
                item->setIsLoop(!item->isLoop());
                // quitar selección
                 item->setIsSelect(false);
            }
            emit contentsModified();
            return;
        }

        // Caso 2: no hay selección → item bajo el cursor
        QWidget *widget = childAt(mousePos);

        while (widget && !qobject_cast<AudioItemMaxi*>(widget)) {
            widget = widget->parentWidget();
        }

        if (auto *item = qobject_cast<AudioItemMaxi*>(widget)) {
             item->setIsLoop(!item->isLoop());
             emit contentsModified();
        }

}

void ContentsPlayer::selectItems(){

    // Caso 2: no hay selección → item bajo el cursor
    QWidget *widget = childAt(mousePos);

    while (widget && !qobject_cast<AudioItemMaxi*>(widget)) {
        widget = widget->parentWidget();
    }

    if (auto *item = qobject_cast<AudioItemMaxi*>(widget)) {
         item->setIsSelect(!item->isSelect());
    }


}


void ContentsPlayer::selectAllItems()
{

    QList<AudioItemMaxi*> items = this->findChildren<AudioItemMaxi*>();

    for (AudioItemMaxi* item : std::as_const(items)) {
        item->setIsSelect(true);
    }
}

void ContentsPlayer::unSelectAllItems(){

    QList<AudioItemMaxi*> items = this->findChildren<AudioItemMaxi*>();

        for (AudioItemMaxi* item : std::as_const(items)) {
            item->setIsSelect(false);
        }


}

void ContentsPlayer::sumSelectedDurations()
{
    double totalSeconds=0.0;
    int selectedCount=0;
    int unknownCount=0;
    for (AudioItemMaxi *item : findChildren<AudioItemMaxi*>()) {
        if (!item || !item->isSelect())
            continue;
        ++selectedCount;
        const auto *fileItem=qobject_cast<AudioItemFileMaxi*>(item);
        if (!fileItem) {
            ++unknownCount;
            continue;
        }
        const double duration=fileItem->second();
        if (!std::isfinite(duration) || duration<=0.0) {
            ++unknownCount;
            continue;
        }
        totalSeconds+=duration;
    }
    if (selectedCount==0)
        return;

    const qint64 seconds=qRound64(totalSeconds);
    const QString formatted=QStringLiteral("%1:%2:%3")
        .arg(seconds/3600,2,10,QLatin1Char('0'))
        .arg((seconds%3600)/60,2,10,QLatin1Char('0'))
        .arg(seconds%60,2,10,QLatin1Char('0'));

    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("SelectedDurationDialog"));
    dialog.setWindowFlags(Qt::Dialog|Qt::FramelessWindowHint);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setModal(true);
    dialog.setWindowTitle(tr("Selected duration"));
    dialog.setFixedWidth(360);
    dialog.setStyleSheet(QStringLiteral(
        "QDialog#SelectedDurationDialog { background:#181e2c; border:1px solid #77859a; }"
        "QDialog#SelectedDurationDialog QLabel { color:#8a8d96; }"
        "QDialog#SelectedDurationDialog Frame#framebarra { background:#4e4d7a; border:none; }"
        "QDialog#SelectedDurationDialog QPushButton { color:#8a8d96; background:#282f40;"
        " border:1px solid #626b7e; padding:2px 10px; }"
        "QDialog#SelectedDurationDialog QPushButton:hover { border-color:#80A4AE; background:#30384b; }"));

    auto *root=new QVBoxLayout(&dialog);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);
    auto *titleBar=new Frame(&dialog);
    titleBar->setObjectName(QStringLiteral("framebarra"));
    titleBar->setFixedHeight(27);
    auto *titleLayout=new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(8,0,4,0);
    auto *title=new Label(titleBar);
    title->setText(dialog.windowTitle());
    titleLayout->addWidget(title);
    titleLayout->addStretch(1);
    auto *close=new Button(titleBar);
    close->setStyleSheet(QStringLiteral("QPushButton { border:none; background:transparent; padding:0px; }"));
    close->setFixedSize(18,18);
    close->SetIcon(QStringLiteral("Close-hover.svg"));
    close->setIconSize(QSize(15,15));
    titleLayout->addWidget(close);
    QObject::connect(close,&QPushButton::clicked,&dialog,&QDialog::accept);
    root->addWidget(titleBar);

    auto *body=new QWidget(&dialog);
    auto *bodyLayout=new QVBoxLayout(body);
    bodyLayout->setContentsMargins(18,16,18,14);
    bodyLayout->setSpacing(8);
    auto *duration=new Label(body);
    duration->setText(formatted);
    duration->setAlignment(Qt::AlignCenter);
    QFont durationFont=duration->font();
    durationFont.setPointSize(durationFont.pointSize()+8);
    durationFont.setBold(true);
    duration->setFont(durationFont);
    duration->setMinimumHeight(46);
    duration->setStyleSheet(QStringLiteral("color:#80A4AE;"));
    bodyLayout->addWidget(duration);
    auto *details=new Label(body);
    details->setText(tr("Selected items: %1\nItems without a known duration: %2")
                         .arg(selectedCount).arg(unknownCount));
    details->setAlignment(Qt::AlignCenter);
    details->setWordWrap(true);
    bodyLayout->addWidget(details);
    root->addWidget(body);

    auto *buttonRow=new QHBoxLayout;
    buttonRow->setContentsMargins(12,0,12,10);
    buttonRow->addStretch(1);
    auto *ok=new Button(&dialog);
    ok->setText(tr("OK"));
    ok->setFixedSize(72,25);
    buttonRow->addWidget(ok);
    root->addLayout(buttonRow);
    QObject::connect(ok,&QPushButton::clicked,&dialog,&QDialog::accept);
    dialog.exec();
}

void ContentsPlayer::deleteSelected(){

    QList<AudioItemMaxi*> selectedItems;

    // Recoger seleccionados
    QList<AudioItemMaxi*> items = findChildren<AudioItemMaxi*>();
    for (AudioItemMaxi* item : std::as_const(items)) {

       /*  if (item->isSelect() && !item->isPlaying()) {
            selectedItems.append(item);
        }*/

         if (item->isSelect()) {
            selectedItems.append(item);
        }
    }

    // Caso 1: hay selección -> mostrar solo el número
    if (!selectedItems.isEmpty()) {

        QString text = QString("¿Seguro que quieres borrar los %1 items seleccionados?")
                           .arg(selectedItems.size());

        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Borrar Item",
            text,
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::No)
            return;

        for (AudioItemMaxi* item : std::as_const(selectedItems)) {
            deleteItem(item);
        }

        return;
    }

    // Caso 2: no hay selección, borrar item bajo el cursor -> mostrar nombre
    QWidget *widget = childAt(mousePos);

    while (widget && !qobject_cast<AudioItemMaxi*>(widget)) {
        widget = widget->parentWidget();
    }

    if (auto *item = qobject_cast<AudioItemMaxi*>(widget)) {

      /*  if (item->isPlaying())
             return;*/


        QString nombre = item->nameFile();

        QString text = nombre.isEmpty()
            ? "¿Seguro que quieres borrar este item?"
            : QString("¿Seguro que quieres borrar \"%1\"?").arg(nombre);

        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Borrar Item",
            text,
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::No)
            return;

        deleteItem(item);
    }
}


// ContentsPlayer.cpp
void ContentsPlayer::copySelected()
{
    clipboard.lista.clear();

    QList<AudioItemMaxi*> items = findChildren<AudioItemMaxi*>();
    bool hasSelection = false;

    for (AudioItemMaxi* item : std::as_const(items)) {
        if (item->isSelect()) {
            item->setProperty("iscut", false);
            clipboard.lista.append(item);
            hasSelection = true;
        }
    }

    // Si no hay selección, copiar item bajo el cursor
    if (!hasSelection) {
        QWidget *widget = childAt(mousePos);
        while (widget && !qobject_cast<AudioItemMaxi*>(widget))
            widget = widget->parentWidget();

        if (auto *item = qobject_cast<AudioItemMaxi*>(widget)) {
            item->setProperty("iscut", false);
            clipboard.lista.append(item);
        }
    }
}

// ContentsPlayer.cpp
void ContentsPlayer::cutSelected()
{
    clipboard.lista.clear();

    QList<AudioItemMaxi*> items = findChildren<AudioItemMaxi*>();
    bool hasSelection = false;

    for (AudioItemMaxi* item : std::as_const(items)) {
        if (item->isSelect()) {
            item->setProperty("iscut", true);  // marcar como cortado
            clipboard.lista.append(item);
            hasSelection = true;
        }
    }

    // Si no hay selección, cortar item bajo el cursor
    if (!hasSelection) {
        QWidget *widget = childAt(mousePos);
        while (widget && !qobject_cast<AudioItemMaxi*>(widget))
            widget = widget->parentWidget();

        if (auto *item = qobject_cast<AudioItemMaxi*>(widget)) {
            item->setProperty("iscut", true);
            clipboard.lista.append(item);
        }
    }
}

// ContentsPlayer.cpp
void ContentsPlayer::pasteClipboard()
{
    if (clipboard.lista.empty())
        return;

    bool isCutOperation = clipboard.lista.constFirst()->property("iscut").toBool();

    // deleteItem removes clipboard entries. Iterate over a snapshot when cutting.
    const auto copiedItems = clipboard.lista;
    for (QWidget *widget : copiedItems) {
        if (auto *itemBase = qobject_cast<AudioItemMaxi*>(widget)) {
            AudioItemMaxi *newItem = itemBase->copy(this);
            createItem(newItem);

            newItem->setIsSelect(false); // deseleccionar pegado

            if (isCutOperation) {
               //itemBase->deleteLater();
                 deleteItem(itemBase);   //
            }
        }
    }

    // limpiar selección de los originales
    for (auto it = clipboard.lista.begin(); it != clipboard.lista.end(); ++it) {
        if (auto *item = qobject_cast<AudioItemMaxi*>(*it)) {
            item->setIsSelect(false);
        }
    }
}

// ContentsPlayer.cpp
void ContentsPlayer::applyColor(const QColor &color)
{
    QList<AudioItemMaxi*> items = findChildren<AudioItemMaxi*>();
    bool hasSelection = false;

    // Aplicar color a los items seleccionados
    for (AudioItemMaxi* item : items) {
        if (item->isSelect()) {
            item->framecolor->setColor(color);
            hasSelection = true;
        }
    }

    // Si no hay selección, aplicar al item bajo el cursor
    if (!hasSelection) {
        QWidget *widget = childAt(mousePos);
        while (widget && !qobject_cast<AudioItemMaxi*>(widget))
            widget = widget->parentWidget();

        if (auto *item = qobject_cast<AudioItemMaxi*>(widget)) {
            item->framecolor->setColor(color);

        }
    }

    // Deseleccionar todos los items después de aplicar color
    for (AudioItemMaxi* item : items) {
        item->setIsSelect(false);
    }
    if (hasSelection || childAt(mousePos))
        emit contentsModified();
}


void ContentsPlayer::loadItems(){

    QString filename = QFileDialog::getOpenFileName(
               this,
               "Load list",
               m_listFileName,
               "Radit List (*.list);;All Files (*)"
           );

           if (filename.isEmpty())
               return;

        QWidget *owner = this;
        while (owner && !qobject_cast<TabPlayer*>(owner)) owner = owner->parentWidget();
        QString error;
        auto *tabs = qobject_cast<TabPlayer*>(owner);
        Io io;
        const bool loaded = tabs ? tabs->loadListFile(filename, &error)
                                 : io.LoadListPlayer(this, filename, &error);
        if (!loaded)
            QMessageBox::warning(this, tr("Load list"), error);

}
void ContentsPlayer::saveItems(){
    if (m_listFileName.isEmpty()) {
        saveAsItems();
        return;
    }
    saveListFile(m_listFileName);
}

bool ContentsPlayer::saveListFile(const QString &filename)
{
    Io io;
    QString error;
    if (!io.SaveListPlayer(layout, filename, &error)) {
        QMessageBox::warning(this, tr("Save list"), error);
        return false;
    }
    m_listFileName = QFileInfo(filename).absoluteFilePath();
    emit contentsModified();
    return true;
}

void ContentsPlayer::saveAsItems()
{
    QFileDialog dialog(this, tr("Save list as"), m_listFileName,
                       tr("Radit List (*.list);;All files (*)"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDefaultSuffix("list");
    if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty())
        return;
    const QString filename = dialog.selectedFiles().first();
    if (saveListFile(filename))
        setTabName(filename);
}

