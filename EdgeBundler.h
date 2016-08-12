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
    void setDrawBezierFunction(void (*drawBezierFunction)(const ControlPoints *data));
    void setDrawLineFunction(void (*drawLineFunction)(const Point *, const Point *, const int));
    void renderLines();
    void renderBezier();

private:
    EdgeBundleTree::Edge *edges;
    const unsigned int numEdges;
    const unsigned int numNeighbors;
    const float curviness;
    EdgeBundleTree *tree;
    void (*drawLine)(const Point *, const Point *, const int);
    void (*drawBezier)(const ControlPoints *);

    void initialize();
    void drawEdgeLines(EdgeBundleTree::Edge *edge);
};

#endif //MINGLEC_EDGEBUNDLER_H
