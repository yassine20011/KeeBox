#include "VaultWidget.h"
#include "ui_VaultWidget.h"
#include "../database/DatabaseManager.h"

#include <QHeaderView>

#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>

VaultWidget::VaultWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::VaultWidget) {
    ui->setupUi(this);
    
    // UI Layout tweaks
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 4);
    
    ui->entriesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    // Context Menu
    ui->groupsTree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->groupsTree, &QTreeWidget::customContextMenuRequested, this, &VaultWidget::showGroupsContextMenu);

    // Connect signals
    connect(ui->groupsTree, &QTreeWidget::itemClicked, this, &VaultWidget::onGroupSelected);

    // Initial Load
    refreshGroups();
}

VaultWidget::~VaultWidget() {
    delete ui;
}

void VaultWidget::refreshGroups() {
    // Remember current selection ID if possible
    int selectedId = -1;
    if (QTreeWidgetItem* current = ui->groupsTree->currentItem()) {
        selectedId = m_groupMap.value(current, -1);
    }

    ui->groupsTree->clear();
    m_groupMap.clear();
    
    // Load recursively starting from root (parentId 0 in our logic means NULL in SQL)
    loadGroupTree(0, nullptr);
    
    ui->groupsTree->expandAll();

    // Restore selection
    if (selectedId != -1) {
        for (auto it = m_groupMap.begin(); it != m_groupMap.end(); ++it) {
            if (it.value() == selectedId) {
                ui->groupsTree->setCurrentItem(it.key());
                break;
            }
        }
    } else if (ui->groupsTree->topLevelItemCount() > 0) {
        // Default to first item (Root)
        QTreeWidgetItem* root = ui->groupsTree->topLevelItem(0);
        ui->groupsTree->setCurrentItem(root);
        onGroupSelected(root, 0);
    }
}

void VaultWidget::loadGroupTree(int parentId, QTreeWidgetItem* parentItem) {
    QList<DatabaseManager::Group> groups = DatabaseManager::instance().getGroups(parentId);
    
    for (const auto& group : groups) {
        QTreeWidgetItem* item;
        if (parentItem) {
            item = new QTreeWidgetItem(parentItem);
        } else {
            item = new QTreeWidgetItem(ui->groupsTree);
        }
        
        item->setText(0, group.name);
        m_groupMap[item] = group.id;
        
        // Recurse for children
        loadGroupTree(group.id, item);
    }
}

void VaultWidget::onGroupSelected(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column);
    if (!item) return;
    
    int groupId = m_groupMap.value(item, -1);
    if (groupId != -1) {
        loadEntries(groupId);
    }
}

void VaultWidget::showGroupsContextMenu(const QPoint& pos) {
    QTreeWidgetItem* item = ui->groupsTree->itemAt(pos);
    
    QMenu menu(this);
    
    if (item) {
        menu.addAction(tr("Add Subgroup"), this, &VaultWidget::onAddGroup);
        menu.addAction(tr("Edit Group"), this, &VaultWidget::onEditGroup);
        menu.addSeparator();
        menu.addAction(tr("Delete Group"), this, &VaultWidget::onDeleteGroup);
    } else {
        menu.addAction(tr("Add Group"), this, &VaultWidget::onAddGroup);
    }
    
    menu.exec(ui->groupsTree->viewport()->mapToGlobal(pos));
}

void VaultWidget::onAddGroup() {
    QTreeWidgetItem* parentItem = ui->groupsTree->currentItem();
    int parentId = parentItem ? m_groupMap.value(parentItem, 0) : 0;
    
    bool ok;
    QString name = QInputDialog::getText(this, tr("Add Group"),
                                         tr("Group name:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !name.isEmpty()) {
        DatabaseManager::instance().createGroup(name, parentId);
        refreshGroups();
    }
}

void VaultWidget::onEditGroup() {
    QTreeWidgetItem* item = ui->groupsTree->currentItem();
    if (!item) return;
    
    int groupId = m_groupMap.value(item, -1);
    if (groupId == -1) return;
    
    bool ok;
    QString name = QInputDialog::getText(this, tr("Edit Group"),
                                         tr("Group name:"), QLineEdit::Normal,
                                         item->text(0), &ok);
    if (ok && !name.isEmpty()) {
        DatabaseManager::instance().updateGroup(groupId, name);
        refreshGroups();
    }
}

void VaultWidget::onDeleteGroup() {
    QTreeWidgetItem* item = ui->groupsTree->currentItem();
    if (!item) return;
    
    int groupId = m_groupMap.value(item, -1);
    if (groupId == -1) return;
    
    // Don't delete Root if you want to protect it, but let's assume user knows what they are doing.
    // If it's the last group, maybe warn more?
    
    auto result = QMessageBox::question(this, tr("Delete Group"),
                                        tr("Are you sure you want to delete group '%1' and all its entries?").arg(item->text(0)),
                                        QMessageBox::Yes | QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        DatabaseManager::instance().deleteGroup(groupId);
        refreshGroups();
    }
}

void VaultWidget::loadEntries(int groupId) {
    ui->entriesTable->setRowCount(0);
    
    QList<DatabaseManager::Entry> entries = DatabaseManager::instance().getEntries(groupId);
    
    for (const auto& entry : entries) {
        int row = ui->entriesTable->rowCount();
        ui->entriesTable->insertRow(row);
        
        ui->entriesTable->setItem(row, 0, new QTableWidgetItem(entry.title));
        ui->entriesTable->setItem(row, 1, new QTableWidgetItem(entry.username));
        ui->entriesTable->setItem(row, 2, new QTableWidgetItem(entry.url));
    }
}