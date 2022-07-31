#include <QtGui>

#include "dialogs.h"
#include "invmodel.h"
#include "mainwindow.h"

PMNewCatDialog::PMNewCatDialog(MainWindow *parent) : QDialog(parent) {
  parentEdit = new QTreeView(this);
  QItemSelectionModel *selections =
      new QItemSelectionModel(parent->modelCategory());
  parentEdit->setModel(parent->modelCategory());
  parentEdit->setSelectionModel(selections);
  parentEdit->setUniformRowHeights(true);
  parentEdit->hideColumn(1);
  parentEdit->setSelectionBehavior(QAbstractItemView::SelectItems);
  parentEdit->setSelectionMode(QAbstractItemView::SingleSelection);

  nameLabel = new QLabel(tr("New category "));
  nameEdit = new QLineEdit;
  nameLabel->setBuddy(nameEdit);

  parentEditLabel = new QLabel(tr("Select parent category (leave unselected "
                                  "<br> to create an upper-level one)"));
  parentEditLabel->setBuddy(parentEdit);

  createButton = new QPushButton(tr("Create"));
  createButton->setDefault(true);
  createButton->setEnabled(false);

  cancelButton = new QPushButton(tr("Cancel"));

  connect(nameEdit, SIGNAL(textChanged(QString)), this,
          SLOT(recalculateButton()));
  connect(createButton, SIGNAL(clicked()), this, SLOT(createCategory()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));

  QVBoxLayout *leftTopLayout = new QVBoxLayout;
  leftTopLayout->addWidget(parentEditLabel);
  leftTopLayout->addWidget(parentEdit);
  QHBoxLayout *leftBotLayout = new QHBoxLayout;
  leftBotLayout->addWidget(nameLabel);
  leftBotLayout->addWidget(nameEdit);
  leftTopLayout->addLayout(leftBotLayout);
  QVBoxLayout *rightLayout = new QVBoxLayout;
  rightLayout->addWidget(createButton);
  rightLayout->addWidget(cancelButton);
  rightLayout->addStretch();

  QGridLayout *mainLayout = new QGridLayout;
  mainLayout->addLayout(leftTopLayout, 0, 0);
  mainLayout->addLayout(rightLayout, 0, 1);
  setLayout(mainLayout);
  setWindowTitle(tr("Create new category"));
  this->resize(this->width(), (int)(mWin->height() * 0.8));
}

void PMNewCatDialog::recalculateButton() {
  if (nameEdit->text().isEmpty()) {
    createButton->setEnabled(false);
  } else {
    createButton->setEnabled(true);
  }
}

void PMNewCatDialog::createCategory() {
  MainWindow *mparent = static_cast<MainWindow *>(parent());
  if (parentEdit->selectionModel()->selectedIndexes().isEmpty()) {
    mparent->modelCategory()->addNewCategoryDB(nameEdit->text(), QModelIndex());
  } else {
    mparent->modelCategory()->addNewCategoryDB(
        nameEdit->text(),
        parentEdit->selectionModel()->selectedIndexes().at(0));
  }
  this->close();
}

/*########################################################################################*/
/*########################################################################################*/
PMChangeCatParDialog::PMChangeCatParDialog(MainWindow *parent)
    : QDialog(parent) {
  parentEdit = new QTreeView(this);
  QItemSelectionModel *selections =
      new QItemSelectionModel(parent->modelCategory());
  parentEdit->setModel(parent->modelCategory());
  parentEdit->setSelectionModel(selections);
  parentEdit->setUniformRowHeights(true);
  parentEdit->hideColumn(1);
  parentEdit->setSelectionBehavior(QAbstractItemView::SelectItems);
  parentEdit->setSelectionMode(QAbstractItemView::SingleSelection);

  parentEditLabel = new QLabel(tr("Select parent category (leave unselected "
                                  "<br> to set an upper-level one)"));
  parentEditLabel->setBuddy(parentEdit);

  createButton = new QPushButton(tr("Change"));
  createButton->setDefault(true);

  cancelButton = new QPushButton(tr("Cancel"));

  connect(createButton, SIGNAL(clicked()), this, SLOT(changeParCategory()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));

  QVBoxLayout *leftTopLayout = new QVBoxLayout;
  leftTopLayout->addWidget(parentEditLabel);
  leftTopLayout->addWidget(parentEdit);
  QVBoxLayout *rightLayout = new QVBoxLayout;
  rightLayout->addWidget(createButton);
  rightLayout->addWidget(cancelButton);
  rightLayout->addStretch();

  QGridLayout *mainLayout = new QGridLayout;
  mainLayout->addLayout(leftTopLayout, 0, 0);
  mainLayout->addLayout(rightLayout, 0, 1);
  setLayout(mainLayout);
  setWindowTitle(tr("Change parent category"));
  this->resize(this->width(), (int)(mWin->height() * 0.8));
}

void PMChangeCatParDialog::changeParCategory() {
  MainWindow *mparent = static_cast<MainWindow *>(parent());
  QModelIndex currCatIndex =
      mWin->catSelectionModel()->selectedIndexes().first();
  if (parentEdit->selectionModel()->selectedIndexes().isEmpty()) {
    mparent->modelCategory()->setCategoryParentDB(currCatIndex, QModelIndex());
  } else {
    if (currCatIndex ==
        parentEdit->selectionModel()->selectedIndexes().first()) {
      QMessageBox::warning(0, tr("Bad selection!"),
                           tr("Can't make a category be it's parent"));
      return;
    } else {
      mparent->modelCategory()->setCategoryParentDB(
          currCatIndex,
          parentEdit->selectionModel()->selectedIndexes().first());
    }
  }
  this->close();
}
/*########################################################################################*/
/*########################################################################################*/

PMRemoveCatDialog::PMRemoveCatDialog(MainWindow *parent) : QDialog(parent) {
  catEdit = new QTreeView(this);
  QItemSelectionModel *selections =
      new QItemSelectionModel(parent->modelCategory());
  catEdit->setModel(parent->modelCategory());
  catEdit->setSelectionModel(selections);
  catEdit->setUniformRowHeights(true);
  catEdit->hideColumn(1);
  catEdit->setSelectionBehavior(QAbstractItemView::SelectItems);
  catEdit->setSelectionMode(QAbstractItemView::ExtendedSelection);

  catEditLabel = new QLabel(tr("Select one or more categories to "
                               "remove<br>(use Ctrl key to select multiple)"));
  catEditLabel->setBuddy(catEdit);

  removeButton = new QPushButton(tr("Remove"));
  removeButton->setDefault(true);
  removeButton->setEnabled(false);

  cancelButton = new QPushButton(tr("Cancel"));

  connect(selections, SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          this, SLOT(recalculateButton()));
  connect(removeButton, SIGNAL(clicked()), this, SLOT(removeCategories()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));

  QVBoxLayout *leftTopLayout = new QVBoxLayout;
  leftTopLayout->addWidget(catEditLabel);
  leftTopLayout->addWidget(catEdit);

  QVBoxLayout *rightLayout = new QVBoxLayout;
  rightLayout->addWidget(removeButton);
  rightLayout->addWidget(cancelButton);
  rightLayout->addStretch();

  QGridLayout *mainLayout = new QGridLayout;
  mainLayout->addLayout(leftTopLayout, 0, 0);
  mainLayout->addLayout(rightLayout, 0, 1);
  setLayout(mainLayout);
  setWindowTitle(tr("Remove categories"));
}

void PMRemoveCatDialog::recalculateButton() {
  if (catEdit->selectionModel()->selectedIndexes().isEmpty()) {
    removeButton->setEnabled(false);
  } else {
    removeButton->setEnabled(true);
  }
}

void PMRemoveCatDialog::removeCategories() {
  QModelIndexList indexList = catEdit->selectionModel()->selectedIndexes();
  QList<quint64> idList;

  MainWindow *mparent = static_cast<MainWindow *>(parent());
  for (int i = 0; i < indexList.count(); i++) {
    quint64 id =
        mparent->modelCategory()->data(indexList.at(i), Qt::CatIdRole).toUInt();
    idList.append(id);
  }

  for (int i = 0; i < idList.count(); i++) {
    QModelIndex indexToRemove =
        mparent->modelCategory()->findCategory(idList.at(i));
    if (indexToRemove.isValid()) {
      mparent->modelCategory()->removeCategoryDB(indexToRemove);
    }
  }
  close();
}

/*########################################################################################*/
/*########################################################################################*/

PMShowImageDialog::PMShowImageDialog(const QPixmap &pixmap, const QString &cap,
                                     QWidget *parent)
    : QDialog(parent) {
  QHBoxLayout *layout = new QHBoxLayout;
  picLabel = new QLabel;
  picLabel->setPixmap(pixmap);
  layout->addWidget(picLabel);
  setLayout(layout);
  this->setWindowTitle(cap + " :: " + pm::progName);
}

/*#########################################################################################*/
/*#########################################################################################*/

PMViewerSettingsDialog::PMViewerSettingsDialog(QWidget *parent)
    : QDialog(parent) {
  setupUi(this);
  widthEdit->setText(QString::number(pm::itemWidth));
  heightEdit->setText(QString::number(pm::itemHeight));
  colNumEdit->setText(QString::number(pm::columnNum));
  connect(saveButton, SIGNAL(clicked()), this, SLOT(saveSettings()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
}

void PMViewerSettingsDialog::saveSettings() {
  QRegExp rx("^\\s*\\d+\\s*$");
  if (!(rx.exactMatch(widthEdit->text()) && rx.exactMatch(heightEdit->text()) &&
        rx.exactMatch(colNumEdit->text()))) {
    QMessageBox::warning(0, tr("Invalid input"),
                         tr("Please enter integers in all fields"));
  } else {
    pm::itemWidth = widthEdit->text().toInt();
    pm::itemHeight = heightEdit->text().toInt();
    pm::columnNum = colNumEdit->text().toInt();

    emit updatedVals();
    this->close();
  }

  return;
}

/*#########################################################################################*/
/*#########################################################################################*/

PMCacheSettingsDialog::PMCacheSettingsDialog(PMCache *cache, QWidget *parent)
    : QDialog(parent) {
  setupUi(this);
  cacheNumEdit->setText(QString::number(pm::cacheLim));
  dirEdit->setText(pm::cacheDir);
  this->cache = cache;
  connect(saveButton, SIGNAL(clicked()), this, SLOT(saveSettings()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
}

void PMCacheSettingsDialog::saveSettings() {
  QRegExp rx("^\\s*\\d+\\s*$");
  QString path;
  QDir dir;
  if (QDir::isRelativePath(dirEdit->text())) {
    path = qApp->applicationDirPath() + QDir::separator() + dirEdit->text();
  }
  if (!(rx.exactMatch(cacheNumEdit->text()) && dir.cd(path)) ||
      dirEdit->text().isEmpty()) {
    QMessageBox::warning(0, tr("Invalid input"),
                         tr("Please enter valid data (cache limit - integer, "
                            "cache directory - a valid directory)"));
  } else {
    pm::cacheLim = cacheNumEdit->text().toInt();
    pm::cacheDir = dirEdit->text();

    cache->clear();
    cache->setLimit(pm::cacheLim);
    cache->setCacheDir(pm::cacheDir);
    this->close();
  }

  return;
}

/*#########################################################################################*/
/*#########################################################################################*/
PMAddInvDialog::PMAddInvDialog(PMInv *inv, bool *ok, QString *dir,
                               QString *commonDescription, int *createInv,
                               int *depth, QWidget *parent)
    : QDialog(parent) {

  setupUi(this);
  headerEdit_2->setReadOnly(true);
  comboBox->insertItems(0, PMInv::typeList);
  colouredCBox->insertItems(0, PMInv::colList);

  this->inv = inv;
  this->ok = ok;
  this->dir = dir;
  this->commonDescription = commonDescription;
  this->createInv = createInv;
  this->depth = depth;

  connect(maxDepthEdit, SIGNAL(valueChanged(int)), this, SLOT(enableBox(int)));
  connect(okButton, SIGNAL(clicked()), this, SLOT(startProcess()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelProcess()));
  connect(dirButton, SIGNAL(clicked()), this, SLOT(showFolderDialog()));
}

void PMAddInvDialog::cancelProcess() {
  *ok = false;
  this->close();
}

void PMAddInvDialog::showFolderDialog() {
  QString text = QFileDialog::getExistingDirectory(
      this, tr("Select the directory from which you wish to add a folder"),
      QString(), QFileDialog::ShowDirsOnly);
  if (!text.isEmpty()) {
    dirEdit->setText(text);
  }
}

void PMAddInvDialog::enableBox(int n) {
  if (n < 1) {
    newInvBox->setEnabled(false);
  } else {
    newInvBox->setEnabled(true);
  }
}

void PMAddInvDialog::startProcess() {
  QDir d;
  if (!d.exists(dirEdit->text())) {
    QMessageBox::warning(this, tr("Directory not set!"),
                         tr("Please set the directory field"));
    return;
  }

  if (invEdit->text().trimmed().isEmpty()) {
    QMessageBox::warning(this, tr("Name not set!"), tr("Please type name"));
    return;
  }

  if (headerEdit->toPlainText().trimmed().isEmpty()) {
    QMessageBox::warning(this, tr("Header not set!"),
                         tr("Please type the header"));
    return;
  }

  inv->header = headerEdit->toPlainText();
  inv->author = authorEdit->text();
  inv->name = invEdit->text();
  inv->internal = (forUsBox->checkState() == Qt::Checked ? true : false);
  inv->coloured = colouredCBox->currentIndex();
  inv->date = dateEdit->date();
  inv->type = comboBox->currentIndex();
  inv->comments = commentsEdit->toPlainText();

  *dir = dirEdit->text();
  *commonDescription = descriptionEdit->toPlainText();
  *depth = maxDepthEdit->value();
  *createInv = newInvBox->checkState();
  *ok = true;
  this->close();
}
