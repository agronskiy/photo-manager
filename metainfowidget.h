#ifndef METAINFOWIDGET_H_INCLUDED
#define METAINFOWIDGET_H_INCLUDED
#include <QItemSelection>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QPushButton>
#include <QRadioButton>
#include <QTextEdit>

#include "dbimagemodel.h"
#include "mainwindow.h"

class PMMetaInfoWidget : public QDialog {
  Q_OBJECT
public:
  PMMetaInfoWidget(MainWindow *mainWindow, QWidget *parent = 0);

public slots:
  void imageSelectionChanged();
  void submitChanges();

protected:
  QLabel *nameEditLabel;
  QLabel *headerEditLabel;
  QLabel *descriptionEditLabel;
  QLabel *dateEditLabel;
  QLabel *locationEditLabel;
  QLabel *inventaryEditLabel;
  QLabel *authorEditLabel;
  QLabel *idLabel;
  QLabel *cdLabel;
  QLabel *invAuthorLabel;
  QLabel *typeLabel;

  QRadioButton *appendDescriptionButton;
  QRadioButton *replaceDescriptionButton;

  QCheckBox *changeDateFlag;
  QCheckBox *forUsBox;

  QLineEdit *nameEdit;
  QLineEdit *locationEdit;
  QLineEdit *inventaryEdit;
  QLineEdit *cdEdit;
  QLineEdit *idEdit;
  QLineEdit *typeEdit;
  QLineEdit *authorEdit;
  QLineEdit *invAuthorEdit;

  QTextEdit *headerEdit;
  QTextEdit *descriptionEdit;
  QTextEdit *categoriesEdit;

  QDateEdit *dateEdit;

  QPushButton *submitButton;
  QPushButton *resetButton;

  QModelIndexList currSelected;
  PMDBImageModel *imageModel;
  MainWindow *parentWindow;
};

#endif // METAINFOWIDGET_H_INCLUDED
