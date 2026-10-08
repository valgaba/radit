#ifndef PLANNERCONTENTS_H
#define PLANNERCONTENTS_H

#include "widgets/ContentsPlayer.h"

class PlannerContents final : public ContentsPlayer
{
    Q_OBJECT

public:
    explicit PlannerContents(QWidget *parent = nullptr);
    AudioItemMaxi *createItem(AudioItemMaxi *item) override;
    void deleteItem(AudioItemMaxi *item) override;

signals:
    void contentDurationsChanged();

protected:
    bool supportsPlaybackOptionsInContextMenu() const override { return false; }
    bool supportsListOptionsInContextMenu() const override { return false; }
    void contextMenuEvent(QContextMenuEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

#endif // PLANNERCONTENTS_H
