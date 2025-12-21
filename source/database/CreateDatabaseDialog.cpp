#include "./CreateDatabaseDialog.h"
#include "./ui_CreateDatabaseDialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

CreateDatabaseDialog::CreateDatabaseDialog(QWidget *parent)
  : QDialog(parent), ui(new Ui::CreateDatabaseDialog) {
  ui->setupUi(this);
  setWindowTitle(tr("Create New Database"));

  // Connect signals
  connect(ui->browseButton, &QPushButton::clicked, this, &CreateDatabaseDialog::onBrowseButtonClicked);
  connect(ui->databaseNameEdit, &QLineEdit::textChanged, this, &CreateDatabaseDialog::validateInputs);
  connect(ui->passwordEdit, &QLineEdit::textChanged, this, &CreateDatabaseDialog::validateInputs);
  connect(ui->confirmPasswordEdit, &QLineEdit::textChanged, this, &CreateDatabaseDialog::validateInputs);
  connect(ui->filePathEdit, &QLineEdit::textChanged, this, &CreateDatabaseDialog::validateInputs);

  // Set password echo mode
  ui->passwordEdit->setEchoMode(QLineEdit::Password);
  ui->confirmPasswordEdit->setEchoMode(QLineEdit::Password);

  // Disable OK button initially
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

CreateDatabaseDialog::~CreateDatabaseDialog() {
  delete ui;
}

QString CreateDatabaseDialog::getDatabaseName() const {
  return ui->databaseNameEdit->text().trimmed();
}

QString CreateDatabaseDialog::getPassword() const {
  return ui->passwordEdit->text();
}

QString CreateDatabaseDialog::getFilePath() const {
  return ui->filePathEdit->text();
}

void CreateDatabaseDialog::onBrowseButtonClicked() {
  QString defaultName = getDatabaseName();
  if (defaultName.isEmpty()) {
    defaultName = "database";
  }
  defaultName += ".db";

  QString fileName = QFileDialog::getSaveFileName(this,
      tr("Save Database File"),
      QDir::homePath() + QDir::separator() + defaultName,
      tr("SQLCipher Database (*.db);;All Files (*)"));

  if (!fileName.isEmpty()) {
    // Ensure the file has .db extension
    if (!fileName.endsWith(".db", Qt::CaseInsensitive)) {
      fileName += ".db";
    }
    ui->filePathEdit->setText(fileName);
  }
}

void CreateDatabaseDialog::validateInputs() {
  bool valid = true;
  QString error;

  // Check database name
  if (getDatabaseName().isEmpty()) {
    valid = false;
  }

  // Check passwords match and meet requirements
  if (ui->passwordEdit->text() != ui->confirmPasswordEdit->text()) {
    error = tr("Passwords do not match");
    valid = false;
  } else if (ui->passwordEdit->text().length() < 8) {
    error = tr("Password must be at least 8 characters long");
    valid = false;
  }

  // Check file path
  if (ui->filePathEdit->text().isEmpty()) {
    valid = false;
  }

  ui->errorLabel->setText(error);
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
}