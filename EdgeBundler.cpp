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
    assert(fp != NULL);
    fscanf(fp, "%i", &numEdges);
    //numEdges /= 10; // HACK FOR TESTING
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
    //numEdges /= 10; // HACK FOR TESTING
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
        pointToEdgesMap[p1].push_back(&edges[i]);
        pointToEdgesMap[p2].push_back(&edges[i]);
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
    delete indices;
    delete dists;
    delete kdTree;
}

void EdgeBundler::doMingle() {
    int numBundled = 0;
    EdgeBundleTree::BundleReturn bundleReturnArray[numNeighbors];
    int level = 1;
    do {
        numBundled = 0;
        printf("Level: %d\n", level++);
        for (unsigned long i = 0; i < tree->numEdges; ++i) {
            if (i % 5000 == 0) {
                printf("\tOn iter %lu of %i\n", i, numEdges);
            }
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
                    numBundled++;
                }
            }
        }
        printf("\tBundled %d of %d\n", numBundled, tree->numEdges);
        tree->coalesceTree();
    } while (numBundled > 0);
}



void EdgeBundler::makeTopEdgesMap(std::unordered_map<int, EdgeBundleTree::Edge*> *map, bool isRendering) {
    if (pointsToRender == nullptr || !isRendering) {
        for (int i = 0; i < numEdges; ++i) {
            (*map)[edges[i].bundle->id] = edges[i].bundle;
        }
    } else {
        for (int i = 0; i < numPointsToRender; ++i) {
            drawPoint(&pointsToRender[i]);
            for (EdgeBundleTree::Edge *edge_ptr : pointToEdgesMap[pointsToRender[i]]) {
                (*map)[edge_ptr->bundle->id] = edge_ptr->bundle;
            }
        }
    }
}


void EdgeBundler::setDrawPointFunction(void (*drawPointFunction)(const Point *)) {
    drawPoint = drawPointFunction;
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
    makeTopEdgesMap(&topEdgeMap, true);
    for (auto pair : topEdgeMap) {
        EdgeBundleTree::Edge *bundle = pair.second;
        drawEdgeLines(bundle);
        drawLine(bundle->sPoint, bundle->tPoint, bundle->weight);
    }
}



void EdgeBundler::write(char *path) {
    FILE *out = fopen(path, "w");
    if (out == nullptr) {
        fprintf(stderr, "Open of %s failed\n", path);
        exit(1);
    }

    std::unordered_map<int, EdgeBundleTree::Edge*> topEdgeMap;
    makeTopEdgesMap(&topEdgeMap, false);
    for (auto pair : topEdgeMap) {
        writeBundle(out, pair.second);
    }
    fclose(out);
}

static void inline writeEnvelope(FILE *out, std::unordered_set<Point *> &points) {
  double xMin = +INFINITY, xMax = -INFINITY,
         yMin = +INFINITY, yMax = -INFINITY;

   for (auto p : points) {
       if (p->x < xMin) { xMin = p->x; }
       if (p->y > yMax) { yMax = p->y; }
       if (p->y < yMin) { yMin = p->y; }
       if (p->x > xMax) { xMax = p->x; }
   }

    double d = 0.0001;
    xMin -= d;
    yMin -= d;
    xMax += d;
    yMax += d;
    fprintf(out, "((%f %f, %f %f, %f %f, %f %f, %f %f))",
            xMin, yMin, xMin, yMax, xMax, yMax, xMax, yMin, xMin, yMin);
}

void EdgeBundler::writeBundle(FILE *out, EdgeBundleTree::Edge *bundle) {

    // Write out the beziers and flatten the tree into individual segments via a traversal
    const Point sPointCurve = lerp(*bundle->sPoint, *bundle->tPoint, curviness);
    const Point tPointCurve = lerp(*bundle->sPoint, *bundle->tPoint, 1 - curviness);
    std::vector<const EdgeBundleTree::Edge *> segments;
    fprintf(out, "MULTILINESTRING(");
    writeEdgeBeziers(out, &segments, bundle, &sPointCurve, &tPointCurve);
    fprintf(out, "(%f %f, %f %f))", sPointCurve.x, sPointCurve.y, tPointCurve.x, tPointCurve.y);
    segments.push_back(bundle);

    // Find all points and all weights
    std::unordered_set<Point *> S;
    std::unordered_set<Point *> T;
    for (auto edge : segments) {
        if (!edge->children) {
            S.insert(edge->sPoint);
            T.insert(edge->tPoint);
        }
    }

    // Write weights
    fprintf(out, "\t");
    for (int i = 0; i < segments.size(); i++) {
        if (i == 0) {
            fprintf(out, "%d", segments[i]->weight);
        } else {
            fprintf(out, " %d", segments[i]->weight);
        }
    }

    // Write size
    fprintf(out, "\t%ld", S.size()+T.size());

    // Write envelopes for endpoints as a multipolygon
    fprintf(out, "\tMULTIPOLYGON(");
    writeEnvelope(out, S);
    fprintf(out, ", ");
    writeEnvelope(out, T);
    fprintf(out, ")\n");
}

void EdgeBundler::writeEdgeBeziers(FILE *out, std::vector<const EdgeBundleTree::Edge *> *segments, const EdgeBundleTree::Edge *edge, const Point *sPointTo, const Point *tPointTo) {
    if (!edge->children) return;
    for (EdgeBundleTree::Edge *child : *edge->children) {
        const Point sChildPointTo = lerp(*child->sPoint, *edge->sPoint, curviness);
        const Point tChildPointTo = lerp(*child->tPoint, *edge->tPoint, curviness);
        const Point *sStart;
        const Point *tStart;
        writeEdgeBeziers(out, segments, child, &sChildPointTo, &tChildPointTo);
        Point sPointFrom, tPointFrom;
        if (!child->children) {
            sPointFrom = *child->sPoint;
            tPointFrom = *child->tPoint;
            sStart = &sPointFrom;
            tStart = &tPointFrom;
        } else {
            sPointFrom = lerp(*child->sPoint, *edge->sPoint, 1 - curviness);
            tPointFrom = lerp(*child->tPoint, *edge->tPoint, 1 - curviness);
            sStart = &sChildPointTo;
            tStart = &tChildPointTo;
        }
        segments->push_back(child);
        writeOneEdgeBezier(out, sStart, &sPointFrom, edge->sPoint, edge->sPoint, sPointTo);
        segments->push_back(child);
        writeOneEdgeBezier(out, tStart, &tPointFrom, edge->tPoint, edge->tPoint, tPointTo);
    }
}

void EdgeBundler::writeOneEdgeBezier(FILE *out, const Point *start, Point *startCurve, const Point *ctrl1, const Point *ctrl2, const Point *end) {
    double p[4][2] = {{startCurve->x, startCurve->y},
                      {ctrl1->x, ctrl1->y},
                      {ctrl2->x, ctrl2->y},
                      {end->x, end->y}};

    // This code is from http://stackoverflow.com/a/11435243/141245

    //the amount of steps in the bezier curve
    unsigned int steps=10;
    double X[steps+2];
    double Y[steps+2];

    X[0] = start->x;
    Y[0] = start->y;

    for(int i = 0 ; i < steps ; i ++) {
        double f = 1.0 * i / steps;     // fraction we have traveled (between [0 and 1])
        double mf = 1.0 - f;            // one minus f (between [1 and 0])
        double x = mf * mf * mf * p[0][0] + 3 * mf * mf * f * p[1][0] + 3 * mf * f * f * p[2][0] + f * f * f * p[3][0];
        double y = mf * mf * mf * p[0][1] + 3 * mf * mf * f * p[1][1] + 3 * mf * f * f * p[2][1] + f * f * f * p[3][1];
        X[i+1] = x;
        Y[i+1] = y;
    }

    X[steps+1] = end->x;
    Y[steps+1] = end->y;

    fprintf(out, "(");
    for (int i = 0; i <= steps; i++) {
        if (i > 0) {
            fprintf(out, ", %f %f", X[i], Y[i]);
        } else {
            fprintf(out, "%f %f", X[i], Y[i]);
        }
    }
    fprintf(out, "), ");
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
    makeTopEdgesMap(&topEdgeMap, true);
    for (auto pair : topEdgeMap) {
        EdgeBundleTree::Edge *bundle = pair.second;
        const Point sPointCurve = lerp(*bundle->sPoint, *bundle->tPoint, curviness);
        const Point tPointCurve = lerp(*bundle->sPoint, *bundle->tPoint, 1 - curviness);
        drawEdgeBeziers(bundle, &sPointCurve, &tPointCurve);
        drawLine(&sPointCurve, &tPointCurve, bundle->weight);
    }
}

void EdgeBundler::setPointsToRender(const Point *points, int numPoints) {
    pointsToRender = points;
    numPointsToRender = numPoints;
}
