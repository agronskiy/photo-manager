#include <QtSql>

#include "dbimagemodel.h"
#include "mainwindow.h"
#include "threads.h"

PMDBImageModel::PMDBImageModel(QObject *parent) : QAbstractListModel(parent) {

  currCatId = 0;
  type = None;
  currSortType = Id;

  sortByDateAction = new QAction(tr("By date shot"), this);
  sortByIdAction = new QAction(tr("By date uploaded"), this);
  sortByNameAction = new QAction(tr("By file name"), this);
  sortByHeaderAction = new QAction(tr("By header"), this);
  sortByIdAction->setCheckable(true);
  sortByDateAction->setCheckable(true);
  sortByHeaderAction->setCheckable(true);
  sortByNameAction->setCheckable(true);

  sortGroup = new QActionGroup(this);
  sortGroup->addAction(sortByIdAction);
  sortGroup->addAction(sortByNameAction);
  sortGroup->addAction(sortByHeaderAction);
  sortGroup->addAction(sortByDateAction);
  sortGroup->setEnabled(false);

  connect(sortByDateAction, SIGNAL(triggered()), this, SLOT(sortDate()));
  connect(sortByIdAction, SIGNAL(triggered()), this, SLOT(sortId()));
  connect(sortByNameAction, SIGNAL(triggered()), this, SLOT(sortName()));
  connect(sortByHeaderAction, SIGNAL(triggered()), this, SLOT(sortHeader()));

  setCheckedAction(currSortType);
}

PMDBImageModel::~PMDBImageModel() { clearModel(); }

QModelIndex PMDBImageModel::index(int row, int column,
                                  const QModelIndex &parent) const {
  if (row < 0 || row >= rowCount()) {
    return QModelIndex();
  } else
    return createIndex(row, 0, images.at(row));
}

QModelIndex PMDBImageModel::parent(const QModelIndex &child) const {
  return QModelIndex();
}

int PMDBImageModel::rowCount(const QModelIndex &parent) const {
  return images.count();
}

QVariant PMDBImageModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  if (role == Qt::PreviewRole) {
    QByteArray arr;
    QBuffer buf(&arr);
    buf.open(QIODevice::WriteOnly);
    imageFromIndex(index)->preview.save(&buf, "JPG");
    buf.close();
    return arr;
  } else if (role == Qt::NameRole) {
    return imageFromIndex(index)->name;
  } else if (role == Qt::HeaderRole) {
    return imageFromIndex(index)->header;
  } else if (role == Qt::DescriptionRole) {
    return imageFromIndex(index)->description;
  } else if (role == Qt::CdRole) {
    return imageFromIndex(index)->cd;
  } else if (role == Qt::DateRole) {
    return imageFromIndex(index)->date;
  } else if (role == Qt::InventaryRole) {
    return imageFromIndex(index)->inventary;
  } else if (role == Qt::LocationRole) {
    return imageFromIndex(index)->location;
  } else if (role == Qt::PidRole) {
    return qVariantFromQList(imageFromIndex(index)->pid);
  } else if (role == Qt::DisplayRole) {
    return imageFromIndex(index)->name;
  } else if (role == Qt::IdRole) {
    return imageFromIndex(index)->id;
  } else if (role == Qt::WidthRole) {
    return imageFromIndex(index)->width;
  } else if (role == Qt::HeightRole) {
    return imageFromIndex(index)->height;
  } else if (role == Qt::OrigWidthRole) {
    return imageFromIndex(index)->origWidth;
  } else if (role == Qt::OrigHeightRole) {
    return imageFromIndex(index)->origHeight;
  } else if (role == Qt::OrigSizeRole) {
    return imageFromIndex(index)->origSize;
  } else if (role == Qt::DpiRole) {
    int x = imageFromIndex(index)->xDensityDpi;
    int y = imageFromIndex(index)->yDensityDpi;
    QString res = "";
    if (x * y == 0) {
      res = "N/A";
    } else if (x * y < 0) {
      res = QString::number(qAbs(x)) + ":" + QString::number(qAbs(y));
    } else if (x < 0) {
      res =
          QString::number(qAbs(x)) + " x " + QString::number(qAbs(y)) + " dpcm";
    } else {
      res =
          QString::number(qAbs(x)) + " x " + QString::number(qAbs(y)) + " dpi";
    }
    return res;
  } else
    return QVariant();
}

QModelIndex PMDBImageModel::findImage(quint64 id) const {
  for (int i = 0; i < rowCount(); i++) {
    if (id == data(index(i), Qt::IdRole).toUInt())
      return index(i);
  }

  return QModelIndex();
}

PMDBImage *PMDBImageModel::imageFromIndex(const QModelIndex &index) const {
  if (index.isValid()) {
    return static_cast<PMDBImage *>(index.internalPointer());
  } else {
    return static_cast<PMDBImage *>(0);
  }
}

void PMDBImageModel::clearModel() {
  qDeleteAll(images);
  images.clear();
  QPixmapCache::clear();
  reset();
}

void PMDBImageModel::loadCategoryContents(const QModelIndex &catIndex,
                                          PMDBImageModel::SortType sortType) {

  clearModel();
  mWin->statusBar()->showMessage(tr("Loading images... Please wait.."));
  mWin->setEnabled(false);

  quint64 catId;
  if (catIndex.isValid()) {
    catId = catIndex.model()->data(catIndex, Qt::CatIdRole).toUInt();
  } else {
    catId = 0;
  }

  currCatId = catId;
  type = Category;

  QSqlQuery query;
  if (sortType == Current) {
    sortType = currSortType;
  }

  sortGroup->setEnabled(true);
  setCheckedAction(sortType);

  QString str = " id DESC";

  if (sortType == Date) {
    str = " date DESC ";
  } else if (sortType == Header) {
    str = " header ";
  } else if (sortType == Name) {
    str = " name ";
  }
  bool ok = query.exec("SELECT id FROM pm_images WHERE pid~'x" +
                       QString::number(catId) + "x' ORDER BY " + str);
  if (!ok) {
    QMessageBox::warning(0, tr("Database error"), query.lastError().text());
  }
  while (query.next()) {
    quint64 id = query.value(0).toUInt();
    addImageToModel(id);
  }
  mWin->setEnabled(true);
  mWin->statusBar()->clearMessage();
}

QModelIndex PMDBImageModel::addImageToModel(quint64 id) {
  if (findImage(id).isValid()) {
    return QModelIndex();
  }
  PMDBImage *image = MainWindow::loadImageFromDB(id /*, pm::WithPreview*/);
  if (image != static_cast<PMDBImage *>(0)) {
    images.append(image);
    reset();
    return index(images.count() - 1);
  }
  return QModelIndex();
}

QVariant PMDBImageModel::qVariantFromQList(const QList<quint64> &list) {
  QString result = "";
  for (int i = 0; i < list.count(); i++) {
    result += "x";
    result += QString::number(list.at(i));
    result += "x ";
  }
  return result;
}

QList<quint64> PMDBImageModel::qListFromQVariant(const QVariant &var) {
  QList<quint64> result;
  if (var.type() != QVariant::String) {
    return result;
  }
  QStringList elements =
      var.toString().remove(" ").split("x", QString::SkipEmptyParts);
  for (int i = 0; i < elements.count(); i++) {
    result.append(elements.at(i).toUInt());
  }
  return result;
}

bool PMDBImageModel::removeImageFromModelDB(const QModelIndex &indexToDelete) {
  if (!indexToDelete.isValid()) {
    return false;
  }

  bool ok = true;
  PMDBImage *image = imageFromIndex(indexToDelete);
  ok = MainWindow::removeImageFromDB(*image);

  int ind = images.indexOf(image);
  images.removeAt(ind);

  reset();
  return ok;
}

void PMDBImageModel::removeImageFromModel(const QModelIndex &indexToDelete) {
  if (!indexToDelete.isValid()) {
    return;
  }

  PMDBImage *image = imageFromIndex(indexToDelete);

  int ind = images.indexOf(image);
  images.removeAt(ind);

  reset();
}
bool PMDBImageModel::setData(const QModelIndex &index, const QVariant &value,
                             int role) {
  if (!index.isValid()) {
    return false;
  }
  PMDBImage *image = imageFromIndex(index);
  if (role == Qt::PreviewRole) {
    image->preview.loadFromData(value.toByteArray(), "JPG");
  } else if (role == Qt::NameRole) {
    image->name = value.toString();
  } else if (role == Qt::HeaderRole) {
    image->header = value.toString();
  } else if (role == Qt::DescriptionRole) {
    image->description = value.toString();
  } else if (role == Qt::CdRole) {
    image->cd = value.toInt();
  } else if (role == Qt::DateRole) {
    image->date = value.toDate();
  } else if (role == Qt::InventaryRole) {
    image->inventary = value.toUInt();
  } else if (role == Qt::LocationRole) {
    image->location = value.toString();
  } else if (role == Qt::PidRole) {
    image->pid = qListFromQVariant(value);
    //    if(type == Category && !image->pid.contains(currCatId)){
    //      bool ok = MainWindow::updateImageInDB(*image);
    //      emit selectImage(index, QItemSelectionModel::Deselect);
    //      removeImageFromModel(index);
    //      return ok;
    //    }
  } else if (role == Qt::WidthRole) {
    image->width = value.toInt();
  } else if (role == Qt::HeightRole) {
    image->height = value.toInt();
  } else if (role == Qt::OrigWidthRole) {
    image->origWidth = value.toInt();
  } else if (role == Qt::OrigHeightRole) {
    image->origHeight = value.toInt();
  } else
    return 0;
  emit dataChanged(index, index);
  if (role != Qt::PreviewRole)
    return MainWindow::updateImageInDB(*image);
  else
    return MainWindow::updateImageInDB(*image, pm::WithPreview);
}

void PMDBImageModel::reloadContents(PMDBImageModel::SortType sortType) {
  if (type == Category) {
    loadCategoryContents(mWin->modelCategory()->findCategory(currCatId),
                         sortType);
  } else if (type == SearchResult) {
    loadSearchResults();
  } else if (type == Inventary) {
    reloadInventaryContents(sortType);
  }
}

void PMDBImageModel::loadSearchResults() {
  clearModel();

  type = SearchResult;
  sortGroup->setEnabled(false);

  foreach (quint64 id, searchResults) {
    addImageToModel(id);
  }
}

void PMDBImageModel::setCheckedAction(PMDBImageModel::SortType type) {
  switch (type) {
  case Id:
    sortByIdAction->setChecked(true);
    break;
  case Name:
    sortByNameAction->setChecked(true);
    break;
  case Header:
    sortByHeaderAction->setChecked(true);
    break;
  case Date:
    sortByDateAction->setChecked(true);
    break;
  default:
    break;
  }
  return;
}

// not used since 1.0.6 version
void PMDBImageModel::removeOutdatedImages(QItemSelectionModel *model) {
  QModelIndexList selected = model->selectedIndexes();
  QItemSelection toBeDeselected;
  if (type == Category) {
    foreach (QModelIndex index, selected) {
      if (!qListFromQVariant(data(index, Qt::PidRole)).contains(currCatId)) {
        toBeDeselected.merge(QItemSelection(index, index),
                             QItemSelectionModel::Select);
      }
    }
    emit selectImage(toBeDeselected, QItemSelectionModel::Deselect);
    foreach (QModelIndex index, toBeDeselected.indexes()) {
      removeImageFromModel(index);
    }
  }
}

void ::PMDBImageModel::loadLastAdded(int num) {
  clearModel();

  mWin->statusBar()->showMessage(tr("Loading images... Please wait.."));
  mWin->setEnabled(false);

  type = SearchResult;
  sortGroup->setEnabled(false);

  QSqlQuery query;
  query.exec("SELECT id FROM pm_images ORDER BY id DESC LIMIT " +
             QString::number(num));
  while (query.next()) {
    quint64 id = query.value(0).toUInt();
    addImageToModel(id);
  }

  mWin->setEnabled(true);
  mWin->statusBar()->clearMessage();
}

void PMDBImageModel::loadInventaryContents(const QModelIndex &invIndex,
                                           PMDBImageModel::SortType sortType) {
  clearModel();
  mWin->statusBar()->showMessage(tr("Loading images... Please wait.."));
  mWin->setEnabled(false);

  quint64 invId;
  if (invIndex.isValid()) {
    invId = invIndex.model()
                ->data(invIndex.model()->index(invIndex.row(), 0))
                .toUInt();
  } else {
    invId = 0;
  }

  loadInventaryContents(invId, sortType);
}

void PMDBImageModel::reloadInventaryContents(
    PMDBImageModel::SortType sortType) {
  loadInventaryContents(currCatId, sortType);
}

void PMDBImageModel::loadInventaryContents(quint64 invId,
                                           PMDBImageModel::SortType sortType) {
  currCatId = invId;
  type = Inventary;

  clearModel();
  mWin->statusBar()->showMessage(tr("Loading images... Please wait.."));
  mWin->setEnabled(false);

  QSqlQuery query;
  if (sortType == Current) {
    sortType = currSortType;
  }

  sortGroup->setEnabled(true);
  setCheckedAction(sortType);

  QString str = " id DESC";

  if (sortType == Date) {
    str = " date DESC ";
  } else if (sortType == Header) {
    str = " header ";
  } else if (sortType == Name) {
    str = " name ";
  }
  bool ok = query.exec("SELECT id FROM pm_images WHERE inventary=" +
                       QString::number(invId) + " ORDER BY " + str);
  if (!ok) {
    QMessageBox::warning(0, tr("Database error"), query.lastError().text());
  }
  while (query.next()) {
    quint64 id = query.value(0).toUInt();
    addImageToModel(id);
  }
  mWin->setEnabled(true);
  mWin->statusBar()->clearMessage();
}
