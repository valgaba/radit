#ifndef TAPPLAYERMENU_H
#define TAPPLAYERMENU_H

#include "widgets/menu.h"
#include <QColor>

class TapPlayerMenu : public Menu
{
    Q_OBJECT

public:
    explicit TapPlayerMenu(QWidget *parent = nullptr);

signals:
    void addListRequested();
    void deleteListRequested();
    void renameListRequested();
    void loadPlayerRequested();
    void savePlayerRequested();
    void savePlayerAsRequested();
    void colorRequested(const QColor &color);

private:
    void setupColorPalette();
    static QIcon colorIcon(const QColor &color);
};

#endif // TAPPLAYERMENU_H
