#include "OpenDatabaseDialog.h"
#include "ui_OpenDatabaseDialog.h"

#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QPushButton>

OpenDatabaseDialog::OpenDatabaseDialog(QWidget *parent) :
  QDialog(parent), ui(new Ui::OpenDatabaseDialog) {
  ui->setupUi(this);

  // Set window icon and title
  setWindowTitle(tr("Open Database"));

  // Connect signals and slots
  connect(ui->browseButton, &QPushButton::clicked, this, &OpenDatabaseDialog::on_browseButton_clicked);
  connect(ui->showPasswordCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
      ui->passwordLineEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
      });

  // Connect text changed signals to validate input
  connect(ui->filePathLineEdit, &QLineEdit::textChanged, this, &OpenDatabaseDialog::validateInput);
  connect(ui->passwordLineEdit, &QLineEdit::textChanged, this, &OpenDatabaseDialog::validateInput);

  // Disable OK button by default
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

  // Set focus to file path line edit
  ui->filePathLineEdit->setFocus();
}

OpenDatabaseDialog::~OpenDatabaseDialog() {
  delete ui;
}

QString OpenDatabaseDialog::getFilePath() const {
  return ui->filePathLineEdit->text();
}

QString OpenDatabaseDialog::getPassword() const {
  return ui->passwordLineEdit->text();
}

void OpenDatabaseDialog::on_browseButton_clicked() {
  QString filePath = QFileDialog::getOpenFileName(
      this,
      tr("Open Database"),
      QDir::homePath(),
      tr("SQLite Database (*.db);;All Files (*)")
      );

  if (!filePath.isEmpty()) {
    ui->filePathLineEdit->setText(QDir::toNativeSeparators(filePath));
    ui->passwordLineEdit->setFocus();
  }
}

void OpenDatabaseDialog::validateInput() {
  bool isValid = !ui->filePathLineEdit->text().isEmpty() && 
    !ui->passwordLineEdit->text().isEmpty();

  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(isValid);
}
