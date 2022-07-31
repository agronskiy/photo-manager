#include <QtGui>

#include "invmodel.h"
#include "metainfowidget.h"

PMMetaInfoWidget::PMMetaInfoWidget(MainWindow *mainWindow, QWidget *parent)
    : QDialog(parent) {
  setFocusPolicy(Qt::StrongFocus);

  imageModel = mainWindow->modelImage();
  parentWindow = mainWindow;

  nameEditLabel = new QLabel(tr("File name"));
  headerEditLabel = new QLabel(tr("Header"));
  descriptionEditLabel = new QLabel(tr("Description"));
  dateEditLabel = new QLabel(tr("Date"));
  inventaryEditLabel = new QLabel(tr("Folder"));
  authorEditLabel = new QLabel(tr("Author"));
  invAuthorLabel = new QLabel(tr("Author"));
  invAuthorLabel->setMinimumWidth(60);
  typeLabel = new QLabel(tr("Type"));
  typeLabel->setMinimumWidth(60);

  idLabel = new QLabel(tr("ID"));
  cdLabel = new QLabel(tr("Subfolder"));

  QVBoxLayout *generalLayout = new QVBoxLayout();
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QHBoxLayout *idLayout = new QHBoxLayout;
  QHBoxLayout *inventaryLayout = new QHBoxLayout;
  QHBoxLayout *invAuthorLayout = new QHBoxLayout;
  QHBoxLayout *inv2Layout = new QHBoxLayout;
  QVBoxLayout *vertInvLayout = new QVBoxLayout;
  QHBoxLayout *nameLayout = new QHBoxLayout;
  QHBoxLayout *headerLayout = new QHBoxLayout;
  QHBoxLayout *dateLayout = new QHBoxLayout;
  QVBoxLayout *descriptionLayout = new QVBoxLayout;
  QVBoxLayout *categLayout = new QVBoxLayout;
  QGroupBox *descriptionBox = new QGroupBox(tr("Description"));
  QGroupBox *categBox = new QGroupBox(tr("Assigned to"));
  QGroupBox *generalBox = new QGroupBox(tr("General info"));
  QHBoxLayout *buttonGroup = new QHBoxLayout;
  //  QHBoxLayout *locationLayout = new QHBoxLayout;

  QGroupBox *invBox = new QGroupBox(tr("Corresponding folder"));

  categoriesEdit = new QTextEdit();
  categoriesEdit->setReadOnly(true);
  categoriesEdit->setFixedHeight(80);
  nameEdit = new QLineEdit;
  nameEdit->setReadOnly(true);
  headerEdit = new QTextEdit;
  headerEdit->setFixedHeight(50);
  descriptionEdit = new QTextEdit;
  dateEdit = new QDateEdit;
  //  locationEdit = new QLineEdit;
  //  locationEdit->setReadOnly(true);
  inventaryEdit = new QLineEdit;
  inventaryEdit->setReadOnly(true);
  typeEdit = new QLineEdit;
  typeEdit->setReadOnly(true);
  authorEdit = new QLineEdit;
  authorEdit->setReadOnly(true);
  invAuthorEdit = new QLineEdit;
  invAuthorEdit->setReadOnly(true);
  authorEdit->setReadOnly(true);
  idEdit = new QLineEdit;
  idEdit->setReadOnly(true);
  idEdit->setMinimumWidth(50);
  cdEdit = new QLineEdit;
  cdEdit->setReadOnly(true);
  cdEdit->setMinimumWidth(65);
  appendDescriptionButton =
      new QRadioButton(tr("Append the new description to the old one"), this);
  replaceDescriptionButton = new QRadioButton(
      tr("Replace the old description with the new one"), this);
  appendDescriptionButton->setChecked(true);
  submitButton = new QPushButton(tr("Submit"));
  submitButton->setAutoDefault(true);
  submitButton->setDefault(true);
  resetButton = new QPushButton(tr("Refresh"));

  changeDateFlag = new QCheckBox(tr("Change date for all selected images"));
  forUsBox = new QCheckBox(tr("Only for us"));
  forUsBox->setEnabled(false);

  if (MainWindow::isClient()) {
    submitButton->setEnabled(false);
    headerEdit->setReadOnly(true);
    descriptionEdit->setReadOnly(true);
    dateEdit->setReadOnly(true);
    changeDateFlag->setEnabled(false);
    appendDescriptionButton->setEnabled(false);
    replaceDescriptionButton->setEnabled(false);
  }

  dateLayout->addWidget(dateEditLabel);
  dateLayout->addStretch();
  dateLayout->addWidget(dateEdit);

  descriptionLayout->addWidget(descriptionEdit);
  descriptionLayout->addWidget(appendDescriptionButton);
  descriptionLayout->addWidget(replaceDescriptionButton);
  descriptionBox->setLayout(descriptionLayout);

  buttonGroup->addWidget(submitButton);
  buttonGroup->addStretch();
  buttonGroup->addWidget(resetButton);

  nameLayout->addWidget(nameEditLabel);
  nameLayout->addWidget(nameEdit);

  idLayout->addWidget(idLabel);
  idLayout->addStretch();
  idLayout->addWidget(idEdit);

  inventaryLayout->addWidget(inventaryEditLabel);
  inventaryLayout->addWidget(inventaryEdit);
  inventaryLayout->addStretch();
  inventaryLayout->addWidget(cdLabel);
  inventaryLayout->addWidget(cdEdit);

  invAuthorLayout->addWidget(invAuthorLabel);
  invAuthorLayout->addWidget(invAuthorEdit);

  inv2Layout->addWidget(typeLabel);
  inv2Layout->addWidget(typeEdit);

  vertInvLayout->addLayout(inventaryLayout);
  vertInvLayout->addLayout(inv2Layout);
  vertInvLayout->addLayout(invAuthorLayout);
  vertInvLayout->addWidget(forUsBox);
  invBox->setLayout(vertInvLayout);

  categLayout->addWidget(categoriesEdit);
  categBox->setLayout(categLayout);

  headerLayout->addWidget(headerEditLabel);
  headerLayout->addWidget(headerEdit);

  //  locationLayout->addWidget(locationEditLabel);
  //  locationLayout->addWidget(locationEdit);

  generalLayout->addLayout(idLayout);
  //  generalLayout->addLayout(locationLayout);
  generalLayout->addLayout(nameLayout);
  generalLayout->addLayout(headerLayout);
  generalLayout->addLayout(dateLayout);
  generalLayout->addWidget(changeDateFlag);
  generalBox->setLayout(generalLayout);

  mainLayout->addWidget(generalBox);
  mainLayout->addWidget(descriptionBox);
  mainLayout->addWidget(categBox);
  mainLayout->addWidget(invBox);
  mainLayout->addLayout(buttonGroup);
  mainLayout->addStretch();

  connect(submitButton, SIGNAL(clicked()), this, SLOT(submitChanges()));
  connect(resetButton, SIGNAL(clicked()), this, SLOT(imageSelectionChanged()));

  this->setEnabled(false);
}

void PMMetaInfoWidget::imageSelectionChanged() {
  currSelected = parentWindow->imageSelectionModel()->selectedIndexes();
  if (currSelected.isEmpty()) {
    changeDateFlag->show();
    idEdit->clear();
    cdEdit->clear();
    nameEdit->clear();
    headerEdit->clear();
    //    locationEdit->setText(tr("[Multiple]"));
    categoriesEdit->clear();
    inventaryEdit->clear();
    typeEdit->clear();
    invAuthorEdit->clear();
    descriptionEdit->clear();
    dateEdit->setDate(QDate(1, 1, 1970));
    this->setEnabled(false);
    mWin->sizeLabel->clear();
    mWin->dimensionsLabel->clear();
    mWin->dpiLabel->clear();
    mWin->nameLabel->clear();
  } else if (currSelected.count() > 1) {
    changeDateFlag->show();
    changeDateFlag->setCheckState(Qt::Unchecked);

    this->setEnabled(true);
    idEdit->setText("[#]");
    cdEdit->setText("[#]");
    nameEdit->setText(tr("[Multiple]"));
    categoriesEdit->setText(tr("[Multiple]"));
    headerEdit->clear();
    //    locationEdit->setText(tr("[Multiple]"));
    inventaryEdit->setText(tr("[##]"));
    typeEdit->setText(tr("[##]"));
    invAuthorEdit->setText(tr("[Multiple]"));
    descriptionEdit->clear();
    dateEdit->setDate(QDate(0, 0, 0));
    appendDescriptionButton->setChecked(true);

    mWin->sizeLabel->clear();
    mWin->dimensionsLabel->clear();
    mWin->dpiLabel->clear();
    mWin->nameLabel->clear();
  } else {
    changeDateFlag->hide();
    this->setEnabled(true);
    QModelIndex index = currSelected.first();

    idEdit->setText(
        QString::number(imageModel->data(index, Qt::IdRole).toUInt()));
    QString cd = imageModel->data(index, Qt::CdRole).toString();
    cdEdit->setText(cd);
    nameEdit->setText(imageModel->data(index, Qt::NameRole).toString());
    headerEdit->setText(imageModel->data(index, Qt::HeaderRole).toString());
    //    locationEdit->setText(tr("DATA_DIRECTORY") + "/" +
    //    imageModel->data(index, Qt::LocationRole).toString());

    dateEdit->setDate(imageModel->data(index, Qt::DateRole).toDate());
    descriptionEdit->setText(
        imageModel->data(index, Qt::DescriptionRole).toString());
    replaceDescriptionButton->setChecked(true);

    QList<quint64> pidList =
        PMDBImageModel::qListFromQVariant(imageModel->data(index, Qt::PidRole));
    QString categ = "";
    foreach (quint64 id, pidList) {
      QModelIndex catInd =
          mWin->modelCategory()->findCategory(id, QModelIndex());
      QString s = mWin->modelCategory()->pathFromRoot(catInd, "/");
      if (!s.isEmpty()) {
        categ = categ + "\"" + s + "\"; ";
      }
    }
    categoriesEdit->setText(categ);

    QSqlQuery q;
    q.exec("SELECT name, type, author, internal_usage FROM pm_inv WHERE id=" +
           imageModel->data(index, Qt::InventaryRole).toString());
    if (q.next()) {
      QSqlRecord rec = q.record();
      PMInv inv = PMInv::sqlRecordToInv(rec);
      inventaryEdit->setText(inv.name);
      typeEdit->setText(PMInv::typeList.at(inv.type));
      invAuthorEdit->setText(inv.author);
      forUsBox->setCheckState(inv.internal == true ? Qt::Checked
                                                   : Qt::Unchecked);
    }

    int x = imageModel->data(index, Qt::OrigWidthRole).toInt();
    int y = imageModel->data(index, Qt::OrigHeightRole).toInt();
    if (x && y) {
      mWin->dimensionsLabel->setText(QString::number(x) + " x " +
                                     QString::number(y));
    } else {
      mWin->dimensionsLabel->setText("N/A");
    }

    double d = imageModel->data(index, Qt::OrigSizeRole).toDouble();
    QString orSize;
    if (d > 1E-7) {
      orSize.setNum(d, 'f', 1);
    } else {
      orSize = "N/A";
    }
    mWin->sizeLabel->setText(orSize + " " + "KB");
    mWin->dpiLabel->setText(imageModel->data(index, Qt::DpiRole).toString());
    mWin->nameLabel->setText(imageModel->data(index, Qt::NameRole).toString());
  }
}

void PMMetaInfoWidget::submitChanges() {
  if (headerEdit->toPlainText().trimmed().isEmpty() &&
      currSelected.count() == 1) {
    QMessageBox::warning(0, tr("Header is empty!"),
                         tr("Please fill \"Header\" field"));
    return;
  }

  parentWindow->statusBar()->showMessage(tr("Updating images..."));
  for (int i = 0; i < currSelected.count(); i++) {
    QModelIndex index = currSelected.at(i);
    if (!headerEdit->toPlainText().trimmed().isEmpty()) {
      imageModel->setData(index, QVariant(headerEdit->toPlainText()),
                          Qt::HeaderRole);
    }

    if (currSelected.count() == 1) {
      imageModel->setData(index, QVariant(dateEdit->date()), Qt::DateRole);
    } else if (changeDateFlag->checkState() == Qt::Checked) {
      imageModel->setData(index, QVariant(dateEdit->date()), Qt::DateRole);
    }

    if (appendDescriptionButton->isChecked() &&
        !descriptionEdit->toPlainText().isEmpty()) {
      imageModel->setData(
          index,
          QVariant(imageModel->data(index, Qt::DescriptionRole).toString() +
                   "\n" + descriptionEdit->toPlainText()),
          Qt::DescriptionRole);
    } else if (replaceDescriptionButton->isChecked()) {
      imageModel->setData(index, QVariant(descriptionEdit->toPlainText()),
                          Qt::DescriptionRole);
    }
  }
  imageSelectionChanged();
  parentWindow->statusBar()->clearMessage();
}
