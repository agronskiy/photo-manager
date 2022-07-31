#include "dbsettingsdialog.h"
#include "pm.h"

PMDBSettingsDialog::PMDBSettingsDialog(QWidget *parent) : QDialog(parent) {
  setupUi(this);
  hostEdit->setText(pm::host);
  portEdit->setText(QString::number(pm::port));
  userEdit->setText(pm::dbUser);
  passEdit->setText(pm::dbPass);
  connect(saveButton, SIGNAL(clicked()), this, SLOT(saveSettings()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
}

void PMDBSettingsDialog::saveSettings() {
  pm::port = portEdit->text().toInt();
  pm::dbUser = userEdit->text();
  pm::dbPass = passEdit->text();
  pm::host = hostEdit->text();

  this->close();
  return;
}
