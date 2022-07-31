#ifndef INVWIDGET_H_INCLUDED
#define INVWIDGET_H_INCLUDED

#include "invmodel.h"
#include "ui_inventarywidget.h"

class QTreeView;
class QWidget;

class PMInvWidget : public QWidget, private Ui::InvWidget {
  Q_OBJECT
public:
  PMInvWidget(QWidget *parent = 0);
  QTreeView *treeView() { return tree; }

public slots:
  void saveSettings();
  void showInfo(QModelIndex curr);
  void recalculateButton();

private:
  QTreeView *tree;
  PMInv currInv;
};

#endif // INVWIDGET_H_INCLUDED
