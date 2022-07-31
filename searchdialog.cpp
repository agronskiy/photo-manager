#include "searchdialog.h"
#include <QtGui>

PMSearchDialog::PMSearchDialog(QStringList *searchList, QWidget *parent)
    : QDialog(parent) {
  setupUi(this);
  this->searchList = searchList;

  searchEdit->installEventFilter(this);

  connect(okButton, SIGNAL(clicked()), this, SLOT(startSearch()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void PMSearchDialog::startSearch() {
  QString str = searchEdit->toPlainText();
  str.remove(QRegExp("[.,!\\?-_\\+\\';:\\*\\\\\\a\\n\\r]"));
  *searchList = str.split(" ", QString::SkipEmptyParts);
  accept();
}

bool PMSearchDialog::eventFilter(QObject *ob, QEvent *e) {
  if (ob == searchEdit && e->type() == QEvent::KeyPress) {
    if ((static_cast<QKeyEvent *>(e))->key() == Qt::Key_Enter ||
        (static_cast<QKeyEvent *>(e))->key() == Qt::Key_Return) {
      okButton->click();
      return true;
    }
  }
  return QDialog::eventFilter(ob, e);
}
