#ifndef THREADS_H_INCLUDED
#define THREADS_H_INCLUDED
#include <QtCore>
#include <QtGui>

#include "dbimagemodel.h"
#include "imageview.h"
#include "mainwindow.h"
#include "searchdialog.h"

class PMTransaction;

class PMAddImagesFromDirThread : public QThread {
  Q_OBJECT
public:
  PMAddImagesFromDirThread(MainWindow *mainWindow, QString dirPath,
                           QObject *parent = 0);

signals:
  void showMessage(QString &warn);
  void percentageLoaded(int i);

protected:
  void run();
  MainWindow *mw;
  QString dirPath;
};

class PMSearchThread : public QThread {
  Q_OBJECT
public:
  PMSearchThread(MainWindow *mainWindow, QString str, QObject *parent = 0);

signals:
  void searchMsg(int i);

protected:
  void run();
  QString str;
  MainWindow *mw;
};

class PMLoadImages : public QThread {
  Q_OBJECT
public:
  PMLoadImages(QObject *parent = 0) { db = QSqlDatabase::database("second"); }
  ~PMLoadImages();
  void addTransaction(PMTransaction t);
  void clearTransactions();
  void run();

  QSqlDatabase db;
signals:
  void finishedImage(int row, QByteArray arr);

protected:
  QMutex mutex;
  QQueue<PMTransaction> trans;
};

#endif // THREADS_H_INCLUDED
