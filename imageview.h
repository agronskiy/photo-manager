#ifndef IMAGEVIEW_H_INCLUDED
#define IMAGEVIEW_H_INCLUDED

#include <QAbstractItemView>
#include <QFont>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QWidget>

#include "mainwindow.h"
#include "pm.h"

class PMLoadImages;

class PMTransaction {
public:
  quint64 id;
  int row;
};

class PMView : public QAbstractItemView {
  Q_OBJECT
public:
  QPixmap debugPix;
  PMView(MainWindow *parent = 0);
  ~PMView();
  QRect visualRect(const QModelIndex &index) const;

  void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);

  QModelIndex indexAt(const QPoint &point) const;

  PMLoadImages *loadThr() { return loadThread; }
public slots:
  void metricsUpdated();
  void receivedPreview(int row, QByteArray arr);

protected slots:
  void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
  void rowsInserted(const QModelIndex &parent, int start, int end);
  void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);

protected:
  QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction,
                         Qt::KeyboardModifiers modifiers);

  int horizontalOffset() const;
  int verticalOffset() const;

  bool isIndexHidden(const QModelIndex &index) const;

  void setSelection(const QRect &rect,
                    QItemSelectionModel::SelectionFlags command);
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void keyPressEvent(QKeyEvent *event);
  void paintEvent(QPaintEvent *event);
  void resizeEvent(QResizeEvent *event);
  void wheelEvent(QWheelEvent *event);
  void scrollContentsBy(int dx, int dy);

  QRegion visualRegionForSelection(const QItemSelection &selection) const;
  QRegion itemRegion(const QModelIndex &index) const;
protected slots:
  void showFullImage(const QModelIndex &index);

protected:
  void updateGeom();
  void calculateMetrics();

  int itemWidth;
  int itemHeight;
  int columnNum;
  int pixWidth;
  int pixHeight;
  int margin;

  bool eventFilter(QObject *ob, QEvent *e);

  QPoint origin;
  QRubberBand *rubberBand;
  QModelIndex startIndexForSel;

  MainWindow *mw;
  PMLoadImages *loadThread;
};

#endif // IMAGEVIEW_H_INCLUDED
