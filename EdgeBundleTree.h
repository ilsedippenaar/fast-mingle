#ifndef MINGLEC_EDGEBUNDLETREE_H
#define MINGLEC_EDGEBUNDLETREE_H

#include <vector>
#include <unordered_set>
#include <cmath>
#include <cassert>
#include "Util.h"

class EdgeBundleTree {

public:
    struct BundleReturn {
        Point s, t, sCentroid, tCentroid;
        double inkUsed;
    };

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
        Edge(Edge *child1, Edge *child2, const BundleReturn *data);  // for parent nodes
    };

    Edge *edges;
    unsigned int numEdges;

    EdgeBundleTree(Edge *edges, unsigned int numEdges);

    static void testBundle(EdgeBundleTree::BundleReturn *bundleReturn, const Edge &bundle1, const Edge &bundle2);
    static void applyBundle(const BundleReturn& bundleReturn, Edge& edge1, Edge& edge2);
    void coalesceTree();

private:
    static void setBundle(Edge& edge, Edge *bundle);
};


#endif //MINGLEC_EDGEBUNDLETREE_H
