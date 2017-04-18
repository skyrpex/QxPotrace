#ifndef BEZIER_H
#define BEZIER_H

#include <QPointF>

// simple linear interpolation between two points
inline QPointF lerp(const QPointF &a, const QPointF &b, qreal t)
{
    return a + (b - a) * t;
}

// evaluate a point on a bezier-curve. t goes from 0 to 1.0
inline QPointF bezier(const QPointF &a, const QPointF &b, const QPointF &c, const QPointF &d, qreal t)
{
    QPointF ab = lerp(a, b, t);           // point between a and b (green)
    QPointF bc = lerp(b, c, t);           // point between b and c (green)
    QPointF cd = lerp(c, d, t);           // point between c and d (green)
    QPointF abbc = lerp(ab, bc, t);       // point between ab and bc (blue)
    QPointF bccd = lerp(bc, cd, t);       // point between bc and cd (blue)
    return lerp(abbc, bccd, t);   // point on the bezier-curve (black)
}

#endif // BEZIER_H
