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
    void setStandbyFileActionsEnabled(bool enabled) { m_standbyFileActionsEnabled=enabled; }

signals:
    void contentDurationsChanged();
    void contentAdded();
    void contentModified();

private:
    bool m_standbyFileActionsEnabled = false;

protected:
    AudioItemMaxi *createFolderItem() override;
    bool supportsPlaybackOptionsInContextMenu() const override { return false; }
    bool supportsListOptionsInContextMenu() const override { return false; }
    void contextMenuEvent(QContextMenuEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

#endif // PLANNERCONTENTS_H
