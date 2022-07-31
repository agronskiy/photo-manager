#ifndef FLAGTREEVIEW_H_INCLUDED
#define FLAGTREEVIEW_H_INCLUDED
#include <QtGui>

#include "categorymodel.h"
#include "dbimagemodel.h"
#include "mainwindow.h"

class PMQCheckBox : public QCheckBox {
  Q_OBJECT
public:
  PMQCheckBox(const QModelIndex &catIndex, quint64 catId, QWidget *parent = 0);
  ~PMQCheckBox() {}

  quint64 id() { return catId; }
  QModelIndex &ind() { return catIndex; }
signals:
  void pmStateChanged(const QModelIndex, quint64, int);

public slots:
  void emitClicked();

protected:
  quint64 catId;
  QModelIndex catIndex;
};

class PMFlagTreeView : public QTreeView {
  Q_OBJECT
public:
  PMFlagTreeView(PMCatModel *catModel, PMDBImageModel *imageModel,
                 MainWindow *mainWindow, QWidget *parent = 0);

public slots:
  void updateMap();
  void imageSelectionChanged();
  void submitChanges();
  void submitBox(const QModelIndex &catIndex, quint64 id, int state);
  void highlightCat(quint64 id);

protected:
  void paintEvent(QPaintEvent *event);
  void drawRectForIndex(QPainter &painter, QModelIndex &index);
  void updateMap2(const QModelIndex &index);
  void initConnections();
  void populateCheckedIdLists();
  void updateBoxesState();

  bool needsUpdate;

  QMap<quint64, PMQCheckBox *> mapIdToBox;
  QMap<QModelIndex, QSet<quint64>> presentCats;

  QVector<PMQCheckBox *> boxes;

  QSet<quint64> checkedCatIds;
  QSet<quint64> grayCheckedCatIds;
  QSet<quint64> grayedBoxIds;

  QModelIndexList currSelectedImages;
  PMCatModel *catModel;
  PMDBImageModel *imageModel;
  MainWindow *mainWindow;
  quint64 foundId;
};

class PMFlagTreeWidget : public QWidget {
  Q_OBJECT
public:
  PMFlagTreeWidget(PMCatModel *catModel, PMDBImageModel *imageModel,
                   MainWindow *mainWindow, QWidget *parent = 0);
  PMFlagTreeView *treeView() { return tree; }

public slots:
  void imageSelectionChanged();

protected slots:
  void showNextFound();
  void highlightCat(int i);
  void searchByName();

protected:
  int searchIter;

  QLineEdit *searchEdit;
  QLabel *searchLabel;
  QPushButton *searchButton;
  QPushButton *nextButton;
  QGroupBox *group;

  QList<quint64> searchResults;
  MainWindow *mainWindow;
  PMFlagTreeView *tree;
};
#endif // FLAGTREEVIEW_H_INCLUDED
