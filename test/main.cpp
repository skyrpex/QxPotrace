#include <QApplication>
#include <QxPotrace>
#include <QFileDialog>
#include <QImage>
#include <QGraphicsView>

int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  QString fileName = QFileDialog::getOpenFileName();
  if(fileName.isEmpty())
    return 0;

  QImage image(fileName);
  if(image.isNull())
    return 1;

  QxPotrace potrace;
  if(!potrace.trace(image)
     || potrace.polygons().isEmpty())
    return 1;

  QGraphicsScene scene;
  QGraphicsView view(&scene);
  foreach(const QxPotrace::Polygon &polygon, potrace.polygons())
  {
    scene.addPolygon(polygon.boundary);
    foreach(const QPolygonF &hole, polygon.holes)
      scene.addPolygon(hole, QPen(Qt::red));
  }


  view.show();
  return app.exec();
}
