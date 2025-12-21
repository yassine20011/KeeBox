#include <QMap>
#include <QTreeWidgetItem>

namespace Ui {
class VaultWidget;
}

class VaultWidget : public QWidget {
    Q_OBJECT

public:
    explicit VaultWidget(QWidget *parent = nullptr);
    ~VaultWidget();

private slots:
    void onGroupSelected(QTreeWidgetItem* item, int column);
    void showGroupsContextMenu(const QPoint& pos);
    void onAddGroup();
    void onEditGroup();
    void onDeleteGroup();

private:
    void refreshGroups();
    void loadGroupTree(int parentId, QTreeWidgetItem* parentItem);
    void loadEntries(int groupId);

    Ui::VaultWidget *ui;
    QMap<QTreeWidgetItem*, int> m_groupMap;
};