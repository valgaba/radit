#ifndef FILEEXPLOREMODEL_H
#define FILEEXPLOREMODEL_H

#include <QAbstractItemModel>
#include <QDateTime>
#include <QDirIterator>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QIcon>
#include <QMimeData>
#include <QPointer>
#include <QTimer>
#include <QThreadPool>
#include <QUrl>
#include <QtConcurrent>
#include <algorithm>
#include <atomic>
#include <functional>
#include <memory>
#include <vector>

// Filesystem access and sorting run on workers. Only small, unsorted insertion
// batches reach the GUI; no QFileSystemModel gatherer or shell icons are used.
class FileExploreModel : public QAbstractItemModel
{
    struct Entry {
        QString path, name;
        bool directory = false;
        QFile::Permissions permissions;
        QDateTime date;
    };
    struct Scan {
        QDateTime stamp;
        QFile::Permissions permissions;
        QString error;
    };
    struct Node {
        Entry entry;
        Node *parent = nullptr;
        int row = 0;
        bool attached = true, loaded = false, loading = false;
        bool scanPublishedRows = false;
        QDateTime stamp;
        std::vector<std::shared_ptr<Node>> children;
        std::shared_ptr<std::atomic_bool> cancel;
    };

public:
    explicit FileExploreModel(const QStringList &filters, QObject *parent)
        : QAbstractItemModel(parent), m_filters(filters),
          m_folder(":/icons/folder.svg"), m_audio(":/icons/audiofile.svg")
    {
        m_root = std::make_shared<Node>();
        m_root->entry.directory = true;
        // Probe loaded folders in a worker, rather than opening network watches
        // or checking timestamps on the GUI thread.
        m_poll.setInterval(2000);
        connect(&m_poll, &QTimer::timeout, this, [this]() { pollFolders(); });
        m_poll.start();
    }

    ~FileExploreModel() override { detach(m_root); }

    std::function<void(int, int)> rootProgress;
    std::function<void(const QString &)> rootError;

    QString rootPath() const { return m_root->entry.path; }
    QModelIndex setRootPath(const QString &path)
    {
        beginResetModel();
        detach(m_root);
        m_root = std::make_shared<Node>();
        m_root->entry.path = path;
        m_root->entry.directory = true;
        endResetModel();
        load(m_root);
        return {};
    }

    void setSortByCreationDate(bool enabled)
    {
        if (m_created == enabled) return;
        m_created = enabled;
        if (!rootPath().isEmpty()) load(m_root);
    }

    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override
    {
        Node *node = nodeFor(parent);
        if (column != 0 || row < 0 || row >= int(node->children.size())) return {};
        return createIndex(row, 0, node->children[row].get());
    }

    QModelIndex index(const QString &path) const
    {
        const auto find = [this, &path](auto &&self, const std::shared_ptr<Node> &node) -> QModelIndex {
            for (const auto &child : node->children) {
                if (child->entry.path == path) return createIndex(child->row, 0, child.get());
                const QModelIndex result = self(self, child);
                if (result.isValid()) return result;
            }
            return {};
        };
        return find(find, m_root);
    }

    QModelIndex parent(const QModelIndex &index) const override
    {
        if (!index.isValid()) return {};
        Node *parent = nodeFor(index)->parent;
        if (!parent || parent == m_root.get()) return {};
        return createIndex(parent->row, 0, parent);
    }

    int rowCount(const QModelIndex &parent = {}) const override
    { return parent.column() > 0 ? 0 : int(nodeFor(parent)->children.size()); }
    int columnCount(const QModelIndex & = {}) const override { return 1; }
    bool hasChildren(const QModelIndex &parent = {}) const override
    {
        const Node *node = nodeFor(parent);
        return node->entry.directory && (!node->loaded || !node->children.empty());
    }
    bool canFetchMore(const QModelIndex &parent) const override
    {
        const Node *node = nodeFor(parent);
        return !node->entry.path.isEmpty() && node->entry.directory && !node->loaded && !node->loading;
    }
    void fetchMore(const QModelIndex &parent) override
    {
        Node *node = nodeFor(parent);
        if (canFetchMore(parent)) load(sharedNode(node));
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid()) return {};
        const auto &entry = nodeFor(index)->entry;
        if (role == Qt::DisplayRole) return entry.name;
        if (role == Qt::DecorationRole) return entry.directory ? m_folder : m_audio;
        if (role == Qt::ToolTipRole) return QDir::toNativeSeparators(entry.path);
        return {};
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        if (!index.isValid()) return Qt::NoItemFlags;
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
    }
    QStringList mimeTypes() const override { return {QStringLiteral("text/uri-list")}; }
    QMimeData *mimeData(const QModelIndexList &indexes) const override
    {
        auto *mime = new QMimeData;
        QList<QUrl> urls;
        for (const auto &index : indexes) {
            if (!index.isValid()) continue;
            const QUrl url = QUrl::fromLocalFile(filePath(index));
            if (!urls.contains(url)) urls.append(url);
        }
        mime->setUrls(urls);
        return mime;
    }
    Qt::DropActions supportedDragActions() const override { return Qt::CopyAction; }
    QString filePath(const QModelIndex &index) const { return nodeFor(index)->entry.path; }
    QString fileName(const QModelIndex &index) const { return nodeFor(index)->entry.name; }
    bool isDir(const QModelIndex &index) const { return nodeFor(index)->entry.directory; }
    QFile::Permissions permissions(const QModelIndex &index) const { return nodeFor(index)->entry.permissions; }

    void refresh() { load(m_root); }

private:
    Node *nodeFor(const QModelIndex &index) const
    { return index.isValid() ? static_cast<Node *>(index.internalPointer()) : m_root.get(); }
    std::shared_ptr<Node> sharedNode(Node *node) const
    {
        if (node == m_root.get()) return m_root;
        return node->parent->children[node->row];
    }
    static void detach(const std::shared_ptr<Node> &node)
    {
        node->attached = false;
        if (node->cancel) node->cancel->store(true);
        for (const auto &child : node->children) detach(child);
    }
    QModelIndex nodeIndex(const std::shared_ptr<Node> &node) const
    { return node == m_root ? QModelIndex() : createIndex(node->row, 0, node.get()); }

    void load(const std::shared_ptr<Node> &node)
    {
        if (!node || !node->attached || node->entry.path.isEmpty()) return;
        if (node->cancel) node->cancel->store(true);
        const auto cancel = std::make_shared<std::atomic_bool>(false);
        node->cancel = cancel;
        node->loading = true;
        node->scanPublishedRows = false;
        if (node == m_root && rootProgress) rootProgress(0, 0);
        const QString path = node->entry.path;
        const auto filters = m_filters;
        const bool created = m_created;
        const std::weak_ptr<Node> weak = node;
        const QPointer<FileExploreModel> guardedModel(this);
        QThreadPool::globalInstance()->start([guardedModel, weak, cancel, path, filters, created]() {
            Scan scan;
            QVector<Entry> batch;
            batch.reserve(32);
            const auto publishBatch = [&]() {
                if (batch.isEmpty() || cancel->load()) return;
                QVector<Entry> published;
                published.swap(batch);
                batch.reserve(32);
                QMetaObject::invokeMethod(guardedModel.data(), [guardedModel, weak, cancel,
                                                         published = std::move(published)]() mutable {
                    if (!guardedModel) return;
                    const auto node = weak.lock();
                    if (!node || !node->attached || cancel->load() || node->cancel != cancel)
                        return;
                    guardedModel->appendBatch(node, std::move(published), cancel);
                }, Qt::QueuedConnection);
            };

            const QFileInfo directory(path);
            if (!directory.isDir() || !directory.isReadable()) {
                scan.error = QStringLiteral("Folder does not exist or is not accessible: %1").arg(path);
            } else {
                scan.stamp = directory.lastModified();
                scan.permissions = directory.permissions();
                QDirIterator iterator(path, QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
                while (!cancel->load() && iterator.hasNext()) {
                    iterator.next();
                    const QFileInfo info = iterator.fileInfo();
                    const bool isDir = info.isDir();
                    if (!isDir && !QDir::match(filters, info.fileName())) continue;
                    Entry entry;
                    entry.path = info.absoluteFilePath();
                    entry.name = info.fileName();
                    entry.directory = isDir;
                    // File permissions are not used by the browser. Avoid an
                    // extra metadata query for every audio file on network shares.
                    if (isDir) entry.permissions = info.permissions();
                    if (created && !isDir) {
                        entry.date = info.birthTime();
                        if (!entry.date.isValid()) entry.date = info.lastModified();
                    }
                    batch.push_back(std::move(entry));
                    if (batch.size() >= 32) publishBatch();
                }
                publishBatch();
            }

            QMetaObject::invokeMethod(guardedModel.data(), [guardedModel, weak, cancel,
                                                             scan = std::move(scan), created]() {
                if (!guardedModel) return;
                const auto node = weak.lock();
                if (!node || !node->attached || cancel->load() || node->cancel != cancel)
                    return;
                if (!scan.error.isEmpty()) {
                    node->loading = false;
                    node->loaded = true;
                    if (node == guardedModel->m_root) {
                        if (guardedModel->rootProgress) guardedModel->rootProgress(-1, 0);
                        if (guardedModel->rootError) guardedModel->rootError(scan.error);
                    }
                    return;
                }
                if (!node->scanPublishedRows)
                    guardedModel->clearChildren(node);
                node->stamp = scan.stamp;
                node->entry.permissions = scan.permissions;
                guardedModel->sortChildren(node, created);
                node->loading = false;
                node->loaded = true;
                if (node == guardedModel->m_root && guardedModel->rootProgress)
                    guardedModel->rootProgress(-1, 0);
            }, Qt::QueuedConnection);
        });
    }

    void clearChildren(const std::shared_ptr<Node> &node)
    {
        if (node->children.empty()) return;
        const QModelIndex parent = nodeIndex(node);
        beginRemoveRows(parent, 0, int(node->children.size())-1);
        for (const auto &child : node->children) detach(child);
        node->children.clear();
        endRemoveRows();
    }

    void appendBatch(const std::shared_ptr<Node> &node, QVector<Entry> entries,
                     const std::shared_ptr<std::atomic_bool> &cancel)
    {
        if (!node->attached || cancel->load() || node->cancel != cancel || entries.isEmpty()) return;
        if (!node->scanPublishedRows) {
            clearChildren(node);
            node->scanPublishedRows = true;
        }
        const int first = int(node->children.size());
        const int last = first + entries.size() - 1;
        beginInsertRows(nodeIndex(node), first, last);
        for (Entry &entry : entries) {
            auto child = std::make_shared<Node>();
            child->entry = std::move(entry);
            child->parent = node.get();
            child->row = int(node->children.size());
            node->children.push_back(std::move(child));
        }
        endInsertRows();
    }

    void sortChildren(const std::shared_ptr<Node> &node, bool created)
    {
        if (node->children.size() < 2) return;
        const QModelIndexList oldIndexes = persistentIndexList();
        emit layoutAboutToBeChanged();
        std::stable_sort(node->children.begin(), node->children.end(), [created](const auto &a, const auto &b) {
            if (a->entry.directory != b->entry.directory) return a->entry.directory;
            if (created && !a->entry.directory && a->entry.date != b->entry.date)
                return a->entry.date > b->entry.date;
            const int cmp = QString::compare(a->entry.name, b->entry.name, Qt::CaseInsensitive);
            return cmp ? cmp < 0 : a->entry.name < b->entry.name;
        });
        for (int row=0; row<int(node->children.size()); ++row)
            node->children[row]->row = row;
        QModelIndexList newIndexes;
        newIndexes.reserve(oldIndexes.size());
        for (const QModelIndex &oldIndex : oldIndexes) {
            auto *persistentNode = static_cast<Node *>(oldIndex.internalPointer());
            newIndexes.append(persistentNode && persistentNode->attached
                ? createIndex(persistentNode->row, oldIndex.column(), persistentNode)
                : QModelIndex());
        }
        changePersistentIndexList(oldIndexes, newIndexes);
        emit layoutChanged();
    }

    void pollFolders()
    {
        if (m_polling || m_root->entry.path.isEmpty()) return;
        QList<QPair<std::weak_ptr<Node>, QString>> folders;
        const auto collect = [&folders](auto &&self, const std::shared_ptr<Node> &node) -> void {
            if (node->entry.directory && node->loaded && !node->loading)
                folders.append({node, node->entry.path});
            for (const auto &child : node->children) self(self, child);
        };
        collect(collect, m_root);
        if (folders.isEmpty()) return;
        m_polling = true;
        auto *watcher = new QFutureWatcher<QList<QDateTime>>(this);
        connect(watcher, &QFutureWatcherBase::finished, this, [this, watcher, folders]() {
            m_polling = false;
            const auto stamps = watcher->result();
            watcher->deleteLater();
            for (int i=0; i<folders.size(); ++i) {
                const auto node = folders[i].first.lock();
                if (node && node->attached && !node->loading && stamps[i] != node->stamp)
                    load(node);
            }
        });
        watcher->setFuture(QtConcurrent::run([folders]() {
            QList<QDateTime> stamps;
            for (const auto &folder : folders) stamps.append(QFileInfo(folder.second).lastModified());
            return stamps;
        }));
    }

    QStringList m_filters;
    QIcon m_folder, m_audio;
    std::shared_ptr<Node> m_root;
    QTimer m_poll;
    bool m_created = false, m_polling = false;
};
#endif
