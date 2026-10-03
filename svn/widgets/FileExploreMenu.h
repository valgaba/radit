#ifndef FILEEXPLOREMENU_H
#define FILEEXPLOREMENU_H

#include "widgets/menu.h"

class FileExplore;
class QAction;

class FileExploreMenu : public Menu
{
    Q_OBJECT

public:
    explicit FileExploreMenu(FileExplore *parent, QWidget *shortcutTarget);
    void showAt(const QPoint &globalPosition);
    void updateActions();

private:
    void setupActions(QWidget *shortcutTarget);
    void connectActions();

    FileExplore *m_explorer;
    QAction *m_cut = nullptr;
    QAction *m_copy = nullptr;
    QAction *m_paste = nullptr;
    QAction *m_rename = nullptr;
    QAction *m_delete = nullptr;
    QAction *m_selectAll = nullptr;
};

#endif // FILEEXPLOREMENU_H
