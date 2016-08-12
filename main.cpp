#include "main.h"

#define FILENAME "test_data/world_list.txt"
#define WIDTH 1000
#define HEIGHT 700

using namespace std;

Point offset(0, 0);
double width;
double height;

EdgeBundler *bundler;

void drawLine(const Point *p1, const Point *p2, const int weight) {
//    glLineWidth(weight * 0.1f);
    glColor3f(0.5f, 0.5f, 1.0f);
    glBegin(GL_LINES);
        glVertex2d((p1->x - offset.x) / width, (p1->y - offset.y) / height);
        glVertex2d((p2->x - offset.x) / width, (p2->y - offset.y) / height);
    glEnd();
}

void drawBezier(const EdgeBundler::ControlPoints *controlPoints) {
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(0.5f, 0.5f, 1.0f);
    glMap1d(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, (GLdouble*) controlPoints);
    glEnable(GL_MAP1_VERTEX_3);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 30; i++) {
        glEvalCoord1d((GLdouble) i / 30);
    }
    glEnd();
}

EdgeBundleTree::Edge* readEdgesFromFile(unsigned int *numRawEdges) {
    double right = 0, top = 0, left = 0, bottom = 0;
    FILE *fp;
    fp = fopen(FILENAME, "r");
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

void init() {
    unsigned int numRawEdges;
    EdgeBundleTree::Edge *edges = readEdgesFromFile(&numRawEdges);
    unsigned int maxNeighbors = 20;
    const unsigned int numNeighbors = numRawEdges < maxNeighbors ? numRawEdges : maxNeighbors;
    bundler = new EdgeBundler(edges, numRawEdges, numNeighbors);
    bundler->doMingle();
    bundler->setDrawLineFunction(drawLine);
}

void renderBundler() {
    glClear(GL_COLOR_BUFFER_BIT);
    bundler->renderLines();
    glFlush();
}

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    init();
    glutCreateWindow("fast-mingle");
    glutDisplayFunc(renderBundler);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glutMainLoop();

    return 0;
}
