#ifndef MINGLEC_POINT_H
#define MINGLEC_POINT_H

#include <cmath>
#include <vector>
#include <cassert>

static const double PHI = (1 + sqrt(5)) / 2;
static const int UNGROUPED = -1;

struct Point {
    double x, y;
    Point(double x, double y) : x(x), y(y) {}

    Point operator+ (Point& p);
    Point operator- (Point& p);
    Point operator* (int k);
    Point operator/ (double k);
    double operator* (Point& p);
    double inline norm() { return sqrt(x * x + y * y); }
    double inline sqDist(const Point& p) {
        double dx = x - p.x, dy = y - p.y;
        return dx * dx + dy * dy;
    };
};


Point lerp(Point &a, Point &b, double delta);
double costFunction(std::vector<Point>& S, std::vector<Point>& T, Point& sPoint, Point& tPoint);


#endif //MINGLEC_POINT_H
