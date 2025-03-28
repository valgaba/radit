/* This file is part of Radit.
   Copyright 2023, Victor Algaba <victorengine@gmail.com> www.radit.org

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




#include <QDebug>


#include "widgets/treeview.h"

#include "core/filesystemmodel.h"
#include "widgets/scrollbar.h"


TreeView::TreeView(QWidget *parent):QTreeView(parent){





    this->setObjectName("TreeView"); //para qss

FileSystemModel *model = new FileSystemModel;


//model->setRootPath(QDir::homePath());


 model->setFilter(QDir::NoDotAndDotDot | QDir::Dirs );

this->setModel(model);

 this->hideColumn(1);
 this->hideColumn(2);
 this->hideColumn(3);
 this->setHeaderHidden(true);


this->setUniformRowHeights(true);

this->setVerticalScrollBar(new ScrollBar); // esta formula de añadir es nueva
this->setHorizontalScrollBar(new ScrollBar);

this->setRootIndex(model->setRootPath(QDir::currentPath()));

}



TreeView::~TreeView(){}

