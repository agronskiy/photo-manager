#ifndef DBSETTINGSDIALOG_H_INCLUDED
#define DBSETTINGSDIALOG_H_INCLUDED
#include <QtGui>

#include "ui_dbsettingsdialog.h"

class PMDBSettingsDialog : public QDialog, private Ui::Dialog {
  Q_OBJECT
public:
  PMDBSettingsDialog(QWidget *parent = 0);
public slots:
  void saveSettings();
};

#endif // DBSETTINGSDIALOG_H_INCLUDED
