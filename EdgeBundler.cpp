#include "EdgeBundler.h"

EdgeBundler::EdgeBundler(const char *edgeFilename, unsigned int numNeighbors, float curviness)
        : numNeighbors(numNeighbors), curviness(curviness / 2.0f) {
    readEdgesFromFile(edgeFilename);
    assignNeighbors();
}

EdgeBundler::EdgeBundler(const char *pointFilename, const char *adjacencyFilename, unsigned int numNeighbors, float curviness)
        : numNeighbors(numNeighbors), curviness(curviness / 2.0f) {
    makeEdgesFromFiles(pointFilename, adjacencyFilename);
    assignNeighbors();
}

void EdgeBundler::readEdgesFromFile(const char *edgeFilename) {
    double right = 0, top = 0, left = 0, bottom = 0;
    FILE *fp;
    fp = fopen(edgeFilename, "r");
    double points[4];
    fscanf(fp, "%i", &numEdges);
    numNeighbors = numEdges < numNeighbors ? numEdges : numNeighbors;
    edges = (EdgeBundleTree::Edge*) malloc(sizeof(EdgeBundleTree::Edge) * numEdges);
    Point *newPoints = (Point*) malloc(sizeof(Point) * numEdges * 2);
    for (int i = 0; i < numEdges; ++i) {
        for (int j = 0; j < 4; ++j) {
            fscanf(fp, "%lf", &points[j]);
        }
        if (points[0] > points[2]) {
            double temp1 = points[0], temp2 = points[1];
            points[0] = points[2];
            points[1] = points[3];
            points[2] = temp1;
            points[3] = temp2;
        }

        left = points[0] < left ? points[0] : left;
        right = points[2] > right ? points[2] : right;
        bottom = points[1] < bottom ? points[1] : bottom;
        bottom = points[3] < bottom ? points[3] : bottom;
        top = points[1] > top ? points[1] : top;
        top = points[3] > top ? points[3] : top;

        newPoints[2*i] = {points[0], points[1]};
        newPoints[2*i + 1] = {points[2], points[3]};
        Point *s = &newPoints[2*i];
        Point *t = &newPoints[2*i + 1];
        edges[i] = EdgeBundleTree::Edge(s, t, &edges[i]);
    }
    fclose(fp);
    width = right - left;
    height = top - bottom;
    center = {width / 2 + left, height / 2 + bottom};
}

void EdgeBundler::makeEdgesFromFiles(const char *pointFilename, const char *adjacencyFilename) {
    double x, y, right = 0, top = 0, left = 0, bottom = 0;
    FILE *fp;
    fp = fopen(pointFilename, "r");
    unsigned int numVertices;
    fscanf(fp, "%i", &numVertices);
    Point *points = (Point*) malloc(sizeof(Point) * numVertices);
    for (int i = 0; i < numVertices; ++i) {
        fscanf(fp, "%lf", &x);
        fscanf(fp, "%lf", &y);

        left = x < left ? x : left;
        right = x > right ? x : right;
        bottom = y < bottom ? y : bottom;
        top = y > top ? y : top;

        points[i].x = x, points[i].y = y;
    }
    fclose(fp);
    width = right - left;
    height = top - bottom;
    center = {width / 2 + left, height / 2 + bottom};

    fp = fopen(adjacencyFilename, "r");
    fscanf(fp, "%i", &numEdges);
    fseek(fp, 1, SEEK_CUR);
    edges = (EdgeBundleTree::Edge*) malloc(sizeof(EdgeBundleTree::Edge) * numEdges);
    int c, edgeIdx = 0, idx = 0, adjIdx, debug = 0;
    do {
        c = fgetc(fp);
        if (c == '\n') idx++;
        else if (c != '\t' && c != EOF) {
            fseek(fp, -1, SEEK_CUR);
            fscanf(fp, "%i", &adjIdx);
            debug++;
            if (idx < adjIdx) {
                Point *s, *t;
                if (points[idx].x < points[adjIdx].x) {
                    s = &points[idx];
                    t = &points[adjIdx];
                } else {
                    s = &points[adjIdx];
                    t = &points[idx];
                }
                edges[edgeIdx] = EdgeBundleTree::Edge(s, t, &edges[edgeIdx]);
                edgeIdx++;
            }
        }
    } while (c != EOF);
    fclose(fp);
    printf("%i\t%i\t%i\n", edgeIdx, numEdges, debug);
    assert(edgeIdx == numEdges);
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
    auto topEdgeMap = new std::unordered_map<int, EdgeBundleTree::Edge*>();
    makeTopEdgesMap(topEdgeMap);
    for (auto pair : *topEdgeMap) {
        EdgeBundleTree::Edge *bundle = pair.second;
        drawEdgeLines(bundle);
        drawLine(bundle->sPoint, bundle->tPoint, bundle->weight);
    }
    delete topEdgeMap;
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
    auto topEdgeMap = new std::unordered_map<int, EdgeBundleTree::Edge*>();
    makeTopEdgesMap(topEdgeMap);
    for (auto pair : *topEdgeMap) {
        EdgeBundleTree::Edge *bundle = pair.second;
        const Point sPointCurve = lerp(*bundle->sPoint, *bundle->tPoint, curviness);
        const Point tPointCurve = lerp(*bundle->sPoint, *bundle->tPoint, 1 - curviness);
        drawEdgeBeziers(bundle, &sPointCurve, &tPointCurve);
        drawLine(&sPointCurve, &tPointCurve, bundle->weight);
    }
    delete topEdgeMap;
}
