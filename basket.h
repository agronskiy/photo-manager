#ifndef BASKET_H_INCLUDED
#define BASKET_H_INCLUDED

class PMBasket : public QObject {
  Q_OBJECT
public:
protected:
  QList<PMDBImage> imageList;
};

#endif // BASKET_H_INCLUDED
