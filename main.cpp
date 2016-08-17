#include "main.h"

#define WIDTH 1000
#define HEIGHT 700

using namespace std;

char *edgeFilename;
Point offset(0, 0);
double width;
double height;

void drawLine(const Point *p1, const Point *p2, const int weight) {
    int w= (int)sqrt(weight) / 10;
    if (w < 1) w = 1;
    float alpha = weight / 1000.0f;
    if (alpha > 1.0) alpha = 1.0f;
    glLineWidth(w);
    glColor4f(0.5f, 0.5f, 1.0f, alpha);
    glBegin(GL_LINES);
    glVertex2d((p1->x - offset.x) / width, (p1->y - offset.y) / height);
    glVertex2d((p2->x - offset.x) / width, (p2->y - offset.y) / height);
    glEnd();
}

void testRender() {
    glClear(GL_COLOR_BUFFER_BIT);
    Point s = Point(0, 0);
    Point t = Point(0, 100);
    drawLine(&s, &t, 5);
    glFlush();
}

EdgeBundleTree::Edge* readEdgesFromFile(unsigned int *numRawEdges) {
    double right = 0, top = 0, left = 0, bottom = 0;
    FILE *fp;
    fp = fopen(edgeFilename, "r");
    assert(fp != NULL);
    double points[4];
    fscanf(fp, "%i", numRawEdges);
    EdgeBundleTree::Edge *edges = (EdgeBundleTree::Edge*) malloc(*numRawEdges * sizeof(EdgeBundleTree::Edge));
    for (int i = 0; i < *numRawEdges; ++i) {
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

        Point *s = new Point(points[0], points[1]);
        Point *t = new Point(points[2], points[3]);
        // TODO: make this more memory efficient (by passing in G = (V, E))
        edges[i] = EdgeBundleTree::Edge(s, t, &edges[i]);
    }
    fclose(fp);
    width = right - left;
    height = top - bottom;
    offset = {width / 2 + left, height / 2 + bottom};
    width *= 0.7;
    height *= 0.7;
    return edges;
}

void testBundlerRender() {
    unsigned int numRawEdges;
    EdgeBundleTree::Edge *edges = readEdgesFromFile(&numRawEdges);
    const int numNeighbors = 5;
    EdgeBundler bundler = EdgeBundler(edges, numRawEdges, numNeighbors);
    bundler.doMingle();
    bundler.setDrawLineFunction(drawLine);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    bundler.render();
    glFlush();
    printf("hi\n");
}

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);

    if (argc != 2) {
        fprintf(stderr, "usage: %s path_edges.tsv\n", *argv);
        return 1;
    }
    edgeFilename = argv[1];

    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("fast-mingle");
    glutDisplayFunc(testBundlerRender);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glutMainLoop();

    return 0;
}
