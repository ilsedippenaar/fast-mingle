#ifndef MINGLEC_EDGEBUNDLER_H
#define MINGLEC_EDGEBUNDLER_H

#include <vector>
#include <unordered_map>
#include "ANN/ANN.h"
#include "EdgeBundleTree.h"


class EdgeBundler {
public:
    typedef double ControlPoints[4][3];
    EdgeBundler(EdgeBundleTree::Edge *rawEdges, unsigned int numRawEdges, const unsigned int numNeighbors, float curviness=0.5f);
    void doMingle();
    void setDrawLineFunction(void (*drawLineFunction)(const Point *, const Point *, const int));
    void setDrawBezierFunction(void (*drawBezierFunction)(const Point *start, const Point *ctrl1, const Point *ctrl2, const Point *end, const int weight));
    void renderLines();
    void renderBezier();

private:
    EdgeBundleTree::Edge *edges;
    const unsigned int numEdges;
    const unsigned int numNeighbors;
    const float curviness;
    EdgeBundleTree *tree;
    void (*drawLine)(const Point *, const Point *, const int);
    void (*drawBezier)(const Point *start, const Point *ctrl1, const Point *ctrl2, const Point *end, const int weight);

    void initialize();
    void makeTopEdgesMap(std::unordered_map<int, EdgeBundleTree::Edge*> *map);
    void drawEdgeLines(EdgeBundleTree::Edge *edge);
    void drawEdgeBeziers(const EdgeBundleTree::Edge *edge, const Point *sPointTo, const Point *tPointTo);
};

#endif //MINGLEC_EDGEBUNDLER_H
