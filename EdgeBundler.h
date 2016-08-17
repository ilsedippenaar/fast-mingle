#ifndef MINGLEC_EDGEBUNDLER_H
#define MINGLEC_EDGEBUNDLER_H

#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include "ANN/ANN.h"
#include "EdgeBundleTree.h"


class EdgeBundler {
public:
    double width, height;
    Point center;

    EdgeBundler(const char *edgeFilename, unsigned int numNeighbors=10, float curviness=0.5f);
    EdgeBundler(const char *pointFilename, const char *adjacencyFilename, unsigned int numNeighbors=10, float curviness=0.5f);
    void doMingle();
    void setDrawLineFunction(void (*drawLineFunction)(const Point *, const Point *, const int));
    void setDrawBezierFunction(void (*drawBezierFunction)(const Point *start, const Point *ctrl1, const Point *ctrl2, const Point *end, const int weight));
    void renderLines();
    void renderBezier();

private:
    EdgeBundleTree::Edge *edges;
    unsigned int numEdges;
    unsigned int numNeighbors;
    float curviness;
    EdgeBundleTree *tree;
    void (*drawLine)(const Point *, const Point *, const int);
    void (*drawBezier)(const Point *start, const Point *ctrl1, const Point *ctrl2, const Point *end, const int weight);

    void readEdgesFromFile(const char *edgeFilename);
    void makeEdgesFromFiles(const char *pointFilename, const char *adjacencyFilename);
    void assignNeighbors();
    void makeTopEdgesMap(std::unordered_map<int, EdgeBundleTree::Edge*> *map);
    void drawEdgeLines(EdgeBundleTree::Edge *edge);
    void drawEdgeBeziers(const EdgeBundleTree::Edge *edge, const Point *sPointTo, const Point *tPointTo);
};

#endif //MINGLEC_EDGEBUNDLER_H
