#include <QtGui>

#include "imageviewer.h"

PMImageViewer::PMImageViewer(QPixmap pix, QWidget *parent) : QDialog(parent) {
  if (pix.isNull()) {
    this->close();
  }

  imageLabel = new QLabel();
  imageLabel->setPixmap(pix);
  imageLabel->setBackgroundRole(QPalette::Base);
  imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  imageLabel->setScaledContents(true);

  scrollArea = new QScrollArea;
  scrollArea->setBackgroundRole(QPalette::Dark);
  scrollArea->setWidget(imageLabel);
  scrollArea->setAlignment(Qt::AlignCenter);

  QVBoxLayout *l = new QVBoxLayout;
  l->addWidget(scrollArea);
  setLayout(l);

  scaleFactor = 1.0;
  firstShown = true;

  zoomInAction = new QAction(tr("Zoom In"), this);
  connect(zoomInAction, SIGNAL(triggered()), this, SLOT(zoomIn()));

  zoomOutAction = new QAction(tr("Zoom Out"), this);
  connect(zoomOutAction, SIGNAL(triggered()), this, SLOT(zoomOut()));

  normalSizeAction = new QAction(tr("Normal Size"), this);
  connect(normalSizeAction, SIGNAL(triggered()), this, SLOT(normalSize()));

  adjustToWindowAction = new QAction(tr("To window size"), this);
  connect(adjustToWindowAction, SIGNAL(triggered()), this, SLOT(adjToWin()));

  contextMenu = new QMenu;
  contextMenu->addAction(zoomInAction);
  contextMenu->addAction(zoomOutAction);
  contextMenu->addAction(normalSizeAction);
  contextMenu->addAction(adjustToWindowAction);

  QRect screenRect = QApplication::desktop()->availableGeometry();
  resize((int)(screenRect.width() * 0.8), (int)(screenRect.height() * 0.8));
}

void PMImageViewer::setCaption(QString &str) { this->setWindowTitle(str); }

void PMImageViewer::zoomIn() { scaleImage(1.25); }

void PMImageViewer::zoomOut() { scaleImage(0.8); }

void PMImageViewer::adjToWin() {
  double factor;
  QRect scrollAreaRect = scrollArea->childrenRect();
  factor = (scrollAreaRect.width()) / (double)(imageLabel->width());
  //  qDebug() << scrollAreaRect.size() << " " << imageLabel->size() << " " <<
  //  factor;
  if ((scrollAreaRect.height()) / (double)(imageLabel->height()) < factor) {
    factor = (scrollAreaRect.height()) / (double)(imageLabel->height());
  }

  scaleImage(factor);
  //  qDebug() << scrollAreaRect.size() << " " << imageLabel->size() << " " <<
  //  factor;
}

void PMImageViewer::scaleImage(double factor) {
  if (imageLabel->pixmap() && !imageLabel->pixmap()->isNull()) {
    scaleFactor *= factor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAction->setEnabled(scaleFactor < 3.0);
    zoomOutAction->setEnabled(scaleFactor > 0.333);
  }
}

void PMImageViewer::adjustScrollBar(QScrollBar *bar, double factor) {
  bar->setValue(
      (int)(factor * bar->value() + (factor - 1) * (bar->pageStep() / 2)));
}

void PMImageViewer::normalSize() {
  imageLabel->adjustSize();
  scaleFactor = 1.0;
}

void PMImageViewer::contextMenuEvent(QContextMenuEvent *event) {
  contextMenu->exec(event->globalPos());
}

void PMImageViewer::showEvent(QShowEvent *event) {
  if (firstShown) {
    adjToWin();
    firstShown = false;
  }
  QDialog::showEvent(event);
}
