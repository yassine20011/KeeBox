#include "VaultWidget.h"
#include "ui_VaultWidget.h"
#include "EntryDialog.h"
#include "../database/DatabaseManager.h"

#include <QHeaderView>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>

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

    // Toolbar Connections
    connect(ui->lockDatabaseButton, &QToolButton::clicked, this, &VaultWidget::onLockDatabase);
    connect(ui->addEntryButton, &QToolButton::clicked, this, &VaultWidget::onAddEntry);
    connect(ui->editEntryButton, &QToolButton::clicked, this, &VaultWidget::onEditEntry);
    connect(ui->deleteEntryButton, &QToolButton::clicked, this, &VaultWidget::onDeleteEntry);
    
    // Search Connection
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &VaultWidget::onSearchTextChanged);
    
    // Table interactions
    ui->entriesTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->entriesTable, &QTableWidget::customContextMenuRequested, this, &VaultWidget::showEntriesContextMenu);
    connect(ui->entriesTable, &QTableWidget::itemDoubleClicked, this, [this](QTableWidgetItem*){ onEditEntry(); });

    // Clipboard Timer
    m_clipboardTimer = new QTimer(this);
    m_clipboardTimer->setInterval(100);
    connect(m_clipboardTimer, &QTimer::timeout, this, &VaultWidget::updateClipboardProgress);

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

void VaultWidget::onAddEntry() {
    QTreeWidgetItem* groupItem = ui->groupsTree->currentItem();
    if (!groupItem) {
        QMessageBox::warning(this, tr("No Group Selected"), tr("Please select a group first."));
        return;
    }
    
    int groupId = m_groupMap.value(groupItem, -1);
    
    EntryDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        DatabaseManager::Entry entry = dialog.getEntry();
        entry.groupId = groupId;
        DatabaseManager::instance().createEntry(entry);
        loadEntries(groupId);
    }
}

void VaultWidget::onEditEntry() {
    int row = ui->entriesTable->currentRow();
    if (row < 0 || row >= m_currentEntries.size()) {
        return;
    }
    
    DatabaseManager::Entry entry = m_currentEntries.at(row);
    EntryDialog dialog(this);
    dialog.setEntry(entry);
    
    if (dialog.exec() == QDialog::Accepted) {
        DatabaseManager::Entry updatedEntry = dialog.getEntry();
        DatabaseManager::instance().updateEntry(updatedEntry);
        loadEntries(entry.groupId);
    }
}

void VaultWidget::onDeleteEntry() {
    int row = ui->entriesTable->currentRow();
    if (row < 0 || row >= m_currentEntries.size()) {
        return;
    }
    
    DatabaseManager::Entry entry = m_currentEntries.at(row);
    
    auto result = QMessageBox::question(this, tr("Delete Entry"),
                                         tr("Are you sure you want to delete entry '%1'?").arg(entry.title),
                                         QMessageBox::Yes | QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        DatabaseManager::instance().deleteEntry(entry.id);
        loadEntries(entry.groupId);
    }
}

void VaultWidget::onLockDatabase() {
    DatabaseManager::instance().closeDatabase();
    emit lockRequested();
}

void VaultWidget::onSearchTextChanged(const QString& text) {
    if (text.isEmpty()) {
        // Return to group view
        QTreeWidgetItem* current = ui->groupsTree->currentItem();
        if (current) {
            onGroupSelected(current, 0);
        } else {
            ui->entriesTable->setRowCount(0);
        }
        return;
    }
    
    ui->entriesTable->setRowCount(0);
    m_currentEntries = DatabaseManager::instance().searchEntries(text);
    
    for (const auto& entry : m_currentEntries) {
        int row = ui->entriesTable->rowCount();
        ui->entriesTable->insertRow(row);
        
        ui->entriesTable->setItem(row, 0, new QTableWidgetItem(entry.title));
        ui->entriesTable->setItem(row, 1, new QTableWidgetItem(entry.username));
        ui->entriesTable->setItem(row, 2, new QTableWidgetItem(entry.url));
    }
}

void VaultWidget::showEntriesContextMenu(const QPoint& pos) {
    QTableWidgetItem* item = ui->entriesTable->itemAt(pos);
    if (!item) return;

    ui->entriesTable->setCurrentItem(item); // Ensure the right-clicked row is selected
    
    QMenu menu(this);
    menu.addAction(tr("Copy Password"), this, &VaultWidget::onCopyPassword);
    menu.addSeparator();
    menu.addAction(tr("Edit Entry"), this, &VaultWidget::onEditEntry);
    menu.addAction(tr("Delete Entry"), this, &VaultWidget::onDeleteEntry);

    menu.exec(ui->entriesTable->viewport()->mapToGlobal(pos));
}

void VaultWidget::onCopyPassword() {
    int row = ui->entriesTable->currentRow();
    if (row < 0 || row >= m_currentEntries.size()) return;

    const auto& entry = m_currentEntries.at(row);
    m_lastCopiedPassword = entry.password;

    QApplication::clipboard()->setText(m_lastCopiedPassword);

    // Start 10s timer (10000ms)
    m_clipboardTimerValue = 10000;
    ui->clipboardProgressBar->setMaximum(10000);
    ui->clipboardProgressBar->setValue(10000);
    ui->clipboardProgressBar->setVisible(true);
    ui->clipboardStatusLabel->setText(tr("Clearing the clipboard in 10 seconds..."));
    ui->clipboardStatusLabel->setVisible(true);
    m_clipboardTimer->start();
}

void VaultWidget::updateClipboardProgress() {
    m_clipboardTimerValue -= 100;
    if (m_clipboardTimerValue <= 0) {
        clearClipboard();
    } else {
        ui->clipboardProgressBar->setValue(m_clipboardTimerValue);
        int seconds = (m_clipboardTimerValue + 999) / 1000; // Round up
        ui->clipboardStatusLabel->setText(tr("Clearing the clipboard in %1 seconds...").arg(seconds));
    }
}

void VaultWidget::clearClipboard() {
    m_clipboardTimer->stop();
    ui->clipboardProgressBar->setVisible(false);
    ui->clipboardStatusLabel->setVisible(false);

    if (QApplication::clipboard()->text() == m_lastCopiedPassword) {
        QApplication::clipboard()->clear();
    }
    m_lastCopiedPassword.clear();
}

void VaultWidget::loadEntries(int groupId) {
    ui->entriesTable->setRowCount(0);
    m_currentEntries = DatabaseManager::instance().getEntries(groupId);
    
    for (const auto& entry : m_currentEntries) {
        int row = ui->entriesTable->rowCount();
        ui->entriesTable->insertRow(row);
        
        ui->entriesTable->setItem(row, 0, new QTableWidgetItem(entry.title));
        ui->entriesTable->setItem(row, 1, new QTableWidgetItem(entry.username));
        ui->entriesTable->setItem(row, 2, new QTableWidgetItem(entry.url));
    }
}