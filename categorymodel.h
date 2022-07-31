#ifndef CATEGORYMODEL_H_INCLUDED
#define CATEGORYMODEL_H_INCLUDED
#include <QtCore>
#include <QtDebug>
#include <QtGui>
#include <QtSql>

#include "pm.h"

class PMCat {
public:
  PMCat(const QString &name = "", quint64 id = -1);
  ~PMCat();

  QString name;
  quint64 id;
  int amount;
  QString approx;
  PMCat *parent;
  QList<PMCat *> children;
};

class PMCatModel : public QAbstractItemModel { // the category model
  Q_OBJECT
public:
  PMCatModel(QObject *parent = 0);
  ~PMCatModel();

  void setRootItem(PMCat *cat);

  QModelIndex index(int row, int column, const QModelIndex &parent) const;
  QModelIndex parent(const QModelIndex &child) const;
  QModelIndex findCategory(quint64 id,
                           const QModelIndex &startIndex = QModelIndex()) const;

  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;

  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orient, int role) const;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role = Qt::NameRole);

  QString pathFromRoot(const QModelIndex &index, const QString &delim) const;

public slots:
  QModelIndex addNewCategoryDB(const QString &name,
                               const QModelIndex &parent = QModelIndex());

  void removeCategoryDB(const QModelIndex &index);
  void setCategoryParentDB(const QModelIndex &index,
                           const QModelIndex &newParent);
  void readDataFromDB();

private:
  QModelIndex appendCategory(const QString &name, int amount, quint64 id,
                             const QModelIndex &parent);

  void calcAmountDB(quint64 catId);
  //  void appendCategoriesRecursively(const QModelIndex &parent, quint64 pid,
  //  const QSqlTableModel &model);

  PMCat *catFromIndex(const QModelIndex &index) const;
  PMCat *rootCat;
};

#endif // CATEGORYMODEL_H_INCLUDED
