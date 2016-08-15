#include "main.h"

#define FILENAME "test_data/testcrossed_list.txt"
#define WIDTH 1000
#define HEIGHT 700

using namespace std;

Point offset = {0, 0};
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

void drawBezier(const Point *start, const Point *ctrl1, const Point *ctrl2, const Point *end, const int weight) {
    GLdouble controlPoints[4][3] = {{(start->x - offset.x) / width, (start->y - offset.y) / height, 0},
                                   {(ctrl1->x - offset.x) / width, (ctrl1->y - offset.y) / height, 0},
                                   {(ctrl2->x - offset.x) / width, (ctrl2->y - offset.y) / height, 0},
                                   {(end->x - offset.x) / width, (end->y - offset.y) / height, 0}};
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(0.5f, 0.5f, 1.0f);
    glMap1d(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, &controlPoints[0][0]);
    glEnable(GL_MAP1_VERTEX_3);
    glLineWidth(weight * 0.1f);
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
    EdgeBundleTree::Edge *edges = (EdgeBundleTree::Edge*) malloc(sizeof(EdgeBundleTree::Edge) * *numRawEdges);
    Point *newPoints = (Point*) malloc(sizeof(Point) * *numRawEdges * 2);
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

        newPoints[2*i] = {points[0], points[1]};
        newPoints[2*i + 1] = {points[2], points[3]};
        Point *s = &newPoints[2*i];
        Point *t = &newPoints[2*i + 1];
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
    bundler->setDrawBezierFunction(drawBezier);
}

void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (w <= h)
        glOrtho(-1.0, 1.0, -1.0*(GLfloat)h/(GLfloat)w,
                1.0*(GLfloat)h/(GLfloat)w, -1.0, 1.0);
    else
        glOrtho(-1.0*(GLfloat)w/(GLfloat)h,
                1.0*(GLfloat)w/(GLfloat)h, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void renderBundler() {
    glClear(GL_COLOR_BUFFER_BIT);
    bundler->renderBezier();
    glFlush();
}

int main(int argc, char *argv[]) {
    init();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("fast-mingle");
    glutDisplayFunc(renderBundler);
    glutReshapeFunc(reshape);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glShadeModel(GL_FLAT);
    glutMainLoop();

    return 0;
}
