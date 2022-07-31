#include "flagtreeview.h"

PMFlagTreeView::PMFlagTreeView(PMCatModel *catModel, PMDBImageModel *imageModel,
                               MainWindow *mainWindow, QWidget *parent)
    : QTreeView(parent) {
  needsUpdate = true;
  foundId = 0;
  setModel(catModel);
  updateMap();

  this->catModel = catModel;
  this->imageModel = imageModel;
  this->mainWindow = mainWindow;

  initConnections();
}

void PMFlagTreeView::paintEvent(QPaintEvent *event) {
  QTreeView::paintEvent(event);

  QPainter painter(viewport());
  QStyleOptionViewItem option = viewOptions();
  QBrush background = option.palette.color(QPalette::Window);
  QPen pen(Qt::NoPen);
  painter.setBrush(background);
  painter.setPen(pen);

  for (int i = 0; i < model()->rowCount(rootIndex()); i++) {
    QModelIndex currIndex = model()->index(i, 0, rootIndex());
    drawRectForIndex(painter, currIndex);
  }
}

void PMFlagTreeView::updateMap() {
  populateCheckedIdLists();

  qDeleteAll(mapIdToBox);
  mapIdToBox.clear();

  for (int i = 0; i < model()->rowCount(rootIndex()); i++) {
    QModelIndex currIndex = model()->index(i, 0, rootIndex());
    updateMap2(currIndex);
  }
  update();
}

void PMFlagTreeView::updateMap2(const QModelIndex &index) {
  quint64 catId = model()->data(index, Qt::CatIdRole).toUInt();
  PMQCheckBox *box = new PMQCheckBox(index, catId, viewport());
  if (MainWindow::isEditor()) {
    connect(box, SIGNAL(pmStateChanged(QModelIndex, quint64, int)), this,
            SLOT(submitBox(QModelIndex, quint64, int)));
  } else {
    box->setEnabled(false);
  }

  box->hide();
  if (MainWindow::isClient()) {
    box->setEnabled(false);
  }
  mapIdToBox.insert(catId, box);

  if (grayedBoxIds.contains(catId)) {
    box->setTristate(true);
  }

  if (grayCheckedCatIds.contains(catId)) {
    box->setCheckState(Qt::PartiallyChecked);
  } else if (checkedCatIds.contains(catId)) {
    box->setCheckState(Qt::Checked);
  } else {
    box->setCheckState(Qt::Unchecked);
  }
  for (int i = 0; i < model()->rowCount(index); i++) {
    QModelIndex currIndex = model()->index(i, 0, index);
    updateMap2(currIndex);
  }
}

void PMFlagTreeView::drawRectForIndex(QPainter &painter, QModelIndex &index) {
  //  qDebug() << "drawREcts";
  QRect rect = visualRect(index);
  quint64 id = model()->data(index, Qt::CatIdRole).toUInt();
  PMQCheckBox *box = mapIdToBox.value(id);
  if (rect.isValid()) {
    box->setGeometry(rect.x() + rect.width() - (int)(rect.height() * 1.5),
                     rect.y() + (rect.height() - box->height()) / 2 + 1,
                     rect.height(), rect.height());
    painter.drawRect(rect.x() + rect.width() - (int)(rect.height() * 1.5),
                     rect.y() + (rect.height() - box->height()) / 2 + 1,
                     box->width(), box->height());
    box->show();
  } else {
    box->hide();
  }

  if (id == foundId) {
    painter.save();
    painter.setBrush(QBrush(QColor(Qt::red)));
    painter.drawRect(rect.x() + rect.width() - (int)(rect.height() * 3),
                     rect.y() + (rect.height() - box->height() / 2) / 2 + 1,
                     box->width() / 2, box->height() / 2);
    painter.restore();
    foundId = 0;
  }

  for (int i = 0; i < model()->rowCount(index); i++) {
    QModelIndex currIndex = model()->index(i, 0, index);
    drawRectForIndex(painter, currIndex);
  }
}

void PMFlagTreeView::initConnections() {
  connect(catModel, SIGNAL(rowsInserted(QModelIndex, int, int)), this,
          SLOT(updateMap()));
  connect(catModel, SIGNAL(rowsRemoved(QModelIndex, int, int)), this,
          SLOT(updateMap()));
  connect(catModel, SIGNAL(modelReset()), this, SLOT(updateMap()));
}

void PMFlagTreeView::imageSelectionChanged() {
  needsUpdate = false;
  currSelectedImages = mainWindow->imageSelectionModel()->selectedIndexes();
  if (currSelectedImages.isEmpty()) {

  } else {
    checkedCatIds.clear();
    grayedBoxIds.clear();
    grayCheckedCatIds.clear();
    presentCats.clear();
    collapseAll();

    QSet<quint64> allPresentCats;

    foreach (QModelIndex index, currSelectedImages) {
      QList<quint64> list = PMDBImageModel::qListFromQVariant(
          imageModel->data(index, Qt::PidRole));
      QSet<quint64> toAdd = QSet<quint64>::fromList(list);
      presentCats.insert(index, toAdd);
      allPresentCats += toAdd;
    }

    foreach (quint64 catId, allPresentCats) {
      if (catId != 0) {
        grayedBoxIds.insert(catId);
        grayCheckedCatIds.insert(catId);
        scrollTo(catModel->findCategory(catId));
      }
    }

    updateBoxesState();
  }
}

void PMFlagTreeView::updateBoxesState() {
  foreach (PMQCheckBox *box, mapIdToBox) {
    quint64 id = box->id();
    box->setTristate(grayedBoxIds.contains(id) ? true : false);
    if (checkedCatIds.contains(id)) {
      box->setCheckState(Qt::Checked);
    } else if (grayCheckedCatIds.contains(id)) {
      box->setCheckState(Qt::PartiallyChecked);
    } else {
      box->setCheckState(Qt::Unchecked);
    }
  }
}

void PMFlagTreeView::populateCheckedIdLists() {
  checkedCatIds.clear();
  grayedBoxIds.clear();
  grayCheckedCatIds.clear();

  foreach (PMQCheckBox *box, mapIdToBox) {
    quint64 id = box->id();
    if (box->isTristate()) {
      grayedBoxIds.insert(id);
    }

    if (box->checkState() == Qt::Checked) {
      checkedCatIds.insert(id);
    } else if (box->checkState() == Qt::PartiallyChecked) {
      grayCheckedCatIds.insert(id);
    }
  }
}

void PMFlagTreeView::submitChanges() {
  QSet<quint64> imagePids;
  populateCheckedIdLists();

  mainWindow->statusBar()->showMessage(tr("Updating images..."));
  foreach (QModelIndex index, currSelectedImages) {
    QList<quint64> imagePidsList =
        PMDBImageModel::qListFromQVariant(imageModel->data(index, Qt::PidRole));
    imagePids = QSet<quint64>::fromList(imagePidsList);

    imagePids &= grayCheckedCatIds;
    imagePids += checkedCatIds;
    if (imagePids.isEmpty()) {
      imagePids.insert(0);
    }
    imagePidsList = imagePids.toList();
    imageModel->setData(index, PMDBImageModel::qVariantFromQList(imagePidsList),
                        Qt::PidRole);
  }
  mainWindow->statusBar()->clearMessage();

  needsUpdate = true;
  //  imageModel->removeOutdatedImages(mainWindow->imageSelectionModel());
  //  if(needsUpdate){
  //    imageSelectionChanged();
  //  }
}

void PMFlagTreeView::submitBox(const QModelIndex &catIndex, quint64 id,
                               int state) {
  mainWindow->statusBar()->showMessage(tr("Updating images..."));
  int deltaAmount = 0;
  foreach (QModelIndex index, currSelectedImages) {
    QList<quint64> imagePidsList =
        PMDBImageModel::qListFromQVariant(imageModel->data(index, Qt::PidRole));
    QSet<quint64> imagePids = QSet<quint64>::fromList(imagePidsList);

    bool containsIdBefore =
        imagePids.contains(id); // for changing amount in catView

    if (state == Qt::Checked) {
      imagePids += id;
    } else if (state == Qt::Unchecked) {
      imagePids -= id;
    } else if (state == Qt::PartiallyChecked) {
      if (presentCats.value(index).contains(id)) {
        imagePids += id;
      } else {
        imagePids -= id;
      }
    }

    if (imagePids.isEmpty()) {
      imagePids.insert(0);
    } else if (imagePids.count() > 1) {
      imagePids -= 0;
    }

    bool containsIdAfter = imagePids.contains(id);
    if (!containsIdBefore && containsIdAfter)
      deltaAmount += 1;
    else if (!containsIdAfter && containsIdBefore)
      deltaAmount -= 1;
    else {
    }

    imagePidsList = imagePids.toList();
    imageModel->setData(index, PMDBImageModel::qVariantFromQList(imagePidsList),
                        Qt::PidRole);
  }
  int catImgAmount =
      mWin->modelCategory()->data(catIndex, Qt::AmountRole).toInt();
  mWin->modelCategory()->setData(catIndex, QVariant(catImgAmount + deltaAmount),
                                 Qt::AmountRole);
  mainWindow->statusBar()->clearMessage();
}

void PMFlagTreeView::highlightCat(quint64 id) {
  scrollTo(catModel->findCategory(id));
  foundId = id;
  setCurrentIndex(catModel->findCategory(id));
}

/*#####################################################################################################
#################################### PMFlagTreeWidget
#################################################
#####################################################################################################*/

PMFlagTreeWidget::PMFlagTreeWidget(PMCatModel *catModel,
                                   PMDBImageModel *imageModel,
                                   MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent) {
  tree = new PMFlagTreeView(catModel, imageModel, mainWindow, this);
  this->mainWindow = mainWindow;

  QVBoxLayout *layout = new QVBoxLayout;
  QHBoxLayout *searchLayout = new QHBoxLayout;
  QHBoxLayout *buttonLayout = new QHBoxLayout;
  QVBoxLayout *searchGroupLayout = new QVBoxLayout;
  QGroupBox *group = new QGroupBox(tr("Search categories by word"));

  searchLabel = new QLabel(tr("Search: "));
  searchEdit = new QLineEdit;
  searchButton = new QPushButton(tr("Search"));
  nextButton = new QPushButton(tr("Next"));

  searchLayout->addWidget(searchLabel, 1);
  searchLayout->addStretch();
  searchLayout->addWidget(searchEdit, 4);
  buttonLayout->addWidget(searchButton);
  buttonLayout->addStretch();
  buttonLayout->addWidget(nextButton);

  layout->addWidget(tree);
  searchGroupLayout->addLayout(searchLayout);
  searchGroupLayout->addLayout(buttonLayout);
  group->setLayout(searchGroupLayout);
  layout->addWidget(group);

  setLayout(layout);

  //  tree->setSelectionMode(QAbstractItemView::NoSelection);
  tree->setUniformRowHeights(true);
  tree->setColumnWidth(0, 200);
  tree->setAlternatingRowColors(true);
  tree->header()->setStretchLastSection(true);

  connect(searchButton, SIGNAL(clicked()), this, SLOT(searchByName()));
  connect(nextButton, SIGNAL(clicked()), this, SLOT(showNextFound()));

  tree->setEnabled(false);
}

void PMFlagTreeWidget::imageSelectionChanged() {
  QModelIndexList currSelectedImages =
      mainWindow->imageSelectionModel()->selectedIndexes();
  if (currSelectedImages.isEmpty()) {
    tree->setEnabled(false);
  } else {
    tree->setEnabled(true);
    tree->imageSelectionChanged();
  }
}

void PMFlagTreeWidget::searchByName() {
  searchResults.clear();

  QString str = searchEdit->text().simplified();
  QSqlQuery query;

  str.remove(QRegExp("[.,!\\?-_\\+\\';:\\*\\\\\\a\\n\\r]"));
  QStringList searchList = str.split(" ", QString::SkipEmptyParts);
  str = searchList.join("|");

  query.clear();
  query.exec(
      "SELECT id FROM pm_categories WHERE lower(name) @@ to_tsquery(lower('" +
      str + "'))");
  while (query.next()) {
    if (!searchResults.contains(query.value(0).toUInt())) {
      searchResults.append(query.value(0).toUInt());
    }
  }

  if (searchResults.isEmpty()) {
    QStringList list = str.split("|");
    str = "";
    foreach (QString s, list) {
      if (!str.isEmpty()) {
        str += " OR ";
      }
      str += " lower(name) ~ lower('" + s + "') ";
    }
    query.clear();
    query.exec("SELECT id FROM pm_categories WHERE " + str);
    while (query.next()) {
      if (!searchResults.contains(query.value(0).toUInt())) {
        searchResults.append(query.value(0).toUInt());
      }
    }
  }

  if (searchResults.isEmpty()) {
    return;
  }

  searchIter = 0;

  highlightCat(searchIter);
}

void PMFlagTreeWidget::highlightCat(int i) {
  if (i < 0 || i >= searchResults.count()) {
    return;
  } else {
    quint64 id = searchResults.at(i);
    treeView()->highlightCat(id);
  }
}

void PMFlagTreeWidget::showNextFound() {
  if (searchResults.isEmpty()) {
    return;
  }
  searchIter++;
  if (searchIter >= searchResults.count()) {
    searchIter = 0;
    mainWindow->statusBar()->showMessage(
        tr("Reached the end of list. Starting from the beginning."), 4000);
  }
  highlightCat(searchIter);
}

/*#####################################################################################################
######################################### PMQCheckBox
#################################################
#####################################################################################################*/

PMQCheckBox::PMQCheckBox(const QModelIndex &catIndex, quint64 catId,
                         QWidget *parent)
    : QCheckBox(parent) {
  this->catId = catId;
  this->catIndex = catIndex;
  connect(this, SIGNAL(clicked()), this, SLOT(emitClicked()));
}

void PMQCheckBox::emitClicked() {
  int state = checkState();
  emit pmStateChanged(catIndex, catId, state);
}
