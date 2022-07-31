#include "imageview.h"
#include "dialogs.h"
#include "imageviewer.h"
#include "threads.h"
#include <QtGui>
#include <cmath>

PMView::PMView(MainWindow *parent) : QAbstractItemView(parent) {
  qRegisterMetaType<QByteArray>("QByteArray");
  setUpdatesEnabled(true);
  loadThread = new PMLoadImages;
  connect(loadThread, SIGNAL(finishedImage(int, QByteArray)), this,
          SLOT(receivedPreview(int, QByteArray)));
  connect(loadThread, SIGNAL(finished()), this, SLOT(update()),
          Qt::DirectConnection);

  viewport()->installEventFilter(this);

  loadThread->run();

  horizontalScrollBar()->setRange(0, 0);
  verticalScrollBar()->setRange(0, 0);
  itemWidth = pm::itemWidth;
  itemHeight = pm::itemHeight;
  columnNum = pm::columnNum;

  calculateMetrics();

  margin = 10;
  rubberBand = 0;

  mw = parent;

  connect(this, SIGNAL(activated(const QModelIndex)), this,
          SLOT(showFullImage(const QModelIndex)));
}

PMView::~PMView() {
  loadThread->terminate();
  delete loadThread;
}

void PMView::metricsUpdated() {
  horizontalScrollBar()->setRange(0, 0);
  verticalScrollBar()->setRange(0, 0);
  itemWidth = pm::itemWidth;
  itemHeight = pm::itemHeight;
  columnNum = pm::columnNum;

  calculateMetrics();
  update();
}

void PMView::calculateMetrics() {
  pixWidth = (int)((double)itemWidth * 0.9);
  pixHeight = (int)((double)itemHeight * 0.7);
  margin = 10;
}

void PMView::dataChanged(const QModelIndex &topLeft,
                         const QModelIndex &bottomRight) {
  QAbstractItemView::dataChanged(topLeft, bottomRight);
  //  viewport()->update();
}

void PMView::paintEvent(QPaintEvent *event) {
  QItemSelectionModel *selections = selectionModel();
  QStyleOptionViewItem option = viewOptions();
  QStyle::State state = option.state;
  QFontMetrics currFontMetrics = fontMetrics();
  QFont italicFont = font();
  italicFont.setItalic(true);
  QBrush background = option.palette.base();
  QPen foreground(option.palette.color(QPalette::WindowText));
  QPen textPen(option.palette.color(QPalette::Text));
  QPen highlightedPen(option.palette.color(QPalette::HighlightedText));

  QPainter painter(viewport());
  painter.setRenderHint(QPainter::Antialiasing);

  painter.fillRect(event->rect(), background);
  painter.setPen(foreground);

  //  QRect pieRect = QRect(margin, margin, pieSize, pieSize);
  //  QPoint keyPoint = QPoint(totalSize - horizontalScrollBar()->value(),
  //                          margin - verticalScrollBar()->value());
  int areaWidth = columnNum * itemWidth;
  int areaHeight = (int)(std::ceil((double)(model()->rowCount(rootIndex())) /
                                   (double)(columnNum))) *
                   itemHeight;

  QRect areaRect = QRect(margin, margin, areaWidth, areaHeight);
  QRect regionToBeUpdated = viewport()->rect();
  regionToBeUpdated.translate(horizontalScrollBar()->value(),
                              verticalScrollBar()->value());

  painter.translate(areaRect.x() - horizontalScrollBar()->value(),
                    areaRect.y() - verticalScrollBar()->value());

  int currColumn = 0;
  int currRow = 0;
  for (int i = 0; i < model()->rowCount(rootIndex()); i++) {
    currRow = i / columnNum;
    currColumn = i - currRow * columnNum;
    QModelIndex indexToDraw = model()->index(i, 0, rootIndex());
    if (itemRegion(indexToDraw).intersected(regionToBeUpdated).isEmpty()) {
      continue;
    }
    PMDBImage image;
    //    VERY SLOW!

    bool foundPix = QPixmapCache::find(
        model()->data(indexToDraw, Qt::IdRole).toString(), image.preview);

    if (!foundPix) {
      //      image.preview = MainWindow::loadPreview(model()->data(indexToDraw,
      //      Qt::IdRole).toUInt());
      //      image.preview.loadFromData(model()->data(indexToDraw,
      //      Qt::PreviewRole).toByteArray(), "JPG");
      PMTransaction t;
      t.id = model()->data(indexToDraw, Qt::IdRole).toUInt();
      t.row = indexToDraw.row();
      loadThread->addTransaction(t);
    }

    image.name = model()->data(indexToDraw, Qt::NameRole).toString();
    image.header = model()->data(indexToDraw, Qt::HeaderRole).toString();

    painter.setPen(Qt::NoPen);
    if (selections->selectedIndexes().contains(indexToDraw)) {
      if (currentIndex() == indexToDraw) {
        painter.setPen(Qt::SolidLine);
        painter.setPen(option.palette.color(QPalette::HighlightedText));
      }

      painter.setBrush(QBrush(option.palette.color(QPalette::Highlight)));
    } else {
      if (currentIndex() == indexToDraw) {
        painter.setPen(Qt::SolidLine);
        painter.setPen(
            QBrush(option.palette.color(QPalette::Highlight)).color());
      }
      painter.setBrush(QBrush(option.palette.color(QPalette::Base)));
    }
    painter.drawRect(currColumn * itemWidth, currRow * itemHeight, itemWidth,
                     itemHeight);
    painter.setPen(Qt::NoPen);

    if (!image.preview.isNull()) {
      if (!foundPix) {

        if (image.preview.width() > pixWidth) {
          image.preview =
              image.preview.scaledToWidth(pixWidth, Qt::SmoothTransformation);
        }
        if (image.preview.height() > pixHeight) {
          image.preview =
              image.preview.scaledToHeight(pixHeight, Qt::SmoothTransformation);
        }
        QPixmapCache::insert(model()->data(indexToDraw, Qt::IdRole).toString(),
                             image.preview);
      }
      painter.drawPixmap(
          currColumn * itemWidth + (itemWidth - image.preview.width()) / 2,
          currRow * itemHeight + (int)((double)itemHeight * 0.05),
          image.preview.width(), image.preview.height(), image.preview);
    } else {
      painter.save();
      painter.setPen(Qt::SolidLine);
      painter.setBrush(QBrush(Qt::darkGray));
      painter.drawRect(currColumn * itemWidth + (itemWidth - pixWidth) / 2,
                       currRow * itemHeight + (int)((double)itemHeight * 0.05),
                       pixWidth, pixHeight);
      painter.restore();
    }
    // drawing name
    if (selections->isSelected(indexToDraw)) {
      painter.setPen(highlightedPen);
    } else {
      painter.setPen(textPen);
    }
    painter.drawText(
        currColumn * itemWidth,
        currRow * itemHeight + (int)((double)itemHeight * 0.05) + pixHeight,
        itemWidth, currFontMetrics.height(), Qt::AlignCenter, image.name);

    QTextLayout textLayout(image.header, italicFont);
    //    int yPos = currRow*itemHeight + (int)((double)itemHeight*0.05) +
    //    pixHeight +
    //                                      currFontMetrics.height() +
    //                                      currFontMetrics.lineSpacing();
    double yPos = 0.0;
    textLayout.setTextOption(QTextOption(Qt::AlignCenter));
    textLayout.beginLayout();
    for (int i = 0; i < 2; i++) {
      QTextLine line = textLayout.createLine();
      if (!line.isValid()) {
        break;
      }
      line.setLineWidth((double)itemWidth * 0.9);
      line.setPosition(QPointF(0, yPos));
      yPos += line.height();
    }
    textLayout.endLayout();
    textLayout.draw(&painter,
                    QPointF(currColumn * itemWidth + itemWidth * 0.05,
                            currRow * itemHeight +
                                (int)((double)itemHeight * 0.05) + pixHeight +
                                currFontMetrics.height()));

    currColumn = (currColumn + 1) % columnNum;
    if (currColumn == 0)
      currRow++;
  }
  updateGeom();
}

QModelIndex PMView::indexAt(const QPoint &point) const {
  int x = point.x() + horizontalScrollBar()->value();
  int y = point.y() + verticalScrollBar()->value();
  x -= margin;
  y -= margin;

  int column = x / itemWidth;
  int row = y / itemHeight;

  if (column >= columnNum) {
    return QModelIndex();
  }

  int currNumber = row * columnNum + column;
  return model()->index(currNumber, 0, rootIndex());
}

bool PMView::isIndexHidden(const QModelIndex &index) const { return false; }

int PMView::horizontalOffset() const { return horizontalScrollBar()->value(); }

void PMView::mousePressEvent(QMouseEvent *event) {
  QAbstractItemView::mousePressEvent(event);
  origin = event->pos() +
           QPoint(horizontalScrollBar()->value(), verticalScrollBar()->value());
  if (!rubberBand)
    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
  rubberBand->setGeometry(QRect(origin - QPoint(horizontalScrollBar()->value(),
                                                verticalScrollBar()->value()),
                                QSize()));
  rubberBand->show();
}

void PMView::mouseMoveEvent(QMouseEvent *event) {
  if (rubberBand)
    rubberBand->setGeometry(
        QRect(origin - QPoint(horizontalScrollBar()->value(),
                              verticalScrollBar()->value()),
              event->pos())
            .normalized());
  QAbstractItemView::mouseMoveEvent(event);
  setSelection(rubberBand->rect(), QItemSelectionModel::Current);
}

void PMView::mouseReleaseEvent(QMouseEvent *event) {
  QAbstractItemView::mouseReleaseEvent(event);
  if (rubberBand)
    rubberBand->hide();
  viewport()->update();
}

QModelIndex PMView::moveCursor(QAbstractItemView::CursorAction cursorAction,
                               Qt::KeyboardModifiers modifiers) {
  QModelIndex current = currentIndex();
  QModelIndex newIndex;
  switch (cursorAction) {
  case MoveLeft:
    if (current.row() == 0) {
      break;
    }
    newIndex = model()->index(current.row() - 1, 0, rootIndex());
    scrollTo(newIndex);
    break;
  case MoveUp:
    if (current.row() < columnNum) {
      break;
    }
    newIndex = model()->index(current.row() - columnNum, 0, rootIndex());
    scrollTo(newIndex);
    break;
  case MoveRight:
    if (current.row() == model()->rowCount(rootIndex()) - 1) {
      break;
    }
    newIndex = model()->index(current.row() + 1, 0, rootIndex());
    scrollTo(newIndex);
    break;
  case MoveDown:
    if (current.row() + columnNum >= model()->rowCount(rootIndex())) {
      newIndex =
          model()->index(model()->rowCount(rootIndex()) - 1, 0, rootIndex());
    } else {
      newIndex = model()->index(current.row() + columnNum, 0, rootIndex());
    }
    scrollTo(newIndex);
    break;
  case MovePageUp:
    if (current.row() < 3 * columnNum) {
      break;
    }
    newIndex = model()->index(current.row() - 3 * columnNum, 0, rootIndex());
    scrollTo(newIndex);
    break;
  case MovePageDown:
    if (current.row() + 3 * columnNum >= model()->rowCount(rootIndex())) {
      newIndex =
          model()->index(model()->rowCount(rootIndex()) - 1, 0, rootIndex());
    } else {
      newIndex = model()->index(current.row() + 3 * columnNum, 0, rootIndex());
    }
    scrollTo(newIndex);
    break;
  default:
    break;
  }
  if (modifiers & Qt::ControlModifier) {
    selectionModel()->select(newIndex, QItemSelectionModel::Toggle);
  }
  if (modifiers & Qt::ShiftModifier) {
    selectionModel()->select(QItemSelection(startIndexForSel, newIndex),
                             QItemSelectionModel::ClearAndSelect);
  }
  return newIndex;
}

void PMView::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Shift && !(event->isAutoRepeat())) {
    startIndexForSel = selectionModel()->currentIndex();
  }

  if (event->modifiers() & Qt::ShiftModifier) {
    switch (event->key()) {
    case Qt::Key_Down:
      selectionModel()->setCurrentIndex(
          moveCursor(MoveDown, event->modifiers()),
          QItemSelectionModel::NoUpdate);
      break;
    case Qt::Key_Up:
      selectionModel()->setCurrentIndex(moveCursor(MoveUp, event->modifiers()),
                                        QItemSelectionModel::NoUpdate);
      break;
    case Qt::Key_Left:
      selectionModel()->setCurrentIndex(
          moveCursor(MoveLeft, event->modifiers()),
          QItemSelectionModel::NoUpdate);
      break;
    case Qt::Key_Right:
      selectionModel()->setCurrentIndex(
          moveCursor(MoveRight, event->modifiers()),
          QItemSelectionModel::NoUpdate);
      break;
    case Qt::Key_PageDown:
      selectionModel()->setCurrentIndex(
          moveCursor(MovePageDown, event->modifiers()),
          QItemSelectionModel::NoUpdate);
      break;
    case Qt::Key_PageUp:
      selectionModel()->setCurrentIndex(
          moveCursor(MovePageUp, event->modifiers()),
          QItemSelectionModel::NoUpdate);
      break;
    default:
      QAbstractItemView::keyPressEvent(event);
    }
  } else {
    QAbstractItemView::keyPressEvent(event);
  }
}

void PMView::rowsInserted(const QModelIndex &parent, int start, int end) {
  QAbstractItemView::rowsInserted(parent, start, end);
  updateGeom();
}

void PMView::rowsAboutToBeRemoved(const QModelIndex &parent, int start,
                                  int end) {
  QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
  updateGeom();
}

int PMView::verticalOffset() const { return verticalScrollBar()->value(); }

void PMView::scrollContentsBy(int dx, int dy) {
  viewport()->scroll(dx, dy);
  loadThread->clearTransactions();
  viewport()->update();
}

void PMView::scrollTo(const QModelIndex &index, ScrollHint hint) {
  QRect area = viewport()->rect().translated(horizontalScrollBar()->value(),
                                             verticalScrollBar()->value());
  QRect rect = itemRegion(index).boundingRect();

  if (rect.left() < area.left()) {
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() +
                                    rect.left() - area.left() - margin);
  } else if (rect.right() > area.right()) {
    horizontalScrollBar()->setValue(
        horizontalScrollBar()->value() + margin +
        qMin(rect.right() - area.right(), rect.left() - area.left()));
  }

  if (rect.top() < area.top()) {
    verticalScrollBar()->setValue(verticalScrollBar()->value() + rect.top() -
                                  area.top() - margin);
  } else if (rect.bottom() > area.bottom()) {
    verticalScrollBar()->setValue(
        verticalScrollBar()->value() + margin +
        qMin(rect.bottom() - area.bottom(), rect.top() - area.top()));
  }
  //  update();
  viewport()->update();
}

QRegion PMView::itemRegion(
    const QModelIndex &index) const { // returns in contents coordinates
  if (!index.isValid()) {
    return QRegion();
  }
  int indexNumber = index.row();
  if (indexNumber >= index.model()->rowCount(QModelIndex())) {
    return QRegion();
  }
  int row = indexNumber / columnNum;
  int column = indexNumber - row * columnNum;

  QRect rect = QRect(margin + column * itemWidth, margin + row * itemHeight,
                     itemWidth, itemHeight);
  return QRegion(rect);
}

void PMView::setSelection(const QRect &rect,
                          QItemSelectionModel::SelectionFlags command) {
  QRect contentsRect = rect.translated(horizontalScrollBar()->value(),
                                       verticalScrollBar()->value())
                           .normalized();
  QModelIndexList indexes;
  for (int i = 0; i < model()->rowCount(rootIndex()); i++) {
    QModelIndex currIndex = model()->index(i, 0, rootIndex());
    QRegion indexRegion = itemRegion(currIndex);
    if (!indexRegion.intersected(contentsRect).isEmpty()) {
      indexes.append(currIndex);
    }
  }
  QItemSelection selection;
  for (int i = 0; i < indexes.count(); i++) {
    QModelIndex currIndex = indexes.at(i);
    selection.merge(QItemSelection(currIndex, currIndex), command);
  }

  selectionModel()->select(selection, command);
  update();
}

void PMView::resizeEvent(QResizeEvent * /* event */) { updateGeom(); }

QRegion
PMView::visualRegionForSelection(const QItemSelection &selection) const {
  int ranges = selection.count();
  QRegion region;
  for (int i = 0; i < ranges; i++) {
    QItemSelectionRange range = selection.at(i);
    for (int row = range.bottom(); row <= range.top(); row++) {
      region += itemRegion(model()->index(row, 0, rootIndex()));
    }
  }
  return region;
}

QRect PMView::visualRect(const QModelIndex &index) const {
  QRect rect = itemRegion(index).boundingRect();
  if (index.isValid()) {
    return rect
        .translated(-horizontalScrollBar()->value(),
                    -verticalScrollBar()->value())
        .normalized();
  } else {
    return rect;
  }
}

void PMView::updateGeom() {
  int rows = (int)(std::ceil((double)(model()->rowCount(rootIndex())) /
                             (double)(columnNum)));

  horizontalScrollBar()->setPageStep(viewport()->width());
  horizontalScrollBar()->setRange(
      0, qMax(0, 2 * margin + columnNum * itemWidth - viewport()->width()));
  verticalScrollBar()->setPageStep(viewport()->height());
  verticalScrollBar()->setRange(
      0, qMax(0, 2 * margin + rows * itemHeight - viewport()->height()));
}

void PMView::showFullImage(const QModelIndex &index) {
  mw->statusBar()->showMessage(tr("Processing full image. "
                                  "Please wait..."));
  QString location = model()->data(index, Qt::LocationRole).toString();
  quint64 id = model()->data(index, Qt::IdRole).toUInt();
  QPixmap pixmap;
  QString caption;
  caption += model()->data(index, Qt::NameRole).toString();
  caption += " :: ";
  caption += model()->data(index, Qt::HeaderRole).toString();

  pixmap = MainWindow::loadFullImage(id);

  if (pixmap.isNull()) {
    QMessageBox::warning(0, tr("Failed to load full preview"),
                         tr("No image. Please check your data directory"));
    mw->statusBar()->clearMessage();
    return;
  } else {
    mw->statusBar()->clearMessage();
    PMImageViewer dialog(pixmap);
    dialog.setCaption(caption);
    dialog.exec();
  }
}

void PMView::wheelEvent(QWheelEvent *event) {
  int numDegrees = event->delta() / 8;
  int numSteps = numDegrees / 15;
  verticalScrollBar()->setValue(
      (int)(verticalScrollBar()->value() -
            numSteps * (0.3 * verticalScrollBar()->pageStep())));
}

void PMView::receivedPreview(int row, QByteArray arr) {
  QModelIndex index = model()->index(row, 0);
  QPixmap pix;
  pix.loadFromData(arr, "JPG");
  if (pix.width() > pixWidth) {
    pix = pix.scaledToWidth(pixWidth, Qt::SmoothTransformation);
  }
  if (pix.height() > pixHeight) {
    pix = pix.scaledToHeight(pixHeight, Qt::SmoothTransformation);
  }
  QPixmapCache::insert(model()->data(index, Qt::IdRole).toString(), pix);

  viewport()->update(visualRect(index));
}

bool PMView::eventFilter(QObject *ob, QEvent *e) {

  //  if(ob == viewport() && e->type()==QEvent::MouseMove){
  //    QMouseEvent *me = static_cast<QMouseEvent *>(e);
  //    qApp->postEvent(this, me);
  //    return true;
  //  }
  return QAbstractItemView::eventFilter(ob, e);
}
