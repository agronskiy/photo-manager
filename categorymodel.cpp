#include "categorymodel.h"
#include "mainwindow.h"

PMCat::PMCat(const QString &name, quint64 id) {
  this->name = name;
  this->id = id;
  approx = "";
  parent = 0;
}

PMCat::~PMCat() {
  qDeleteAll(children);
  children.clear();
}

PMCatModel::PMCatModel(QObject *parent) : QAbstractItemModel(parent) {
  rootCat = 0;
}

PMCatModel::~PMCatModel() { delete rootCat; }

void PMCatModel::setRootItem(PMCat *cat) {
  delete rootCat;
  rootCat = cat;
  reset();
}

QModelIndex PMCatModel::index(int row, int column,
                              const QModelIndex &parent) const {
  //  qDebug() << "Index " << row << " " << column << " " << parent;
  if (!rootCat) {
    return QModelIndex();
  }
  PMCat *par = catFromIndex(parent);
  if (row < 0 || row >= par->children.count()) {
    return QModelIndex();
  }
  QModelIndex temp = createIndex(row, column, par->children[row]);
  //  qDebug() << "TEMPorary " << par->id << " Count " << par->children[-1] << "
  //  " << temp;
  return temp;
}

PMCat *PMCatModel::catFromIndex(const QModelIndex &index) const {
  //  qDebug() << "CatFromIndex " << index;
  if (index.isValid()) {
    return static_cast<PMCat *>(index.internalPointer());
  } else {
    //    qDebug()  << "Return rootCat " << const_cast<PMCat *>(rootCat);
    return rootCat;
  }
}

int PMCatModel::rowCount(const QModelIndex &parent) const {
  //  qDebug() << "rowCount " << parent;
  PMCat *parentCat = catFromIndex(parent);
  if (!parentCat) {
    //    qDebug() << "Returning 0 from rowCount()";
    return 0;
  } else {
    //    qDebug() << "Returning " << parentCat -> children.count();
    return parentCat->children.count();
  }
}

int PMCatModel::columnCount(const QModelIndex &parent) const {
  //  qDebug() << "columnCount " << parent;
  return 1;
}

QModelIndex PMCatModel::parent(const QModelIndex &child) const {
  //  qDebug() << "parent " << child;
  PMCat *cat = catFromIndex(child);
  if (!cat)
    return QModelIndex();
  PMCat *parentCat = cat->parent;
  if (!parentCat)
    return QModelIndex();
  PMCat *grandParentCat = parentCat->parent;
  if (!grandParentCat)
    return QModelIndex();

  int row = grandParentCat->children.indexOf(parentCat);
  return createIndex(row, child.column(), parentCat);
}

QVariant PMCatModel::data(const QModelIndex &index, int role) const {
  if (role != Qt::DisplayRole && role != Qt::CatIdRole &&
      role != Qt::AmountRole && role != Qt::NameRole)
    return QVariant();
  PMCat *cat = catFromIndex(index);
  if (!cat) {
    return QVariant();
  }

  if (index.column() == 0) {
    if (role == Qt::NameRole)
      return cat->name;
    else if (role == Qt::CatIdRole)
      return cat->id;
    else if (role == Qt::AmountRole)
      return cat->amount;
    else if (role == Qt::DisplayRole)
      return QVariant(cat->name + " (" + cat->approx +
                      QString::number(cat->amount) + ")");
  }
  return QVariant();
}

QVariant PMCatModel::headerData(int section, Qt::Orientation orient,
                                int role) const {
  if (orient == Qt::Horizontal && role == Qt::DisplayRole) {
    if (section == 0) {
      return tr("Category");
    }
  }
  return QVariant();
}

QModelIndex PMCatModel::addNewCategoryDB(const QString &name,
                                         const QModelIndex &parent) {
  QSqlTableModel table;
  table.setTable("pm_categories");

  QSqlQuery query;
  QSqlDatabase::database().transaction();

  bool ok = query.exec(
      "INSERT INTO pm_categories (pid, name, amount, outdated) VALUES (" +
      QString::number(catFromIndex(parent)->id) + ", '" + name + "', 0, '1')");
  if (!ok) {
    QMessageBox::warning(0, tr("Database error"), query.lastError().text());
    QSqlDatabase::database().rollback();
    return QModelIndex();
  } else {
    QSqlDatabase::database().commit();
    query.exec("SELECT MAX(id) FROM pm_categories");
    query.next();
    QModelIndex returnInd =
        appendCategory(name, 0, query.value(0).toUInt(), parent);
    reset();
    return returnInd;
  }
}

void PMCatModel::removeCategoryDB(const QModelIndex &index) {
  QSqlQuery query, q1, q2;

  for (int i = 0; i < rowCount(index); i++) {
    QModelIndex nextIndexToDelete = this->index(i, 0, index);
    removeCategoryDB(nextIndexToDelete);
  }
  PMCat *currentCat = catFromIndex(index);
  if (index.isValid()) {
    QSqlDatabase::database().transaction();
    bool ok = query.exec("DELETE FROM pm_categories WHERE id=" +
                         QString::number(currentCat->id));
    if (!ok) {
      QMessageBox::warning(0, tr("Error removing category"),
                           query.lastError().text());
      QSqlDatabase::database().rollback();
    } else {
      QSqlDatabase::database().commit();
      q1.exec("UPDATE pm_images SET pid=regexp_replace(pid,'x" +
              QString::number(currentCat->id) + "x','') WHERE 'x" +
              QString::number(currentCat->id) + "x'::tsquery @@ pid::tsvector");
      q2.exec(QString("UPDATE pm_images SET pid='x0x' ") +
              " WHERE  (pid !~ E'\\\\d')");
      mWin->modelImage()->clearModel();
    }

    PMCat *parentCat = catFromIndex(parent(index));
    int indexOf = parentCat->children.indexOf(currentCat);
    parentCat->children.removeAt(indexOf);
  }

  //  query.clear();
  //  QSqlDatabase::database().transaction();
  //  query.exec("SELECT id, pid FROM pm_images");
  //
  //  while(query.next()){
  //
  //    QString pids = query.value(1).toString();
  //    if(pids.contains(":" + QString::number(id) + ":")){
  //      pids.remove(QString::number(id)+":");
  //      if(pids.split(":", QString::SkipEmptyParts).isEmpty()){
  //        pids=":0:";
  //      }
  //
  //      QSqlQuery query2;
  //      query2.prepare("UPDATE pm_images SET pid=:pids WHERE id=" +
  //      QString::number(query.value(0).toUInt())); query2.bindValue(":pids",
  //      QVariant(pids)); bool ok = query2.exec(); if(!ok){
  //        QMessageBox::warning(0, tr("Error removing category"),
  //        query2.lastError().text());
  //      }
  //    }
  //  }
  //  QSqlDatabase::database().commit();

  delete currentCat;
  reset();
  //  Use findCategory for better access to categories
}

QModelIndex PMCatModel::appendCategory(const QString &name, int amount,
                                       quint64 id, const QModelIndex &parent) {
  PMCat *newCat = new PMCat(name, id);
  newCat->amount = amount;
  PMCat *parentCat = catFromIndex(parent);
  newCat->parent = parentCat;
  parentCat->children.append(newCat);
  // reset();
  return createIndex(parentCat->children.indexOf(newCat), 0, newCat);
}

void PMCatModel::readDataFromDB() {
  //  QSqlTableModel table;

  mWin->statusBar()->showMessage(tr("Loading categories tree... Please wait."));
  mWin->setEnabled(false);

  //  table.setTable("pm_categories");
  //  table.setSort(table.fieldIndex("name"), Qt::AscendingOrder);
  //  table.select();

  QSqlQueryModel table;
  QSqlQuery q;
  table.setQuery(QSqlQuery(
      " SELECT * FROM connectby('pm_categories', 'id', 'pid', 'name', '0', 0, '~') \
      AS t(id bigint, pid bigint, level int, branch text, pos int);"));

  delete rootCat;
  // reset();

  // checking outdated amount
  q.exec("SELECT id FROM pm_categories WHERE outdated='1'");
  while (q.next()) {
    calcAmountDB(q.value(0).toUInt());
  }

  rootCat = 0;
  rootCat = new PMCat("RootCategory", 0);
  QModelIndex ind = QModelIndex(), prevInd;
  int prevDepth = 1, currDepth, modelRowCount = table.rowCount();
  quint64 currId;

  //  appendCategoriesRecursively(ind, 0, table);
  for (int i = 1; i < modelRowCount; i++) {
    q.clear();
    currId = table.record(i).value("id").toUInt();
    currDepth = table.record(i).value("level").toInt();
    q.exec("SELECT name, amount FROM pm_categories WHERE id=" +
           QString::number(currId));
    q.next();
    if (currDepth == prevDepth) {
      prevInd = appendCategory(q.value(0).toString(), q.value(1).toInt(),
                               currId, ind);
    } else if (currDepth > prevDepth) {
      prevDepth = currDepth;
      ind = prevInd;
      prevInd = appendCategory(q.value(0).toString(), q.value(1).toInt(),
                               currId, ind);
    } else {
      ind = parent(ind);
      prevDepth--;
      i--;
      continue;
    }
  }
  reset();

  mWin->setEnabled(true);
  mWin->statusBar()->clearMessage();
}

// void PMCatModel::appendCategoriesRecursively(const QModelIndex &parent,
// quint64 pid,
//                                                                const
//                                                                QSqlTableModel
//                                                                &model){
//   qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
//   for(int i = 0; i<model.rowCount(); i++){
//     if(model.record(i).value("pid").toUInt() != pid) continue;
//     else {
//       QString name = model.record(i).value("name").toString();
//       quint64 id = model.record(i).value("id").toUInt();
//       //QModelIndex newIndex = appendCategory(name, id, parent);
//
//      // appendCategoriesRecursively(newIndex, id, model);
//     }
//   }
//   return;
// }

QModelIndex PMCatModel::findCategory(quint64 id,
                                     const QModelIndex &startIndex) const {
  if (id <= 0)
    return QModelIndex();
  PMCat *cat = catFromIndex(startIndex);
  if (cat->id == id)
    return startIndex;
  for (int i = 0; i < rowCount(startIndex); i++) {
    QModelIndex resIndex = findCategory(id, index(i, 0, startIndex));
    if (resIndex.isValid()) {
      return resIndex;
    }
  }
  return QModelIndex();
}

bool PMCatModel::setData(const QModelIndex &index, const QVariant &value,
                         int role) {
  if (role == Qt::NameRole && value.type() == QVariant::String) {
    PMCat *cat = catFromIndex(index);
    cat->name = value.toString();
    QSqlQuery query;
    query.prepare("UPDATE pm_categories SET name=:name WHERE id=" +
                  QString::number(cat->id));
    query.bindValue(":name", value);
    bool ok = query.exec();
    if (!ok) {
      QMessageBox::warning(0, tr("Error renaming category"),
                           query.lastError().text());
    }
    emit dataChanged(index, index);
    return ok;
  } else if (role == Qt::AmountRole && value.type() == QVariant::Int) {
    PMCat *cat = catFromIndex(index);
    cat->amount = value.toInt();
    cat->approx = "~ ";
    QSqlQuery query;
    query.prepare(
        "UPDATE pm_categories SET amount=:amount, outdated='1' WHERE id=" +
        QString::number(cat->id));
    query.bindValue(":amount", value);
    bool ok = query.exec();
    emit dataChanged(index, index);
    return ok;
  }
  return false;
}

QString PMCatModel::pathFromRoot(const QModelIndex &index,
                                 const QString &delim) const {
  QString result = "";
  QModelIndex currIndex = index;
  if (!index.isValid()) {
    return result;
  }

  result = data(currIndex, Qt::NameRole).toString();
  currIndex = currIndex.parent();

  while (currIndex.isValid()) {
    result = data(currIndex, Qt::NameRole).toString() + delim + result;
    currIndex = currIndex.parent();
  }

  return result;
}

void PMCatModel::setCategoryParentDB(const QModelIndex &index,
                                     const QModelIndex &newParent) {
  if (!index.isValid()) {
    return;
  }

  PMCat *newParCat = catFromIndex(newParent);
  PMCat *currCat = catFromIndex(index);
  PMCat *currParCat = currCat->parent;

  if (newParCat == 0 || currParCat == 0) {
    return;
  }

  if (newParCat == currCat) {
    return;
  }

  QSqlQuery q;
  q.prepare("UPDATE pm_categories SET pid=:pid WHERE id=" +
            QString::number(currCat->id));
  quint64 newParId = newParCat->id;
  q.bindValue("pid", newParId);
  bool ok = q.exec();
  if (!ok) {
    QMessageBox::warning(0, tr("Database error"), q.lastError().text());
    return;
  }

  currCat->parent = newParCat;
  currParCat->children.removeAll(currCat);
  newParCat->children.append(currCat);

  reset();
  return;
}

void PMCatModel::calcAmountDB(quint64 catId) {
  QSqlQuery q;
  q.exec("SELECT COUNT(*) FROM pm_images WHERE pid~'x" +
         QString::number(catId) + "x'");
  if (q.next()) {
    int amount = q.value(0).toInt();
    QSqlQuery q2;
    q2.exec("UPDATE pm_categories SET outdated='0', amount=" +
            QString::number(amount) + " WHERE id=" + QString::number(catId));
  }
}
