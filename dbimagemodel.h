#ifndef DBIMAGEMODEL_H_INCLUDED
#define DBIMAGEMODEL_H_INCLUDED
#include <QtGui>

#include "categorymodel.h"
#include "pm.h"

class PMDBImage {
public:
  PMDBImage() {}
  ~PMDBImage() {}

  quint64 id;
  QString name;
  QString header;
  QString description;

  quint64 inventary; // inventary number
  QString location;  // location in the %PHOTO_MANAGER_DATA_DIR%
  QString cd;
  QList<quint64> pid;
  QDate date;

  int width, height, origWidth, origHeight, xDensityDpi,
      yDensityDpi; // TODO work here
  double origSize;
  QPixmap preview;
};

class PMDBImageModel : public QAbstractListModel {
  friend class MainWindow;
  Q_OBJECT
public:
  PMDBImageModel(QObject *parent = 0);
  ~PMDBImageModel();

  int rowCount(const QModelIndex &parent = QModelIndex()) const;

  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orient, int role) const {
    return QVariant();
  }

  bool setData(const QModelIndex &index, const QVariant &value,
               int role = Qt::EditRole);

  QModelIndex findImage(quint64 id) const;
  QModelIndex index(int row, int column = 0,
                    const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &child = QModelIndex()) const;

  static QVariant qVariantFromQList(const QList<quint64> &list);
  static QList<quint64> qListFromQVariant(const QVariant &var);

  void clearModel();
  void setCatModel(PMCatModel *model) { currCatModel = model; }

  enum CurrLoadedType { None = 0, Category, SearchResult, Inventary };

  enum SortType { Current = 0, Id, Name, Date, Header };

  QActionGroup *sortGroup;

public slots:
  void loadCategoryContents() {
    loadCategoryContents(QModelIndex(), PMDBImageModel::Current);
  }
  void loadCategoryContents(
      const QModelIndex &catIndex,
      PMDBImageModel::SortType sortType = PMDBImageModel::Current);
  void loadInventaryContents(
      const QModelIndex &invIndex,
      PMDBImageModel::SortType sortType = PMDBImageModel::Current);
  void loadInventaryContents(quint64 invId, PMDBImageModel::SortType sortType =
                                                PMDBImageModel::Current);
  void reloadInventaryContents(
      PMDBImageModel::SortType sortType = PMDBImageModel::Current);
  void
  reloadContents(PMDBImageModel::SortType sortType = PMDBImageModel::Current);
  void loadSearchResults();
  void loadLastAdded(int num);
  void setSearchResults(const QList<quint64> &idList) {
    searchResults = idList;
  }
  void removeImageFromModel(const QModelIndex &indexToDelete = QModelIndex());
  void sortId() {
    currSortType = Id;
    reloadContents(PMDBImageModel::Id);
  }
  void sortDate() {
    currSortType = Date;
    reloadContents(PMDBImageModel::Date);
  }
  void sortName() {
    currSortType = Name;
    reloadContents(PMDBImageModel::Name);
  }
  void sortHeader() {
    currSortType = Header;
    reloadContents(PMDBImageModel::Header);
  }
  void removeOutdatedImages(QItemSelectionModel *model);

  bool removeImageFromModelDB(const QModelIndex &indexToDelete = QModelIndex());

  QModelIndex addImageToModel(quint64 id);
signals:
  void selectImage(const QItemSelection &sel,
                   QItemSelectionModel::SelectionFlags flags);

private:
  PMDBImage *imageFromIndex(const QModelIndex &index) const;

  void setCheckedAction(PMDBImageModel::SortType type);

  QAction *sortByIdAction;
  QAction *sortByDateAction;
  QAction *sortByHeaderAction;
  QAction *sortByNameAction;

  quint64 currCatId;
  CurrLoadedType type;
  SortType currSortType;
  QList<quint64> searchResults;
  PMCatModel *currCatModel;
  QList<PMDBImage *> images;
};

#endif // DBIMAGEMODEL_H_INCLUDED
