#pragma once

#include <QWidget>
#include <QMap>
#include <QTreeWidgetItem>
#include <QTimer>
#include <QEvent>
#include "../database/DatabaseManager.h"

namespace Ui {
class VaultWidget;
}

class VaultWidget : public QWidget {
    Q_OBJECT

public:
    explicit VaultWidget(QWidget *parent = nullptr);
    ~VaultWidget();

    bool eventFilter(QObject* watched, QEvent* event) override;

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
    void showEntriesContextMenu(const QPoint& pos);
    void onCopyPassword();
    void updateClipboardProgress();
    void clearClipboard();

private:
    void refreshGroups();
    void loadGroupTree(int parentId, QTreeWidgetItem* parentItem);
    void loadEntries(int groupId);
    void resetInactivityTimer();

    Ui::VaultWidget *ui;
    QMap<QTreeWidgetItem*, int> m_groupMap;
    QList<DatabaseManager::Entry> m_currentEntries;
    
    QTimer* m_clipboardTimer = nullptr;
    int m_clipboardTimerValue = 0;
    QString m_lastCopiedPassword;

    QTimer* m_inactivityTimer = nullptr;
};