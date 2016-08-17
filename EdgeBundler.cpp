#include "EdgeBundler.h"

EdgeBundler::EdgeBundler(const char *edgeFilename, unsigned int numNeighbors, float curviness)
        : numNeighbors(numNeighbors), curviness(curviness / 2.0f) {
    printf("Reading edges from file\n");
    readEdgesFromFile(edgeFilename);
    printf("Doing KNN\n");
    assignNeighbors();
}

void EdgeBundler::readEdgesFromFile(const char *edgeFilename) {
    double right = 0, top = 0, left = 0, bottom = 0;
    FILE *fp;
    fp = fopen(edgeFilename, "r");
    fscanf(fp, "%i", &numEdges);
    numNeighbors = numEdges < numNeighbors ? numEdges : numNeighbors;
    edges = (EdgeBundleTree::Edge*) malloc(sizeof(EdgeBundleTree::Edge) * numEdges);
    std::unordered_map<Point, unsigned int, PointHasher> pointMap;
    Point p1, p2;
    unsigned int idx = 0;
    for (int i = 0; i < numEdges; ++i) {
        fscanf(fp, "%lf", &p1.x);
        fscanf(fp, "%lf", &p1.y);
        fscanf(fp, "%lf", &p2.x);
        fscanf(fp, "%lf", &p2.y);
        if (pointMap.find(p1) == pointMap.end()) pointMap[p1] = idx++;
        if (pointMap.find(p2) == pointMap.end()) pointMap[p2] = idx++;
    }

    fseek(fp, 0, SEEK_SET);
    fscanf(fp, "%i", &numEdges);
    Point *newPoints = (Point*) malloc(sizeof(Point) * idx);
    for (int i = 0; i < numEdges; ++i) {
        fscanf(fp, "%lf", &p1.x);
        fscanf(fp, "%lf", &p1.y);
        fscanf(fp, "%lf", &p2.x);
        fscanf(fp, "%lf", &p2.y);
        if (p1.x > p2.x) {
            Point temp = p1;
            p1 = p2;
            p2 = temp;
        }

        left = p1.x < left ? p1.x : left;
        right = p2.x > right ? p2.x : right;
        bottom = p1.y < bottom ? p1.y : bottom;
        bottom = p2.y < bottom ? p2.y : bottom;
        top = p1.y > top ? p1.y : top;
        top = p2.y > top ? p2.y : top;

        unsigned int idx1 = pointMap[p1];
        unsigned int idx2 = pointMap[p2];
        newPoints[idx1] = p1;
        newPoints[idx2] = p2;
        Point *s = &newPoints[idx1];
        Point *t = &newPoints[idx2];
        edges[i] = EdgeBundleTree::Edge(s, t, &edges[i]);
    }
    fclose(fp);
    width = right - left;
    height = top - bottom;
    center = {width / 2 + left, height / 2 + bottom};
}

void EdgeBundler::assignNeighbors() {
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
    int iter = 1;
    do {
        inkWasSaved = false;
        printf("Iter: %d\t", iter++);
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



void EdgeBundler::makeTopEdgesMap(std::unordered_map<int, EdgeBundleTree::Edge *> *map) {
    for (int i = 0; i < numEdges; ++i) {
        (*map)[edges[i].bundle->id] = edges[i].bundle;
    }
}



void EdgeBundler::setDrawLineFunction(void (*drawLineFunction)(const Point *, const Point *, const int)) {
    drawLine = drawLineFunction;
}

void EdgeBundler::drawEdgeLines(EdgeBundleTree::Edge *edge) {
    if (!edge->children) return;
    for (EdgeBundleTree::Edge *child : *edge->children) {
        drawEdgeLines(child);
        drawLine(edge->sPoint, child->sPoint, child->weight);
        drawLine(edge->tPoint, child->tPoint, child->weight);
    }
}

void EdgeBundler::renderLines() {
    std::unordered_map<int, EdgeBundleTree::Edge*> topEdgeMap;
    makeTopEdgesMap(&topEdgeMap);
    for (auto pair : topEdgeMap) {
        EdgeBundleTree::Edge *bundle = pair.second;
        drawEdgeLines(bundle);
        drawLine(bundle->sPoint, bundle->tPoint, bundle->weight);
    }
}



void EdgeBundler::setDrawBezierFunction(void (*drawBezierFunction)(const Point *start, const Point *ctrl1, const Point *ctrl2, const Point *end, const int weight)) {
    drawBezier = drawBezierFunction;
}

void EdgeBundler::drawEdgeBeziers(const EdgeBundleTree::Edge *edge, const Point *sPointTo, const Point *tPointTo) {
    if (!edge->children) return;
    for (EdgeBundleTree::Edge *child : *edge->children) {
        const Point sChildPointTo = lerp(*child->sPoint, *edge->sPoint, curviness);
        const Point tChildPointTo = lerp(*child->tPoint, *edge->tPoint, curviness);
        drawEdgeBeziers(child, &sChildPointTo, &tChildPointTo);
        Point sPointFrom, tPointFrom;
        if (!child->children) {
            sPointFrom = *child->sPoint;
            tPointFrom = *child->tPoint;
        } else {
            sPointFrom = lerp(*child->sPoint, *edge->sPoint, 1 - curviness);
            tPointFrom = lerp(*child->tPoint, *edge->tPoint, 1 - curviness);
            drawLine(&sChildPointTo, &sPointFrom, child->weight);
            drawLine(&tChildPointTo, &tPointFrom, child->weight);
        }
        drawBezier(&sPointFrom, edge->sPoint, edge->sPoint, sPointTo, child->weight);
        drawBezier(&tPointFrom, edge->tPoint, edge->tPoint, tPointTo, child->weight);
    }
}

void EdgeBundler::renderBezier() {
    std::unordered_map<int, EdgeBundleTree::Edge*> topEdgeMap;
    makeTopEdgesMap(&topEdgeMap);
    for (auto pair : topEdgeMap) {
        EdgeBundleTree::Edge *bundle = pair.second;
        const Point sPointCurve = lerp(*bundle->sPoint, *bundle->tPoint, curviness);
        const Point tPointCurve = lerp(*bundle->sPoint, *bundle->tPoint, 1 - curviness);
        drawEdgeBeziers(bundle, &sPointCurve, &tPointCurve);
        drawLine(&sPointCurve, &tPointCurve, bundle->weight);
    }
}
