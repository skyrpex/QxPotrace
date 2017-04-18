#include "qxpotrace.h"
#include "bezier.h"
#include "potrace/potracelib.h"
#include <QDebug>
#include <QElapsedTimer>
#include <memory>

#define BM_WORDSIZE ((int)sizeof(potrace_word))
#define BM_WORDBITS (8*BM_WORDSIZE)
#define BM_HIBIT (((potrace_word)1)<<(BM_WORDBITS-1))
#define bm_scanline(bm, y) ((bm)->map + (y)*(bm)->dy)
#define bm_index(bm, x, y) (&bm_scanline(bm, y)[(x)/BM_WORDBITS])
#define bm_mask(x) (BM_HIBIT >> ((x) & (BM_WORDBITS-1)))
#define bm_range(x, a) ((int)(x) >= 0 && (int)(x) < (a))
#define bm_safe(bm, x, y) (bm_range(x, (bm)->w) && bm_range(y, (bm)->h))
#define BM_USET(bm, x, y) (*bm_index(bm, x, y) |= bm_mask(x))
#define BM_UCLR(bm, x, y) (*bm_index(bm, x, y) &= ~bm_mask(x))
#define BM_UPUT(bm, x, y, b) ((b) ? BM_USET(bm, x, y) : BM_UCLR(bm, x, y))
#define BM_PUT(bm, x, y, b) (bm_safe(bm, x, y) ? BM_UPUT(bm, x, y, b) : 0)

class PotraceBitmap
{
public:
    PotraceBitmap(int width, int height)
        : bitmap_(new potrace_bitmap_t)
    {
        bitmap_->w = width;
        bitmap_->h = height;
        bitmap_->dy = (width + BM_WORDBITS - 1) / BM_WORDBITS;
        map_.resize(bitmap_->dy * height);
        bitmap_->map = map_.data();
    }
    PotraceBitmap(PotraceBitmap&& other) = default;
    PotraceBitmap(const PotraceBitmap& other) = default;

    potrace_bitmap_t* data() const
    {
        return bitmap_.get();
    }

protected:
    std::unique_ptr<potrace_bitmap_t> bitmap_;
    std::vector<potrace_word> map_;
};

PotraceBitmap bitmapFromImage(const QImage &image, int threshold)
{
    // De momento no implementamos que la imagen no tenga alpha channel
    Q_ASSERT(image.hasAlphaChannel());

    // potrace_bitmap_t *bitmap = bm_new(image.width(), image.height());
    PotraceBitmap bitmap(image.width(), image.height());
    int pi = 0;
    for (int i = 0; i < image.byteCount(); i += 4) {
        int x = pi % image.width();
        int y = pi / image.width();
        int p;
        if (image.hasAlphaChannel()) {
            p = (image.bits()[i+3] > threshold)? 1 : 0;
        } else {
            p = (image.bits()[i] > threshold)? 0 : 1;
        }
        BM_PUT(bitmap.data(), x, y, p);

        ++pi;
    }

    return bitmap;
}

QPolygonF polygonFromPath(potrace_path_t *path, int bezierPrecision)
{
    QPolygonF poly;
    if (!path) {
        return poly;
    }

    int n = path->curve.n;
    int *tag = path->curve.tag;
    potrace_dpoint_t (*c)[3] = path->curve.c;
    for (int i = 0; i < n; ++i) {
        switch (tag[i]) {
            case POTRACE_CORNER: {
                poly << QPointF(c[i][1].x, c[i][1].y) << QPointF(c[i][2].x, c[i][2].y);
            }
            break;

            case POTRACE_CURVETO: {
                QPointF pa = poly.isEmpty() ? QPointF(c[n-1][2].x, c[n-1][2].y) : poly.last();
                QPointF pb = QPointF(c[i][0].x, c[i][0].y);
                QPointF pc = QPointF(c[i][1].x, c[i][1].y);
                QPointF pd = QPointF(c[i][2].x, c[i][2].y);
                for (int i = 1; i <= bezierPrecision; ++i) {
                    poly << bezier(pa, pb, pc, pd, static_cast<qreal>(i) / bezierPrecision);
                }
            }
            break;
        }
    }

    if (!poly.isEmpty() && poly.first() == poly.last()) {
        poly.remove(poly.size()-1);
    }

    return poly;
}

QList<QPolygonF> holesFromPath(potrace_path_t *path, int bezierPrecision)
{
    QList<QPolygonF> holes;
    if (!path) return holes;

    path = path->childlist;
    if (!path) {
        return holes;
    }

    while (path) {
        if (path->sign == '-') {
            holes << polygonFromPath(path, bezierPrecision);
        }
        path = path->sibling;
    }
    return holes;
}

QxPotrace::Polygon tracedPolygonFromPath(potrace_path_t *path, int bezierPrecision)
{
    QxPotrace::Polygon def;
    def.boundary = polygonFromPath(path, bezierPrecision);
    def.holes = holesFromPath(path, bezierPrecision);
    return def;
}

QList<QxPotrace::Polygon> tracedPolygonsFromPath(potrace_path_t *path, int bezierPrecision)
{
    QList<QxPotrace::Polygon> defs;
    while (path) {
        if (path->sign == '+') {
            defs << tracedPolygonFromPath(path, bezierPrecision);
        }
        path = path->next;
    }
    return defs;
}

QxPotrace::QxPotrace() :
    m_alphaMax(1.0),
    m_turdSize(2),
    m_curveTolerance(0.2),
    m_threshold(0),
    m_bezierPrecision(4)
{
}

bool QxPotrace::trace(const QImage &image)
{
    QElapsedTimer timer;
    timer.start();

    PotraceBitmap bitmap = bitmapFromImage(image, m_threshold);

    potrace_param_t *params = potrace_param_default();
    params->alphamax = m_alphaMax;
    params->opttolerance = m_curveTolerance;
    params->turdsize = m_turdSize;
    //  params->progress.callback = &Tracer::progress;

    potrace_state_t *st = potrace_trace(params, bitmap.data());

    potrace_param_free(params);

    if (!st || st->status != POTRACE_STATUS_OK) {
        return false;
    }

    m_meshDefs = tracedPolygonsFromPath(st->plist, m_bezierPrecision);

    potrace_state_free(st);
    return true;
}

QList<QxPotrace::Polygon> QxPotrace::polygons() const
{
    return m_meshDefs;
}

void QxPotrace::setAlphaMax(qreal alphaMax)
{
    m_alphaMax = alphaMax;
}

void QxPotrace::setTurdSize(int turdSize)
{
    m_turdSize = turdSize;
}

void QxPotrace::setCurveTolerance(qreal tolerance)
{
    m_curveTolerance = tolerance;
}

void QxPotrace::setThreshold(int threshold)
{
    m_threshold = threshold;
}

void QxPotrace::setBezierPrecision(int precision)
{
    Q_ASSERT(precision > 0);
    m_bezierPrecision = precision;
}
