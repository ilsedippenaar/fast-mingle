#include "Util.h"

Point lerp(Point& a, Point& b, double delta) {
    return {a.x * (1 - delta) + b.x * delta, a.y * (1 - delta) + b.y * delta};
}

double costFunction(std::vector<Point>& S, std::vector<Point>& T, Point& sPoint, Point& tPoint) {
    double total_dist = sqrt(sPoint.sqDist(tPoint));
    unsigned long n = S.size();
    assert(n == T.size());
    for (int i = 0; i < n; ++i) {
        const Point& xS = S[i];
        const Point& xT = T[i];
        total_dist += sqrt(sPoint.sqDist(xS)) + sqrt(tPoint.sqDist(xT));
    }
    return total_dist;
}


Point Point::operator+(Point &p) { return {x + p.x, y + p.y}; }

Point Point::operator-(Point& p) { return {x - p.x, y - p.y}; }

Point Point::operator*(int k) { return {k * x, k * y}; }

double Point::operator*(Point& p) { return x * p.x + y * p.y; }

Point Point::operator/(double k) { return {x/k, y/k}; }
