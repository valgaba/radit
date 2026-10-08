#ifndef FILEEXPLORE_H
#define FILEEXPLORE_H

#include "widgets/frame.h"
#include <QStringList>
#include <QSet>
#include <QPointer>
#include <QFutureWatcher>

class Button;
class FileExploreFilter;
class FileExploreMenu;
class QComboBox;
class FileExploreModel;
class LoadingProgress;
class QLineEdit;
class QMenu;
class QAction;
class QTreeView;

class FileExplore : public Frame
{
    Q_OBJECT

public:
    explicit FileExplore(QWidget *parent = nullptr);
    ~FileExplore();
    QString currentPath() const;
    QStringList selectedFilePaths() const;
    bool canPasteFiles() const;

public slots:
    bool setPath(const QString &path);
    void copySelectedFiles();
    void cutSelectedFiles();
    void pasteFiles();
    void renameSelectedFile();
    void deleteSelectedFiles();
    void selectAllFiles();

signals:
    void directoryChanged(const QString &path);
    void fileActivated(const QString &path);

private:
    void updateFavoriteButton();
    void savePreferences();
    QAction *addLocationAction(QMenu *menu, const QString &label, const QString &path);
    QString pasteDestination() const;

    FileExploreModel *m_model = nullptr;
    LoadingProgress *m_loading = nullptr;
    FileExploreFilter *m_filter = nullptr;
    QTreeView *m_tree = nullptr;
    FileExploreMenu *m_fileMenu = nullptr;
    QComboBox *m_path = nullptr;
    QLineEdit *m_search = nullptr;
    QComboBox *m_sort = nullptr;
    Button *m_up = nullptr;
    Button *m_favoritesButton = nullptr;
    QStringList m_favorites;
    QStringList m_driveRoots;
    QPointer<QMenu> m_locationsMenu;
    QPointer<QAction> m_drivesPlaceholder;
    QFutureWatcher<QStringList> *m_driveWatcher = nullptr;
    bool m_driveScanStarted = false;
    bool m_driveRootsLoaded = false;
    QStringList m_history;
    QString m_currentPath;
    QSet<QString> m_restoreSelection;
};

#endif // FILEEXPLORE_H
