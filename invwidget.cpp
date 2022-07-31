#include <QtCore>
#include <QtGui>
#include <QtSql>

#include "invmodel.h"
#include "invwidget.h"
#include "mainwindow.h"
#include "pm.h"

PMInvWidget::PMInvWidget(QWidget *parent) : QWidget(parent) {
  setupUi(this);
  comboBox->insertItems(0, PMInv::typeList);
  colouredCBox->insertItems(0, PMInv::colList);

  tree = new QTreeView(this);
  QVBoxLayout *l = new QVBoxLayout;
  l->addWidget(tree, 10);
  l->addWidget(groupBox, 1);
  l->addStretch();

  saveButton->setEnabled(false);

  setLayout(l);
  if (pm::client) {
    comboBox->setEnabled(false);
    colouredCBox->setEnabled(false);
    authorEdit->setReadOnly(true);
    headerEdit->setReadOnly(true);
    nameEdit->setReadOnly(true);
    dateEdit->setReadOnly(true);
    forUsBox->setEnabled(false);
    saveButton->setEnabled(false);
    commentsEdit->setReadOnly(true);
  }
  connect(saveButton, SIGNAL(clicked()), this, SLOT(saveSettings()));
  connect(tree, SIGNAL(clicked(QModelIndex)), this,
          SLOT(showInfo(QModelIndex)));
}

void PMInvWidget::saveSettings() {
  currInv.author = authorEdit->text();
  if (!headerEdit->toPlainText().trimmed().isEmpty()) {
    currInv.header = headerEdit->toPlainText();
  }
  if (nameEdit->text().trimmed().isEmpty()) {
    nameEdit->setText(tr("Enter name here!"));
    return;
  }
  currInv.name = nameEdit->text();
  currInv.date = dateEdit->date();
  currInv.type = (PMInv::InvType)comboBox->currentIndex();
  currInv.internal = (forUsBox->checkState() == Qt::Checked ? true : false);
  currInv.coloured = colouredCBox->currentIndex();
  currInv.comments = commentsEdit->toPlainText();
  QSqlRecord rec = PMInv::invToSqlRecord(currInv);

  QSqlQuery q;
  q.prepare(
      "UPDATE pm_inv SET name=:name, author=:author, header=:header, coloured=:coloured, internal_usage=:internal_usage, \
   type=:type, date=:date, comments=:comments WHERE id=" +
      QString::number(currInv.id));
  q.bindValue(":name", rec.value("name"));
  q.bindValue(":author", rec.value("author"));
  q.bindValue(":header", rec.value("header"));
  q.bindValue(":coloured", rec.value("coloured"));
  q.bindValue(":internal_usage", rec.value("internal_usage"));
  q.bindValue(":type", rec.value("type"));
  q.bindValue(":date", rec.value("date"));
  q.bindValue(":comments", rec.value("comments"));

  q.exec();
  mWin->modelInv()->setQuery(mWin->invQuery);
}

void PMInvWidget::showInfo(QModelIndex curr) {
  QSqlRecord rec = mWin->modelInv()->record(curr.row());
  currInv = PMInv::sqlRecordToInv(rec);
  comboBox->setCurrentIndex(currInv.type);
  colouredCBox->setCurrentIndex(currInv.coloured);
  authorEdit->setText(currInv.author);
  nameEdit->setText(currInv.name);
  headerEdit->setText(currInv.header);
  dateEdit->setDate(currInv.date);
  forUsBox->setCheckState((currInv.internal ? Qt::Checked : Qt::Unchecked));
  commentsEdit->setText(currInv.comments);
}

void PMInvWidget::recalculateButton() {
  if (tree->selectionModel()->selectedIndexes().isEmpty()) {
    saveButton->setEnabled(false);
  } else if (mWin->isEditor()) {
    saveButton->setEnabled(true);
  }
}
