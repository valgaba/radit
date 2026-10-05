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


#include "widgets/FileExploreMenu.h"
#include "widgets/fileexplore.h"

#include <QAction>
#include <QKeySequence>

FileExploreMenu::FileExploreMenu(FileExplore *parent, QWidget *shortcutTarget)
    : Menu(parent), m_explorer(parent)
{
    setupActions(shortcutTarget);
    connectActions();
    connect(this, &QMenu::aboutToShow, this, &FileExploreMenu::updateActions);
    updateActions();
}

void FileExploreMenu::setupActions(QWidget *shortcutTarget)
{
    const auto action = [this, shortcutTarget](const QString &text,
                                             const QString &icon,
                                             const QKeySequence &shortcut) {
        auto *result = new QAction(QIcon(icon), text, this);
        result->setShortcut(shortcut);
        result->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        shortcutTarget->addAction(result);
        addAction(result);
        return result;
    };

    m_cut = action(tr("Cut"), ":/icons/ActionCut.svg", QKeySequence::Cut);
    m_copy = action(tr("Copy"), ":/icons/ActionCopy.svg", QKeySequence::Copy);
    m_paste = action(tr("Paste"), ":/icons/ActionPaste.svg", QKeySequence::Paste);
    addSeparator();
    m_rename = action(tr("Rename"), ":/icons/rename.svg", QKeySequence(Qt::Key_F2));
    m_delete = action(tr("Delete"), ":/icons/Remove.svg", QKeySequence::Delete);
    addSeparator();
    m_selectAll = action(tr("Select All"), ":/icons/Selectall.svg", QKeySequence::SelectAll);
}

void FileExploreMenu::connectActions()
{
    connect(m_cut, &QAction::triggered, m_explorer, &FileExplore::cutSelectedFiles);
    connect(m_copy, &QAction::triggered, m_explorer, &FileExplore::copySelectedFiles);
    connect(m_paste, &QAction::triggered, m_explorer, &FileExplore::pasteFiles);
    connect(m_rename, &QAction::triggered, m_explorer, &FileExplore::renameSelectedFile);
    connect(m_delete, &QAction::triggered, m_explorer, &FileExplore::deleteSelectedFiles);
    connect(m_selectAll, &QAction::triggered, m_explorer, &FileExplore::selectAllFiles);
}

void FileExploreMenu::updateActions()
{
    const QStringList files = m_explorer->selectedFilePaths();
    m_cut->setEnabled(!files.isEmpty());
    m_copy->setEnabled(!files.isEmpty());
    m_delete->setEnabled(!files.isEmpty());
    m_rename->setEnabled(files.size() == 1);
    m_paste->setEnabled(m_explorer->canPasteFiles());
}

void FileExploreMenu::showAt(const QPoint &globalPosition)
{
    updateActions();
    exec(globalPosition);
}
