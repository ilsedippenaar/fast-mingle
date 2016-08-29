#include "Util.h"

Point lerp(const Point& a, const Point& b, const double delta) {
    return {(b.x - a.x) * delta + a.x, (b.y - a.y) * delta + a.y};
}

Point Point::operator+(Point &p) { return {x + p.x, y + p.y}; }

Point Point::operator-(Point& p) { return {x - p.x, y - p.y}; }

Point Point::operator*(int k) { return {k * x, k * y}; }

double Point::operator*(Point& p) { return x * p.x + y * p.y; }

Point Point::operator/(double k) { return {x/k, y/k}; }

bool Point::operator==(const Point &other) const { return x == other.x && y == other.y; }

std::size_t PointHasher::operator()(const Point &p) const {
    return std::hash<double>()(p.x) ^ (std::hash<double>()(p.y) << 1);
}
