#ifndef IMAGEVIEWER_H_INCLUDED
#define IMAGEVIEWER_H_INCLUDED

#include <QtGui>

class PMImageViewer : public QDialog {
  Q_OBJECT
public:
  PMImageViewer(QPixmap pix, QWidget *parent = 0);
  void setCaption(QString &str);

public slots:
  void adjToWin();

protected slots:
  void zoomIn();
  void zoomOut();
  void normalSize();

  void contextMenuEvent(QContextMenuEvent *event);
  void showEvent(QShowEvent *event);

protected:
  void adjustScrollBar(QScrollBar *bar, double factor);
  void scaleImage(double factor);

  QAction *zoomInAction;
  QAction *zoomOutAction;
  QAction *normalSizeAction;
  QAction *adjustToWindowAction;

  QMenu *contextMenu;

  QLabel *imageLabel;

  QScrollArea *scrollArea;

  double scaleFactor;
  bool firstShown;
};

#endif // IMAGEVIEWER_H_INCLUDED
