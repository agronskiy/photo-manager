#ifndef SEARCHDIALOG_H_INCLUDED
#define SEARCHDIALOG_H_INCLUDED
#include "mainwindow.h"
#include "ui_searchdialog.h"

class PMSearchDialog : public QDialog, private Ui::searchDialog {
  Q_OBJECT
public:
  PMSearchDialog(QStringList *searchList, QWidget *parent = 0);

public slots:
  void startSearch();

protected:
  bool eventFilter(QObject *ob, QEvent *e);
  QStringList *searchList;
};

#endif // SEARCHDIALOG_H_INCLUDED
