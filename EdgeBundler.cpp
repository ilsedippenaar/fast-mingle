#include "EdgeBundler.h"

EdgeBundler::EdgeBundler(EdgeBundleTree::Edge *rawEdges, unsigned int numRawEdges, const unsigned int numNeighbors, float curviness)
        : edges(rawEdges), numEdges(numRawEdges), numNeighbors(numNeighbors), curviness(curviness / 2.0f) {
    initialize();
}

void EdgeBundler::initialize() {
    const int dim = 4;

    ANNpoint queryPoint;
    ANNpointArray points;
    ANNidxArray indices;
    ANNdistArray dists;
    ANNkd_tree *kdTree;

    queryPoint = annAllocPt(dim);
    points = annAllocPts(numEdges, dim);
    indices = new ANNidx[numNeighbors];
    dists = new ANNdist[numNeighbors];

    for (int i = 0; i < numEdges; ++i) {
        ANNpoint p = points[i];
        EdgeBundleTree::Edge& edge = edges[i];
        p[0] = edge.sPoint->x;
        p[1] = edge.sPoint->y;
        p[2] = edge.tPoint->x;
        p[3] = edge.tPoint->y;
    }
    kdTree = new ANNkd_tree(points, numEdges, dim);

    EdgeBundleTree::Edge **neighbors = (EdgeBundleTree::Edge**) malloc(sizeof(EdgeBundleTree::Edge*) * numEdges * numNeighbors);
    for (unsigned int i = 0; i < numEdges; ++i) {
        EdgeBundleTree::Edge& edge = edges[i];
        queryPoint[0] = edge.sPoint->x;
        queryPoint[1] = edge.sPoint->y;
        queryPoint[2] = edge.tPoint->x;
        queryPoint[3] = edge.tPoint->y;
        kdTree->annkSearch(queryPoint, numNeighbors, indices, dists);
        for (int j = 0; j < numNeighbors; ++j) {
            neighbors[i*numNeighbors + j] = &edges[indices[j]];
        }
        edge.neighbors = &neighbors[i*numNeighbors];
    }
    tree = new EdgeBundleTree(edges, numEdges);
    delete indices, dists, kdTree;
}

void EdgeBundler::doMingle() {
    bool inkWasSaved;
    EdgeBundleTree::BundleReturn bundleReturnArray[numNeighbors];
    Point *points = (Point*) malloc(sizeof(Point) * numNeighbors * 4);
    for (int i = 0; i < numNeighbors; ++i) {
        bundleReturnArray[i].s = &points[i*4];
        bundleReturnArray[i].t = &points[i*4 + 1];
        bundleReturnArray[i].sCentroid = &points[i*4 + 2];
        bundleReturnArray[i].tCentroid = &points[i*4 + 3];
    }
    do {
        inkWasSaved = false;
        for (unsigned long i = 0; i < tree->numEdges; ++i) {
            EdgeBundleTree::Edge& edge = tree->edges[i];
            if (!edge.bundle->grouped) {
                double maxInkSaved = -INFINITY;
                int maxSavingNeighborIndex = 0;
                for (int j = 0; j < numNeighbors; ++j) {
                    EdgeBundleTree::Edge& neighbor = *edge.neighbors[j];
                    if (edge.bundle == neighbor.bundle) continue;
                    EdgeBundleTree::testBundle(&bundleReturnArray[j], *edge.bundle, *neighbor.bundle);
                    EdgeBundleTree::BundleReturn& bundleReturn = bundleReturnArray[j];
                    double inkSaved = edge.bundle->ink + neighbor.bundle->ink - bundleReturn.inkUsed;
                    if (inkSaved > maxInkSaved) {
                        maxInkSaved = inkSaved;
                        maxSavingNeighborIndex = j;
                    }
                }
                if (maxInkSaved > 0) {
                    EdgeBundleTree::applyBundle(bundleReturnArray[maxSavingNeighborIndex],
                                                edge,
                                                *edge.neighbors[maxSavingNeighborIndex]);
                    inkWasSaved = true;
                }
            }
        }
        tree->coalesceTree();
    } while (inkWasSaved);
}




void EdgeBundler::setDrawLineFunction(void (*drawLineFunction)(const Point *, const Point *, const int)) {
    drawLine = drawLineFunction;
}

void EdgeBundler::setDrawBezierFunction(void (*drawBezierFunction)(const ControlPoints *)) {
    drawBezier = drawBezierFunction;
}

void EdgeBundler::renderLines() {
    auto topEdgeSet = new std::unordered_map<int, EdgeBundleTree::Edge*>();
    for (int i = 0; i < numEdges; ++i) {
        (*topEdgeSet)[edges[i].bundle->id] = edges[i].bundle;
    }
    for (auto pair : *topEdgeSet) {
        EdgeBundleTree::Edge *bundle = pair.second;
        drawEdgeLines(bundle);
        drawLine(bundle->sPoint, bundle->tPoint, bundle->weight);
    }
}

void EdgeBundler::drawEdgeLines(EdgeBundleTree::Edge *edge) {
    if (!edge->children) return;
    for (EdgeBundleTree::Edge *child : *edge->children) {
        drawEdgeLines(child);
        drawLine(edge->sPoint, child->sPoint, child->weight);
        drawLine(edge->tPoint, child->tPoint, child->weight);
    }
}

void EdgeBundler::renderBezier() {

}
