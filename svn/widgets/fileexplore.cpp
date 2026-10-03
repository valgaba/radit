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

#include "widgets/fileexplore.h"
#include "widgets/FileExploreMenu.h"
#include "core/MediaManager.h"
#include "widgets/button.h"
#include "widgets/scrollbar.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QSettings>
#include <QSignalBlocker>
#include <QSortFilterProxyModel>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QStyle>
#include <QTreeView>
#include <QUrl>
#include <QVBoxLayout>

// Conserva la raíz y sus antecesores aunque no coincidan con la búsqueda.
// El filtro se aplica por nombre a las entradas de la carpeta visible.
class FileExploreFilter : public QSortFilterProxyModel
{
public:
    explicit FileExploreFilter(QObject *parent)
        : QSortFilterProxyModel(parent), m_audioIcon(":/icons/audiofile.svg") {}

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (index.isValid() && index.column() == 0 && role == Qt::DecorationRole) {
            const auto *model = static_cast<QFileSystemModel *>(sourceModel());
            const QModelIndex source = mapToSource(index);
            if (model->fileInfo(source).isFile())
                return m_audioIcon;
        }
        return QSortFilterProxyModel::data(index, role);
    }

    void setSearch(const QString &text)
    {
        m_search = text.trimmed();
        invalidateFilter();
    }

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override
    {
        const auto *model = static_cast<QFileSystemModel *>(sourceModel());
        const QModelIndex index = model->index(row, 0, parent);
        const QString path = model->filePath(index);
        const QString root = model->rootPath();
        const QString prefix = path.endsWith('/') ? path : path + '/';
        if (path == root || root.startsWith(prefix))
            return true;
        return m_search.isEmpty() ||
               model->fileName(index).contains(m_search, Qt::CaseInsensitive);
    }

private:
    QString m_search;
    QIcon m_audioIcon;
};

namespace {
class FileExplorePathCombo : public QComboBox
{
public:
    FileExplorePathCombo(QWidget *parent, QMenu *menu)
        : QComboBox(parent), m_menu(menu)
    {
        // Restaurar el estado del combo al cerrar el menú personalizado.
        connect(m_menu, &QMenu::aboutToHide, this, &QComboBox::hidePopup);
    }

    void showPopup() override
    {
        m_menu->setMinimumWidth(width());
        m_menu->popup(mapToGlobal(QPoint(0, height())));
    }

private:
    QMenu *m_menu;
};

QStringList explorerNameFilters()
{
    QStringList filters = MediaManager::supportedAudioNameFilters();
    filters.append("*.list");
    filters.removeDuplicates();
    return filters;
}

QStringList clipboardFiles()
{
    QStringList paths;
    const QMimeData *data = QApplication::clipboard()->mimeData();
    if (!data)
        return paths;
    for (const QUrl &url : data->urls()) {
        if (!url.isLocalFile())
            continue;
        const QFileInfo info(url.toLocalFile());
        if (info.isFile() && QDir::match(explorerNameFilters(), info.fileName()))
            paths.append(info.absoluteFilePath());
    }
    paths.removeDuplicates();
    return paths;
}

bool clipboardIsCut(const QMimeData *data)
{
    if (!data)
        return false;
    const QByteArray effect = data->data(
        "application/x-qt-windows-mime;value=\"Preferred DropEffect\"");
    return data->data("application/x-radit-cut-files") == "1" ||
           data->data("application/x-kde-cutselection") == "1" ||
           (!effect.isEmpty() && (static_cast<unsigned char>(effect.at(0)) & 2));
}

void setFileClipboard(const QList<QUrl> &urls, bool cut)
{
    auto *data = new QMimeData;
    data->setUrls(urls);
    data->setData("application/x-radit-cut-files", cut ? "1" : "0");
    data->setData("application/x-kde-cutselection", cut ? "1" : "0");
    QByteArray effect(4, '\0');
    effect[0] = cut ? 2 : 1; // DWORD little-endian: MOVE / COPY.
    data->setData("application/x-qt-windows-mime;value=\"Preferred DropEffect\"", effect);
    QApplication::clipboard()->setMimeData(data);
}

QList<QUrl> fileUrls(const QStringList &paths)
{
    QList<QUrl> urls;
    for (const QString &path : paths)
        urls.append(QUrl::fromLocalFile(path));
    return urls;
}

QString availableCopyPath(const QDir &directory, const QString &fileName)
{
    QString path = directory.filePath(fileName);
    if (!QFileInfo::exists(path) && !QFileInfo(path).isSymLink())
        return path;
    const int dot = fileName.lastIndexOf('.');
    const QString base = dot > 0 ? fileName.left(dot) : fileName;
    const QString suffix = dot > 0 ? fileName.mid(dot) : QString();
    int number = 1;
    do {
        const QString copy = number == 1 ? " (copy)" : QString(" (copy %1)").arg(number);
        path = directory.filePath(base + copy + suffix);
        ++number;
    } while (QFileInfo::exists(path) || QFileInfo(path).isSymLink());
    return path;
}

void showFileErrors(QWidget *parent, const QStringList &errors)
{
    if (!errors.isEmpty())
        QMessageBox::warning(parent, FileExplore::tr("File operation failed"), errors.join("\n\n"));
}

QString preferencesFile()
{
    return QCoreApplication::applicationDirPath() + "/fileexplore.ini";
}

QIcon explorerIcon(bool search)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor(search ? "#80a4ae" : "#8a8d96"), 2,
                        Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    if (search) {
        painter.drawEllipse(QRectF(2, 2, 8, 8));
        painter.drawLine(QPointF(9, 9), QPointF(14, 14));
    } else {
        painter.drawLine(QPointF(8, 13), QPointF(8, 3));
        painter.drawLine(QPointF(3, 8), QPointF(8, 3));
        painter.drawLine(QPointF(8, 3), QPointF(13, 8));
    }
    return QIcon(pixmap);
}
}


FileExplore::FileExplore(QWidget *parent) : Frame(parent)
{
    setObjectName("FileExplore");
    setMinimumWidth(220);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *toolbar = new Frame(this);
    toolbar->setObjectName("FileExploreToolbar");
    toolbar->setFixedHeight(34);
    auto *bar = new QHBoxLayout(toolbar);
    bar->setContentsMargins(2, 2, 2, 2);
    bar->setSpacing(2);

    m_up = new Button(toolbar);
    m_up->setObjectName("FileExploreUp");
    m_up->setFixedSize(30, 30);
    m_up->setIcon(explorerIcon(false));
    m_up->setToolTip(tr("Go to parent folder"));
    m_up->setAccessibleName(m_up->toolTip());
    bar->addWidget(m_up);

    auto *locationsMenu = new QMenu(this);
    locationsMenu->setObjectName("FileExploreLocations");
    m_path = new FileExplorePathCombo(toolbar, locationsMenu);
    m_path->setObjectName("FileExplorePath");
    m_path->setEditable(true);
    m_path->setInsertPolicy(QComboBox::NoInsert);
    m_path->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_path->setMinimumContentsLength(8);
    m_path->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_path->setFixedHeight(30);
    m_path->setToolTip(tr("Folder path (press Enter to open)"));
    m_path->setAccessibleName(tr("Ruta de la carpeta"));
    bar->addWidget(m_path, 1);

    auto *locationsButton = new Button(toolbar);
    locationsButton->setObjectName("FileExploreLocationsButton");
    locationsButton->setFixedSize(30, 30);
    locationsButton->setText(QString(QChar(0x25BC))); // Flecha hacia abajo visible.
    locationsButton->setToolTip(tr("Folders and drives"));
    locationsButton->setAccessibleName(locationsButton->toolTip());
    connect(locationsButton, &Button::clicked, this, [this]() {
        m_path->showPopup();
    });
    bar->addWidget(locationsButton);

    connect(locationsMenu, &QMenu::aboutToShow, this, [this, locationsMenu]() {
        locationsMenu->clear();
        const auto addLocation = [this, locationsMenu](const QString &label,
                                                       const QString &path) {
            QAction *action = locationsMenu->addAction(label);
            action->setEnabled(!path.isEmpty() && QFileInfo(path).isDir());
            action->setToolTip(QDir::toNativeSeparators(path));
            connect(action, &QAction::triggered, this, [this, path]() { setPath(path); });
        };

        addLocation(tr("Home Folder"), QDir::homePath());
        addLocation(tr("Desktop"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
        addLocation(tr("Documents"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
        addLocation(tr("Downloads"), QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
        locationsMenu->addSeparator();
        locationsMenu->addAction(tr("Drives"))->setEnabled(false);

        QStringList roots;
        for (const QStorageInfo &storage : QStorageInfo::mountedVolumes()) {
            if (!storage.isValid() || !storage.isReady())
                continue;
            const QString root = storage.rootPath();
            if (roots.contains(root))
                continue;
            roots.append(root);
            QString label = QDir::toNativeSeparators(root);
            if (!storage.name().isEmpty())
                label += " [" + storage.name() + "]";
            addLocation(label, root);
        }
    });

    auto *favorites = new Button(toolbar);
    favorites->setObjectName("FileExploreFavorites");
    favorites->setFixedSize(30, 30);
    favorites->setIcon(QIcon(":/icons/Favorites.svg"));
    favorites->setToolTip(tr("Favorite folders"));
    favorites->setAccessibleName(favorites->toolTip());
    m_favoritesMenu = new QMenu(this);
    favorites->setMenu(m_favoritesMenu);
    bar->addWidget(favorites);

    layout->addWidget(toolbar);

    auto *searchLayout = new QHBoxLayout;
    searchLayout->setContentsMargins(8, 6, 8, 6);
    m_search = new QLineEdit(this);
    m_search->setObjectName("FileExploreSearch");
    m_search->setPlaceholderText(tr("Buscar en esta carpeta"));
    m_search->setAccessibleName(tr("Buscar por nombre"));
    m_search->setFixedHeight(26);
    m_search->setClearButtonEnabled(true);
    m_search->addAction(explorerIcon(true), QLineEdit::LeadingPosition);
    searchLayout->addWidget(m_search);
    layout->addLayout(searchLayout);

    m_model = new QFileSystemModel(this);
    m_model->setReadOnly(true);
    m_model->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    m_model->setNameFilterDisables(false);
    m_model->setNameFilters(explorerNameFilters());
    m_filter = new FileExploreFilter(this);
    m_filter->setSourceModel(m_model);
    m_filter->setSortCaseSensitivity(Qt::CaseInsensitive);

    m_tree = new QTreeView(this);
    m_tree->setObjectName("FileExploreTree");
    m_tree->setProperty("multipleSelection", false);
    m_tree->setModel(m_filter);
    m_tree->setHeaderHidden(true);
    for (int column = 1; column < m_model->columnCount(); ++column)
        m_tree->hideColumn(column);
    m_tree->header()->setStretchLastSection(true);
    m_tree->setIndentation(16);
    m_tree->setUniformRowHeights(true);
    m_tree->setAnimated(false);
    m_tree->setIconSize(QSize(16, 16));
    m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tree->setDragEnabled(true);
    m_tree->setDragDropMode(QAbstractItemView::DragOnly);
    m_tree->setVerticalScrollBar(new ScrollBar(m_tree));
    m_tree->setHorizontalScrollBar(new ScrollBar(m_tree));
    m_tree->setSortingEnabled(true);
    m_tree->sortByColumn(0, Qt::AscendingOrder);
    layout->addWidget(m_tree, 1);

    m_fileMenu = new FileExploreMenu(this, m_tree);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tree, &QTreeView::customContextMenuRequested, this, [this](const QPoint &position) {
        const QModelIndex clicked = m_tree->indexAt(position);
        if (!clicked.isValid()) {
            m_tree->clearSelection();
        } else if (!m_tree->selectionModel()->isSelected(clicked)) {
            m_tree->selectionModel()->setCurrentIndex(clicked,
                QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
        m_fileMenu->showAt(m_tree->viewport()->mapToGlobal(position));
    });
    connect(m_tree->selectionModel(), &QItemSelectionModel::selectionChanged,
            m_fileMenu, &FileExploreMenu::updateActions);
    connect(m_tree->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this]() {
        const bool multiple = m_tree->selectionModel()->selectedRows(0).size() > 1;
        if (m_tree->property("multipleSelection").toBool() == multiple)
            return;
        m_tree->setProperty("multipleSelection", multiple);
        m_tree->style()->unpolish(m_tree);
        m_tree->style()->polish(m_tree);
        m_tree->viewport()->update();
    });
    connect(QApplication::clipboard(), &QClipboard::dataChanged,
            m_fileMenu, &FileExploreMenu::updateActions);

    connect(m_up, &Button::clicked, this, [this]() {
        QDir dir(m_currentPath);
        if (dir.cdUp())
            setPath(dir.absolutePath());
    });
    connect(m_path->lineEdit(), &QLineEdit::returnPressed, this, [this]() {
        const QString typedPath = m_path->currentText();
        const QString path = QDir::isRelativePath(typedPath)
                ? QDir(m_currentPath).absoluteFilePath(typedPath) : typedPath;
        if (!setPath(path)) {
            m_path->setEditText(QDir::toNativeSeparators(m_currentPath));
            m_path->setToolTip(tr("Folder does not exist or is not accessible: %1").arg(typedPath));
        }
    });
    connect(m_search, &QLineEdit::textChanged, this, [this](const QString &text) {
        m_filter->setSearch(text);
        m_tree->setRootIndex(m_filter->mapFromSource(m_model->index(m_currentPath)));
    });
    connect(m_tree, &QTreeView::activated, this, [this](const QModelIndex &index) {
        const QModelIndex source = m_filter->mapToSource(index);
        const QString path = m_model->filePath(source);
        if (m_model->isDir(source))
            setPath(path);
        else
            emit fileActivated(path);
    });
    connect(m_favoritesMenu, &QMenu::aboutToShow,
            this, &FileExplore::updateFavoritesMenu);

    QSettings settings(preferencesFile(), QSettings::IniFormat);
    m_favorites = settings.value("favorites").toStringList();
    m_favorites.removeDuplicates();
    QString initialPath = settings.value("path").toString();
    if (!setPath(initialPath)) {
        initialPath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
        if (!setPath(initialPath))
            setPath(QDir::homePath());
    }
}



FileExplore::~FileExplore() = default;

QString FileExplore::currentPath() const
{
    return m_currentPath;
}

bool FileExplore::setPath(const QString &path)
{
    const QFileInfo info(QDir::fromNativeSeparators(path.trimmed()));
    if (path.trimmed().isEmpty() || !info.exists() || !info.isDir() || !info.isReadable())
        return false;

    const QString absolutePath = QDir::cleanPath(info.absoluteFilePath());
    const bool changed = absolutePath != m_currentPath;
    m_currentPath = absolutePath;
    m_model->setNameFilters(explorerNameFilters());
    m_model->setRootPath(m_currentPath);
    m_search->clear();
    m_filter->setSearch(QString());
    m_tree->setRootIndex(m_filter->mapFromSource(m_model->index(m_currentPath)));

    m_history.removeAll(m_currentPath);
    m_history.prepend(m_currentPath);
    while (m_history.size() > 15)
        m_history.removeLast();
    const QSignalBlocker blocker(m_path);
    m_path->clear();
    for (const QString &entry : m_history)
        m_path->addItem(QDir::toNativeSeparators(entry), entry);
    m_path->setCurrentIndex(0);
    m_path->setToolTip(tr("Folder path (press Enter to open): %1")
                       .arg(QDir::toNativeSeparators(m_currentPath)));
    m_up->setEnabled(!QDir(m_currentPath).isRoot());
    if (m_fileMenu)
        m_fileMenu->updateActions();
    savePreferences();
    if (changed)
        emit directoryChanged(m_currentPath);
    return true;
}

void FileExplore::updateFavoritesMenu()
{
    m_favoritesMenu->clear();
    const bool favorite = m_favorites.contains(m_currentPath);
    QAction *toggle = m_favoritesMenu->addAction(favorite
        ? tr("Quitar esta carpeta de favoritos") : tr("Añadir esta carpeta a favoritos"));
    connect(toggle, &QAction::triggered, this, [this, favorite]() {
        if (favorite)
            m_favorites.removeAll(m_currentPath);
        else
            m_favorites.append(m_currentPath);
        savePreferences();
    });
    m_favoritesMenu->addSeparator();
    for (const QString &path : m_favorites) {
        QAction *action = m_favoritesMenu->addAction(QDir::toNativeSeparators(path));
        action->setEnabled(QFileInfo(path).isDir());
        connect(action, &QAction::triggered, this, [this, path]() { setPath(path); });
    }
    if (m_favorites.isEmpty())
        m_favoritesMenu->addAction(tr("Sin carpetas favoritas"))->setEnabled(false);
}

void FileExplore::savePreferences()
{
    QSettings settings(preferencesFile(), QSettings::IniFormat);
    settings.setValue("path", m_currentPath);
    settings.setValue("favorites", m_favorites);
}

QStringList FileExplore::selectedFilePaths() const
{
    QStringList paths;
    for (const QModelIndex &index : m_tree->selectionModel()->selectedRows(0)) {
        const QFileInfo info = m_model->fileInfo(m_filter->mapToSource(index));
        // Las operaciones de este menú son para ficheros, no carpetas.
        if (!info.isFile())
            return {};
        paths.append(info.absoluteFilePath());
    }
    paths.removeDuplicates();
    return paths;
}

QString FileExplore::pasteDestination() const
{
    const QModelIndexList selection = m_tree->selectionModel()->selectedRows(0);
    if (selection.size() == 1) {
        const QFileInfo info = m_model->fileInfo(m_filter->mapToSource(selection.first()));
        if (info.isDir())
            return info.absoluteFilePath();
    }
    return m_currentPath;
}

bool FileExplore::canPasteFiles() const
{
    const QFileInfo destination(pasteDestination());
    return destination.isDir() && destination.isWritable() && !clipboardFiles().isEmpty();
}

void FileExplore::copySelectedFiles()
{
    const QStringList paths = selectedFilePaths();
    if (!paths.isEmpty())
        setFileClipboard(fileUrls(paths), false);
}

void FileExplore::cutSelectedFiles()
{
    const QStringList paths = selectedFilePaths();
    if (!paths.isEmpty())
        setFileClipboard(fileUrls(paths), true);
}

void FileExplore::pasteFiles()
{
    if (!canPasteFiles())
        return;
    const QMimeData *data = QApplication::clipboard()->mimeData();
    const bool cut = clipboardIsCut(data);
    QList<QUrl> remaining = data->urls();
    const QStringList paths = clipboardFiles();
    const QDir destination(pasteDestination());
    QStringList errors;

    for (const QString &path : paths) {
        const QFileInfo source(path);
        if (cut && source.dir().canonicalPath() == destination.canonicalPath()) {
            remaining.removeAll(QUrl::fromLocalFile(path));
            continue;
        }
        const QString target = availableCopyPath(destination, source.fileName());
        QFile file(path);
        bool success = cut ? file.rename(target) : file.copy(target);
        if (cut && !success && !QFileInfo::exists(target)) {
            // Movimiento entre volúmenes: copiar y borrar solo tras copiar con éxito.
            if (file.copy(target)) {
                success = file.remove();
                if (!success) {
                    errors.append(tr("Copied to %1, but could not remove the original %2: %3")
                                  .arg(target, path, file.errorString()));
                    continue;
                }
            }
        }
        if (success) {
            if (cut)
                remaining.removeAll(QUrl::fromLocalFile(path));
        } else {
            errors.append(tr("Could not %1 %2 to %3: %4")
                          .arg(cut ? tr("move") : tr("copy"), path, target, file.errorString()));
        }
    }
    if (cut) {
        if (remaining.isEmpty())
            QApplication::clipboard()->clear();
        else
            setFileClipboard(remaining, true);
    }
    m_fileMenu->updateActions();
    showFileErrors(this, errors);
}

void FileExplore::renameSelectedFile()
{
    const QStringList paths = selectedFilePaths();
    if (paths.size() != 1)
        return;
    const QFileInfo source(paths.first());
    bool accepted = false;
    const QString name = QInputDialog::getText(this, tr("Rename file"), tr("New name:"),
                                              QLineEdit::Normal, source.fileName(), &accepted);
    if (!accepted || name == source.fileName())
        return;
    if (name.trimmed().isEmpty() || name == "." || name == ".." ||
        name.contains('/') || name.contains('\\') || QDir::isAbsolutePath(name)) {
        showFileErrors(this, {tr("Please enter a valid file name without a folder path.")});
        return;
    }
    const QString target = source.dir().filePath(name);
    if (QFileInfo::exists(target) || QFileInfo(target).isSymLink()) {
        showFileErrors(this, {tr("A file with that name already exists: %1").arg(target)});
        return;
    }
    QFile file(source.absoluteFilePath());
    if (!file.rename(target))
        showFileErrors(this, {tr("Could not rename %1: %2").arg(source.fileName(), file.errorString())});
    m_fileMenu->updateActions();
}

void FileExplore::deleteSelectedFiles()
{
    const QStringList paths = selectedFilePaths();
    if (paths.isEmpty())
        return;
    if (QMessageBox::question(this, tr("Delete files"),
        tr("Move %n selected file(s) to the trash?", nullptr, paths.size()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;

    QStringList errors;
    for (const QString &path : paths) {
        QFile file(path);
        if (!file.moveToTrash())
            errors.append(tr("Could not move %1 to the trash: %2").arg(path, file.errorString()));
    }
    m_fileMenu->updateActions();
    showFileErrors(this, errors);
}

void FileExplore::selectAllFiles()
{
    m_tree->selectAll();
    const QModelIndexList selection = m_tree->selectionModel()->selectedRows(0);
    for (const QModelIndex &index : selection) {
        if (!m_model->fileInfo(m_filter->mapToSource(index)).isFile())
            m_tree->selectionModel()->select(index, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
    }
}
