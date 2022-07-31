#ifndef CATWIDGET_H_INCLUDED
#define CATWIDGET_H_INCLUDED
#include <QtGui>

#include "ui_categorypanel.h"

class MainWindow;

class PMCatWidget : public QWidget, public Ui_CatWidget {
  Q_OBJECT
public:
  PMCatWidget(MainWindow *m, QWidget *parent = 0);
  QTreeView *treeView() { return tree; }

public slots:
  void saveChanges();
  void treeClicked(const QModelIndex &index);
  void recalculateButtons();

protected:
  QTreeView *tree;
};

#endif // CATWIDGET_H_INCLUDED
