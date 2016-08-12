#include "EdgeBundleTree.h"

int edgeId = 0;

EdgeBundleTree::Edge::Edge(Point *s, Point *t, Edge *bundle) {
    assert(s->x <= t->x);
    sPoint = s;
    tPoint = t;
    sCentroid = s;
    tCentroid = t;
    this->bundle = bundle;
    S->insert(s);
    T->insert(t);

    children = nullptr;
    ink = sqrt(t->sqDist(*s));
    weight = 1;
    grouped = false;
    id = edgeId++;
}

EdgeBundleTree::Edge::Edge(Edge *child1, Edge *child2, BundleReturn *data) {
    assert(data->s->x <= data->t->x);
    Point *points = (Point*) malloc(sizeof(Point) * 4);
    points[0] = *data->s;
    points[1] = *data->t;
    points[2] = *data->sCentroid;
    points[3] = *data->tCentroid;
    sPoint = &points[0];
    tPoint = &points[1];
    sCentroid = &points[2];
    tCentroid = &points[3];
    bundle = this;
    S->insert(child1->sPoint);
    S->insert(child2->sPoint);
    T->insert(child1->tPoint);
    T->insert(child2->tPoint);

    children = new std::vector<Edge*>();
    children->push_back(child1);
    children->push_back(child2);
    neighbors = nullptr;
    ink = data->inkUsed;
    weight = child1->weight + child2->weight;
    grouped = true;
    id = edgeId++;
}

EdgeBundleTree::EdgeBundleTree(EdgeBundleTree::Edge *edges, unsigned int numEdges)
        : edges(edges), numEdges(numEdges) {}

void EdgeBundleTree::testBundle(EdgeBundleTree::BundleReturn *bundleReturn, Edge& bundle1, Edge& bundle2) {
    int combinedWeight = bundle1.weight + bundle2.weight;
    *bundleReturn->sCentroid = {(bundle1.sCentroid->x * bundle1.weight + bundle2.sCentroid->x * bundle2.weight) / combinedWeight,
                       (bundle1.sCentroid->y * bundle1.weight + bundle2.sCentroid->y * bundle2.weight) / combinedWeight};
    *bundleReturn->tCentroid = {(bundle1.tCentroid->x * bundle1.weight + bundle2.tCentroid->x * bundle2.weight) / combinedWeight,
                       (bundle1.tCentroid->y * bundle1.weight + bundle2.tCentroid->y * bundle2.weight) / combinedWeight};
    Point& sCentroid = *bundleReturn->sCentroid;
    Point& tCentroid = *bundleReturn->tCentroid;
    Point u = tCentroid - sCentroid;
    u = u / u.norm();
    double dist, distSum = 0, maxDist = 0;
    int k = 0;
    std::unordered_set<Point*> combinedS, combinedT;
    combinedS.insert(bundle1.S->begin(), bundle1.S->end());
    combinedS.insert(bundle2.S->begin(), bundle2.S->end());
    combinedT.insert(bundle1.T->begin(), bundle1.T->end());
    combinedT.insert(bundle2.T->begin(), bundle2.T->end());
    for (auto point_ptr : combinedS) {
        Point v = *point_ptr - sCentroid;
        dist = u * v;
        maxDist = maxDist < dist ? dist : maxDist;
        distSum += dist;
        k += 1;
    }
    for (auto point_ptr : combinedT) {
        Point v = *point_ptr - tCentroid;
        dist = -(u * v);
        maxDist = maxDist < dist ? dist : maxDist;
        distSum += dist;
        k += 1;
    }
    Point delta = tCentroid - sCentroid;
    double d = delta.norm();
    double x = ((distSum + 2 * d) / (k + 4) / d);
    *bundleReturn->s = lerp(sCentroid, tCentroid, x);
    *bundleReturn->t = lerp(sCentroid, tCentroid, 1 - x);
    Point& sPoint = *bundleReturn->s;
    Point& tPoint = *bundleReturn->t;
    delta = tPoint - sPoint;
    double inkValueCombined = delta.norm();
    for (auto point_ptr : combinedS) {
        delta = *point_ptr - sPoint;
        inkValueCombined += delta.norm();
    }
    for (auto point_ptr : combinedT) {
        delta = *point_ptr - tPoint;
        inkValueCombined += delta.norm();
    }
    bundleReturn->inkUsed = inkValueCombined;
}

void EdgeBundleTree::applyBundle(EdgeBundleTree::BundleReturn& bundleReturn, EdgeBundleTree::Edge& edge1,
                                 EdgeBundleTree::Edge& edge2) {
    if (edge2.bundle->grouped) {
        // bundle2 should absorb bundle1
        Edge *bundle = edge2.bundle;
        *bundle->sPoint = *bundleReturn.s;
        *bundle->tPoint = *bundleReturn.t;
        bundle->S->insert(edge1.bundle->sPoint);
        bundle->T->insert(edge1.bundle->tPoint);
        *bundle->sCentroid = *bundleReturn.sCentroid;
        *bundle->tCentroid = *bundleReturn.tCentroid;
        bundle->children->push_back(edge1.bundle);
        bundle->weight += edge1.bundle->weight;
        bundle->ink = bundleReturn.inkUsed;
        setBundle(edge1, bundle);
    } else {
        Edge *newBundle = new Edge(&edge1, &edge2, &bundleReturn);
        setBundle(*newBundle, newBundle);
    }
}

void EdgeBundleTree::setBundle(EdgeBundleTree::Edge& edge, EdgeBundleTree::Edge *bundle) {
    edge.bundle = bundle;
    if (!edge.children) return;
    for (auto child_ptr : *edge.children) {
        setBundle(*child_ptr, bundle);
    }
}

void EdgeBundleTree::coalesceTree() {
    for (int i = 0; i < numEdges; ++i) {
        Edge& edge = edges[i];
        edge.grouped = false;
        edge.ink = sqrt(edge.tPoint->sqDist(*edge.sPoint));
    }
}

