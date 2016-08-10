#ifndef MINGLEC_EDGEBUNDLER_H
#define MINGLEC_EDGEBUNDLER_H

#include <vector>
#include <unordered_map>
#include "ANN/ANN.h"
#include "EdgeBundleTree.h"


class EdgeBundler {
    EdgeBundleTree::Edge *edges;
    unsigned int numEdges;
    const int numNeighbors;
    EdgeBundleTree *tree;
    void (*drawLine)(const Point *, const Point *, const int);

public:
    EdgeBundler(EdgeBundleTree::Edge *rawEdges, unsigned int numRawEdges, const int numNeighbors);
    void doMingle();
    void setDrawLineFunction(void (*drawLineFunction)(const Point *, const Point *, const int));
    void render();

private:
    void initialize();
    void drawEdge(EdgeBundleTree::Edge *edge);
};

#endif //MINGLEC_EDGEBUNDLER_H
