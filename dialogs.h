#ifndef DIALOGS_H_INCLUDED
#define DIALOGS_H_INCLUDED

#include <QDialog>

#include "ui_addinventaryform.h"
#include "ui_cachesettings.h"
#include "ui_viewersettings.h"

class QTreeView;
class QLineEdit;
class MainWindow;
class QLabel;
class QPushButton;
class QItemSelectionModel;
class PMCache;
class PMInv;

class PMNewCatDialog : public QDialog {
  Q_OBJECT
public:
  PMNewCatDialog(MainWindow *parent = 0);
public slots:
  void recalculateButton();
  void createCategory();

private:
  QTreeView *parentEdit;
  QLineEdit *nameEdit;
  QPushButton *createButton;
  QPushButton *cancelButton;
  QLabel *nameLabel;
  QLabel *parentEditLabel;
};

class PMRemoveCatDialog : public QDialog {
  Q_OBJECT
public:
  PMRemoveCatDialog(MainWindow *parent = 0);
public slots:
  void recalculateButton();
  void removeCategories();

private:
  QTreeView *catEdit;
  QPushButton *removeButton;
  QPushButton *cancelButton;
  QLabel *catEditLabel;
};

class PMChangeCatParDialog : public QDialog {
  Q_OBJECT
public:
  PMChangeCatParDialog(MainWindow *parent = 0);
public slots:
  void changeParCategory();

private:
  QTreeView *parentEdit;
  QPushButton *createButton;
  QPushButton *cancelButton;
  QLabel *parentEditLabel;
};

class PMShowImageDialog : public QDialog {
  Q_OBJECT
public:
  PMShowImageDialog(const QPixmap &pixmap, const QString &cap,
                    QWidget *parent = 0);

private:
  QLabel *picLabel;
};

class PMViewerSettingsDialog : public QDialog,
                               private Ui::ViewerSettingsDialog {
  Q_OBJECT
public:
  PMViewerSettingsDialog(QWidget *parent = 0);

signals:
  void updatedVals();

public slots:
  void saveSettings();
};

class PMCacheSettingsDialog : public QDialog, private Ui::CacheSettingsDialog {
  Q_OBJECT
public:
  PMCacheSettingsDialog(PMCache *cache, QWidget *parent = 0);

public slots:
  void saveSettings();

protected:
  PMCache *cache;
};

class PMAddInvDialog : public QDialog, private Ui::AddNewInvDialog {
  Q_OBJECT
public:
  PMAddInvDialog(PMInv *inv, bool *ok, QString *dir, QString *commonDescription,
                 int *createInv, int *depth, QWidget *parent = 0);
  enum { Coloured = 0, Monohrome };
public slots:
  void startProcess();
  void cancelProcess();
  void showFolderDialog();
  void enableBox(int n);

private:
  PMInv *inv;
  bool *ok;
  QString *dir, *commonDescription;
  int *createInv, *depth;
};

#endif // DIALOGS_H_INCLUDED
