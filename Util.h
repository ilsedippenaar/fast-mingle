#ifndef MINGLEC_POINT_H
#define MINGLEC_POINT_H

#include <cmath>
#include <vector>
#include <cassert>
#include <functional>

static const double PHI = (1 + sqrt(5)) / 2;
static const int UNGROUPED = -1;

struct Point {
    double x, y;

    Point operator+ (Point& p);
    Point operator- (Point& p);
    Point operator* (int k);
    Point operator/ (double k);
    bool operator==(const Point& other) const;
    double operator* (Point& p);
    double inline norm() { return sqrt(x * x + y * y); }
    double inline sqDist(const Point& p) {
        double dx = x - p.x, dy = y - p.y;
        return dx * dx + dy * dy;
    };
};

struct PointHasher {
    std::size_t operator()(const Point& p) const;
};


Point lerp(const Point &a, const Point &b, const double delta);


#endif //MINGLEC_POINT_H
