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
#include "widgets/FileExploreModel.h"
#include "widgets/LoadingProgress.h"
#include "widgets/menu.h"
#include <functional>
#include <QKeySequence>
#include "core/MediaManager.h"
#include "widgets/button.h"
#include "widgets/label.h"
#include "widgets/scrollbar.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QTimer>
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

// This proxy only filters names. Ordering is prepared on the directory worker.
class FileExploreFilter : public QSortFilterProxyModel
{
public:
    explicit FileExploreFilter(QObject *parent) : QSortFilterProxyModel(parent) {}
    void setSearch(const QString &text) { m_search = text.trimmed(); invalidateFilter(); }
    void setSortByCreationDate(bool enabled)
    { static_cast<FileExploreModel *>(sourceModel())->setSortByCreationDate(enabled); }
protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override
    {
        return m_search.isEmpty() || sourceModel()->index(row, 0, parent).data().toString()
            .contains(m_search, Qt::CaseInsensitive);
    }
private:
    QString m_search;
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
    filters.append("*.player");
    filters.append("*.zip");
    filters.removeDuplicates();
    return filters;
}

QStringList clipboardFiles(bool validate = true)
{
    QStringList paths;
    const QMimeData *data = QApplication::clipboard()->mimeData();
    if (!data)
        return paths;
    for (const QUrl &url : data->urls()) {
        if (!url.isLocalFile())
            continue;
        const QFileInfo info(url.toLocalFile());
        if ((!validate || info.isFile()) && QDir::match(explorerNameFilters(), info.fileName()))
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

void installRaditEditMenu(QLineEdit *edit)
{
    edit->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(edit, &QWidget::customContextMenuRequested, edit, [edit](const QPoint &position) {
        Menu menu(edit);
        const int selectionStart = edit->selectionStart();
        const int selectionLength = edit->selectedText().size();
        const int cursorPosition = edit->cursorPosition();
        const auto addEditAction = [edit, &menu, selectionStart, selectionLength, cursorPosition](const QString &text, const QString &iconPath,
                                          const QKeySequence &shortcut, bool enabled,
                                          const std::function<void()> &callback) {
            QIcon icon(iconPath);
            icon.addFile(iconPath, QSize(), QIcon::Disabled);
            QAction *action = menu.addAction(icon, text);
            action->setShortcut(shortcut);
            action->setEnabled(enabled);
            QObject::connect(action, &QAction::triggered, &menu,
                [edit, callback, selectionStart, selectionLength, cursorPosition]() {
                    edit->setFocus();
                    if (selectionStart >= 0)
                        edit->setSelection(selectionStart, selectionLength);
                    else
                        edit->setCursorPosition(cursorPosition);
                    callback();
                });
        };
        addEditAction(FileExplore::tr("Undo"), ":/icons/Undo.svg", QKeySequence::Undo,
                      edit->isUndoAvailable(), [edit]() { edit->undo(); });
        addEditAction(FileExplore::tr("Redo"), ":/icons/Redo.svg", QKeySequence::Redo,
                      edit->isRedoAvailable(), [edit]() { edit->redo(); });
        menu.addSeparator();
        const bool selected = edit->hasSelectedText();
        addEditAction(FileExplore::tr("Cut"), ":/icons/ActionCut.svg", QKeySequence::Cut,
                      selected, [edit]() { edit->cut(); });
        addEditAction(FileExplore::tr("Copy"), ":/icons/ActionCopy.svg", QKeySequence::Copy,
                      selected, [edit]() { edit->copy(); });
        addEditAction(FileExplore::tr("Paste"), ":/icons/ActionPaste.svg", QKeySequence::Paste,
                      !QApplication::clipboard()->text().isEmpty(), [edit]() { edit->paste(); });
        addEditAction(FileExplore::tr("Delete"), ":/icons/Remove.svg", QKeySequence(Qt::Key_Delete),
                      selected, [edit]() { edit->del(); });
        menu.addSeparator();
        addEditAction(FileExplore::tr("Select All"), ":/icons/Selectall.svg", QKeySequence::SelectAll,
                      !edit->text().isEmpty(), [edit]() { edit->selectAll(); });
        menu.exec(edit->mapToGlobal(position));
    });
}
}


FileExplore::FileExplore(QWidget *parent) : Frame(parent)
{
    setObjectName("FileExplore");
    setMinimumWidth(220);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *framebarra = new Frame(this);
    framebarra->setObjectName("framebarra");
    framebarra->setFixedHeight(25);
    auto *layoutbarra = new QHBoxLayout(framebarra);
    layoutbarra->setContentsMargins(0, 0, 0, 0);
    layoutbarra->setSpacing(0);
    auto *labeltitle = new Label(framebarra);
    labeltitle->setObjectName("PanelTitle");
    labeltitle->setText(tr("File Browser"));
    layoutbarra->addWidget(labeltitle);
    layoutbarra->addStretch(1);
    auto *btnclose = new Button(framebarra);
    btnclose->setStyleSheet("QPushButton { border: none; background: transparent; padding: 0px; }");
    btnclose->setFixedSize(15, 15);
    btnclose->SetIcon("Close-hover.svg");
    btnclose->setIconSize(QSize(15, 15));
    btnclose->setToolTip(tr("Close file browser"));
    btnclose->setAccessibleName(btnclose->toolTip());
    layoutbarra->addWidget(btnclose);

    connect(btnclose, &Button::clicked, this, [this]() {
            hide();
    });

    layout->addWidget(framebarra);

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
    installRaditEditMenu(m_path->lineEdit());
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
        addLocation(tr("Locution"), QDir(QCoreApplication::applicationDirPath()).filePath("locution"));
        const QString capturePath = QDir(QCoreApplication::applicationDirPath()).filePath("capture");
        QAction *captureAction = locationsMenu->addAction(tr("Capture"));
        captureAction->setToolTip(QDir::toNativeSeparators(capturePath));
        connect(captureAction, &QAction::triggered, this, [this, capturePath]() {
            if (!QDir().mkpath(capturePath) || !setPath(capturePath)) {
                QMessageBox::warning(this, tr("Capture folder"),
                    tr("Unable to open the capture folder: %1")
                        .arg(QDir::toNativeSeparators(capturePath)));
            }
        });
        locationsMenu->addSeparator();
        locationsMenu->addAction(tr("Favorites"))->setEnabled(false);
        for (const QString &path : m_favorites) {
            const QString name = QDir(path).dirName();
            addLocation(name.isEmpty() ? QDir::toNativeSeparators(path) : name, path);
        }
        if (m_favorites.isEmpty())
            locationsMenu->addAction(tr("No favorite folders"))->setEnabled(false);
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
    m_favoritesButton = favorites;
    favorites->setObjectName("FileExploreFavorites");
    favorites->setFixedSize(30, 30);
    favorites->setIcon(QIcon(":/icons/Favorites.svg"));
    favorites->setCheckable(true);
    connect(favorites, &Button::clicked, this, [this]() {
        if (m_favorites.contains(m_currentPath))
            m_favorites.removeAll(m_currentPath);
        else
            m_favorites.append(m_currentPath);
        savePreferences();
        updateFavoriteButton();
    });
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
    installRaditEditMenu(m_search);
    searchLayout->addWidget(m_search, 1);
    m_sort = new QComboBox(this);
    m_sort->setObjectName("FileExploreSort");
    m_sort->setFixedHeight(26);
    m_sort->addItem(tr("Name"), "name");
    m_sort->addItem(tr("Creation date"), "created");
    m_sort->setAccessibleName(tr("Sort files"));
    m_sort->setToolTip(tr("Sort by name or creation date (newest first). "
                        "If creation date is unavailable, modification date is used."));
    searchLayout->addWidget(m_sort);
    layout->addLayout(searchLayout);

    m_loading = new LoadingProgress(this);
    layout->addWidget(m_loading);
    m_model = new FileExploreModel(explorerNameFilters(), this);
    m_model->rootProgress = [this](int completed, int total) {
        if (completed < 0) {
            m_loading->finish();
            if (m_fileMenu) m_fileMenu->updateActions();
        } else if (total == 0) {
            m_restoreSelection.clear();
            if (m_tree) {
                for (const QModelIndex &index : m_tree->selectionModel()->selectedRows(0))
                    m_restoreSelection.insert(m_model->filePath(m_filter->mapToSource(index)));
            }
            m_loading->begin(tr("Reading folder..."));
        } else {
            if (m_loading->maximum() != total)
                m_loading->begin(tr("Loading files..."), total);
            m_loading->setProgress(completed, total);
        }
    };
    m_model->rootError = [this](const QString &error) {
        m_path->setToolTip(error);
        m_loading->finish();
    };
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
    m_tree->setSortingEnabled(false); // Worker supplies sorted rows; never sort during insertion.
    layout->addWidget(m_tree, 1);

    // Restore selection after sorting/refresh without scanning the complete list.
    connect(m_model, &QAbstractItemModel::rowsInserted, this,
        [this](const QModelIndex &parent, int first, int last) {
            if (m_restoreSelection.isEmpty()) return;
            QItemSelection selection;
            for (int row = first; row <= last; ++row) {
                const QModelIndex source = m_model->index(row, 0, parent);
                if (!m_restoreSelection.remove(m_model->filePath(source))) continue;
                const QModelIndex index = m_filter->mapFromSource(source);
                if (index.isValid()) selection.select(index, index);
            }
            if (!selection.isEmpty())
                m_tree->selectionModel()->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        });

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
        setPath(QDir::cleanPath(m_currentPath + "/.."));
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
        m_tree->setRootIndex(QModelIndex());
    });
    connect(m_sort, &QComboBox::currentIndexChanged, this, [this]() {
        m_filter->setSortByCreationDate(m_sort->currentData().toString() == "created");
        savePreferences();
    });
    connect(m_tree, &QTreeView::activated, this, [this](const QModelIndex &index) {
        const QModelIndex source = m_filter->mapToSource(index);
        const QString path = m_model->filePath(source);
        if (m_model->isDir(source))
            setPath(path);
        else
            emit fileActivated(path);
    });

    QSettings settings(preferencesFile(), QSettings::IniFormat);
    m_favorites = settings.value("favorites").toStringList();
    m_favorites.removeDuplicates();
    {
        const QSignalBlocker blocker(m_sort);
        const int sortIndex = m_sort->findData(settings.value("sortMode", "name").toString());
        m_sort->setCurrentIndex(sortIndex >= 0 ? sortIndex : 0);
    }
    m_filter->setSortByCreationDate(m_sort->currentData().toString() == "created");
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
    const QString normalized = QDir::fromNativeSeparators(path.trimmed());
    if (normalized.isEmpty() || QDir::isRelativePath(normalized)) return false;
    // Existence and permissions are checked by the worker, including offline shares.
    const QString absolutePath = QDir::cleanPath(normalized);
    const bool changed = absolutePath != m_currentPath;
    m_currentPath = absolutePath;
    m_model->setRootPath(m_currentPath);
    m_search->clear();
    m_filter->setSearch(QString());
    m_tree->setRootIndex(QModelIndex());

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
    updateFavoriteButton();
    if (m_fileMenu)
        m_fileMenu->updateActions();
    savePreferences();
    if (changed)
        emit directoryChanged(m_currentPath);
    return true;
}

void FileExplore::updateFavoriteButton()
{
    const bool favorite = m_favorites.contains(m_currentPath);
    m_favoritesButton->setChecked(favorite);
    m_favoritesButton->setToolTip(favorite ? tr("Remove current folder from favorites")
                                         : tr("Add current folder to favorites"));
    m_favoritesButton->setAccessibleName(m_favoritesButton->toolTip());
}

void FileExplore::savePreferences()
{
    QSettings settings(preferencesFile(), QSettings::IniFormat);
    settings.setValue("path", m_currentPath);
    settings.setValue("favorites", m_favorites);
    settings.setValue("sortMode", m_sort->currentData());
}

QStringList FileExplore::selectedFilePaths() const
{
    QStringList paths;
    for (const QModelIndex &index : m_tree->selectionModel()->selectedRows(0)) {
        const QModelIndex source = m_filter->mapToSource(index);
        // Use cached directory flags and paths; selection must not query the network.
        if (m_model->isDir(source)) return {};
        paths.append(m_model->filePath(source));
    }
    paths.removeDuplicates();
    return paths;
}

QString FileExplore::pasteDestination() const
{
    const QModelIndexList selection = m_tree->selectionModel()->selectedRows(0);
    if (selection.size() == 1) {
        const QModelIndex source = m_filter->mapToSource(selection.first());
        if (m_model->isDir(source)) return m_model->filePath(source);
    }
    return m_currentPath;
}

bool FileExplore::canPasteFiles() const
{
    // Avoid touching the current network folder on every selection change.
    if (clipboardFiles(false).isEmpty()) return false;
    const QModelIndex destination = m_model->index(pasteDestination());
    return m_model->isDir(destination) &&
           (m_model->permissions(destination) & (QFile::WriteUser | QFile::WriteGroup | QFile::WriteOther));
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
    m_model->refresh();
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
    m_model->refresh();
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
    m_model->refresh();
    m_fileMenu->updateActions();
    showFileErrors(this, errors);
}

void FileExplore::selectAllFiles()
{
    m_tree->selectAll();
    const QModelIndexList selection = m_tree->selectionModel()->selectedRows(0);
    for (const QModelIndex &index : selection) {
        if (m_model->isDir(m_filter->mapToSource(index)))
            m_tree->selectionModel()->select(index, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
    }
}
