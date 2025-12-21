#include "EntryDialog.h"
#include "ui_EntryDialog.h"
#include <QPushButton>

EntryDialog::EntryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EntryDialog)
{
    ui->setupUi(this);

    connect(ui->showPasswordCheck, &QCheckBox::toggled, this, &EntryDialog::onShowPasswordToggled);
    connect(ui->titleEdit, &QLineEdit::textChanged, this, &EntryDialog::validateInput);

    validateInput();
}

EntryDialog::~EntryDialog()
{
    delete ui;
}

void EntryDialog::setEntry(const DatabaseManager::Entry& entry)
{
    m_id = entry.id;
    m_groupId = entry.groupId;
    ui->titleEdit->setText(entry.title);
    ui->usernameEdit->setText(entry.username);
    ui->passwordEdit->setText(entry.password);
    ui->urlEdit->setText(entry.url);
    ui->notesEdit->setPlainText(entry.notes);
}

DatabaseManager::Entry EntryDialog::getEntry() const
{
    DatabaseManager::Entry entry;
    entry.id = m_id;
    entry.groupId = m_groupId;
    entry.title = ui->titleEdit->text();
    entry.username = ui->usernameEdit->text();
    entry.password = ui->passwordEdit->text();
    entry.url = ui->urlEdit->text();
    entry.notes = ui->notesEdit->toPlainText();
    return entry;
}

void EntryDialog::onShowPasswordToggled(bool checked)
{
    ui->passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
}

void EntryDialog::validateInput()
{
    bool isValid = !ui->titleEdit->text().trimmed().isEmpty();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(isValid);
}
