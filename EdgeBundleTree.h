#ifndef MINGLEC_EDGEBUNDLETREE_H
#define MINGLEC_EDGEBUNDLETREE_H

#include <vector>
#include <unordered_set>
#include <cmath>
#include <cassert>
#include "Util.h"

class EdgeBundleTree {

public:
    struct Edge {
        std::unordered_set<Point*> *S = new std::unordered_set<Point*>(), *T = new std::unordered_set<Point*>();
        Point *sPoint, *tPoint;
        Point *sCentroid, *tCentroid;
        Edge *bundle;  // highest parent for this edge
        std::vector<Edge*> *children;
        Edge **neighbors;
        double ink;
        int weight;
        bool grouped;
        int id;

        Edge(Point *s, Point *t, Edge *bundle);  // for leaf nodes
        Edge(Point *s, Point *t, Point *sCentroid, Point *tCentroid, Edge **children, double inkValue);  // for parent nodes
    };

    struct BundleReturn {
        Point *s, *t, *sCentroid, *tCentroid;
        double inkUsed;
    };

    Edge *edges;
    unsigned int numEdges;

    EdgeBundleTree(Edge *edges, unsigned int numEdges);

    static double getInkValueFromPoints(Edge& edge0, Edge& edge1, Point& sPoint, Point& tPoint);
    static void testBundle(EdgeBundleTree::BundleReturn *bundleReturn, Edge &bundle1, Edge &bundle2);
    static void applyBundle(BundleReturn& bundleReturn, Edge& edge1, Edge& edge2);
    void coalesceTree();

private:
    double goldenSectionSearch(Edge& node0, Edge& node1);
    static void setBundle(Edge& edge, Edge *bundle);
};


#endif //MINGLEC_EDGEBUNDLETREE_H
