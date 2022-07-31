#include "catwidget.h"
#include "mainwindow.h"

PMCatWidget::PMCatWidget(MainWindow *m, QWidget *parent) : QWidget(parent) {
  setupUi(this);

  tree = new QTreeView(this);

  QVBoxLayout *l = new QVBoxLayout;
  l->addWidget(tree, 10);
  l->addWidget(groupBox, 1);
  l->addStretch();

  setLayout(l);
  if (mWin->isClient()) {
    nameEdit->setReadOnly(true);
    newButton->setEnabled(false);
  }

  saveButton->setEnabled(false);
  removeButton->setEnabled(false);
  changeParButton->setEnabled(false);

  connect(newButton, SIGNAL(clicked()), m, SLOT(createCategory()));
  connect(removeButton, SIGNAL(clicked()), m, SLOT(removeCategory()));
  connect(changeParButton, SIGNAL(clicked()), m, SLOT(changeCatParent()));
  connect(saveButton, SIGNAL(clicked()), this, SLOT(saveChanges()));
  connect(this->treeView(), SIGNAL(clicked(QModelIndex)), this,
          SLOT(treeClicked(QModelIndex)));
}

void PMCatWidget::treeClicked(const QModelIndex &index) {
  QString catName = mWin->modelCategory()->data(index, Qt::NameRole).toString();
  int amount = mWin->modelCategory()->data(index, Qt::AmountRole).toInt();
  nameEdit->setText(catName);
  amountEdit->setText(QString::number(amount));
}

void PMCatWidget::saveChanges() {
  QString s = nameEdit->text();
  if (treeView()->selectionModel()->selectedIndexes().isEmpty()) {
    return;
  }
  QModelIndex index = treeView()->selectionModel()->selectedIndexes().first();
  if (s.trimmed().isEmpty()) {
    QString catName =
        mWin->modelCategory()->data(index, Qt::NameRole).toString();
    nameEdit->setText(catName);
    return;
  } else {
    mWin->modelCategory()->setData(index, QVariant(s), Qt::NameRole);
  }
}

void PMCatWidget::recalculateButtons() {
  if (treeView()->selectionModel()->selectedIndexes().isEmpty()) {
    nameEdit->clear();
    saveButton->setEnabled(false);
    removeButton->setEnabled(false);
    changeParButton->setEnabled(false);
  } else if (mWin->isEditor()) {
    saveButton->setEnabled(true);
    removeButton->setEnabled(true);
    changeParButton->setEnabled(true);
  }
}
