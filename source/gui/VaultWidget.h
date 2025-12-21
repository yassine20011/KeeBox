#include <QMap>
#include <QTreeWidgetItem>
#include "../database/DatabaseManager.h"

namespace Ui {
class VaultWidget;
}

class VaultWidget : public QWidget {
    Q_OBJECT

public:
    explicit VaultWidget(QWidget *parent = nullptr);
    ~VaultWidget();

signals:
    void lockRequested();

private slots:
    void onGroupSelected(QTreeWidgetItem* item, int column);
    void showGroupsContextMenu(const QPoint& pos);
    void onAddGroup();
    void onEditGroup();
    void onDeleteGroup();
    void onAddEntry();
    void onEditEntry();
    void onDeleteEntry();
    void onLockDatabase();
    void onSearchTextChanged(const QString& text);

private:
    void refreshGroups();
    void loadGroupTree(int parentId, QTreeWidgetItem* parentItem);
    void loadEntries(int groupId);

    Ui::VaultWidget *ui;
    QMap<QTreeWidgetItem*, int> m_groupMap;
    QList<DatabaseManager::Entry> m_currentEntries;
};