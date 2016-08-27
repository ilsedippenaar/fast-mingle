#include "main.h"

using namespace std;

EdgeBundler *bundler;

const int WIDTH = 2000;
const int HEIGHT = 1000;
const double ZOOM_CONST = 1.9;


bool shouldRender() {
    return getenv("SKIP_RENDER") == nullptr;
}

void drawLine(const Point *p1, const Point *p2, const int weight) {
    int w = (int)sqrt(weight) / 2;
    if (w < 1) w = 1;
    float alpha = weight / 5.0f;
    if (alpha > 1.0) alpha = 1.0f;
    glLineWidth(w);
    glColor4f(0.5f, 0.5f, 1.0f, alpha);
    glBegin(GL_LINES);
        glVertex2d((p1->x - bundler->center.x) / bundler->width * ZOOM_CONST, (p1->y - bundler->center.y) / bundler->height * ZOOM_CONST);
        glVertex2d((p2->x - bundler->center.x) / bundler->width * ZOOM_CONST, (p2->y - bundler->center.y) / bundler->height * ZOOM_CONST);
    glEnd();
}

void drawBezier(const Point *start, const Point *ctrl1, const Point *ctrl2, const Point *end, const int weight) {
    GLdouble controlPoints[4][3] = {{(start->x - bundler->center.x) / bundler->width * ZOOM_CONST, (start->y - bundler->center.y) / bundler->height * ZOOM_CONST, 0},
                                   {(ctrl1->x - bundler->center.x) / bundler->width * ZOOM_CONST, (ctrl1->y - bundler->center.y) / bundler->height * ZOOM_CONST, 0},
                                   {(ctrl2->x - bundler->center.x) / bundler->width * ZOOM_CONST, (ctrl2->y - bundler->center.y) / bundler->height * ZOOM_CONST, 0},
                                   {(end->x - bundler->center.x) / bundler->width * ZOOM_CONST, (end->y - bundler->center.y) / bundler->height * ZOOM_CONST, 0}};
    float alpha = weight / 5.0f;
    if (alpha > 1.0) alpha = 1.0f;
    glColor4f(0.5f, 0.5f, 1.0f, alpha);
    glMap1d(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, &controlPoints[0][0]);
    glEnable(GL_MAP1_VERTEX_3);
    glLineWidth((weight <= 10.0f) ? 1.0f : (weight * 0.1f));
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 30; i++) {
        glEvalCoord1d((GLdouble) i / 30);
    }
    glEnd();
}

void init(char *pathIn) {
    bundler = new EdgeBundler(pathIn, 20, 0.8f);
    printf("Created Edge Bundler\n");
    bundler->doMingle();
    printf("Finished mingling");
    if (shouldRender()) {
        bundler->setDrawLineFunction(drawLine);
        bundler->setDrawBezierFunction(drawBezier);
    }
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    bundler->renderBezier();
    glFlush();
}

int main(int argc, char *argv[]) {
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "usage: %s path_edges.txt path_output.txt\n", *argv);
        return 1;
    }
    init(argv[1]);

    if (shouldRender()) {
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
        glutInitWindowSize(WIDTH, HEIGHT);
        glutCreateWindow("fast-mingle");
        glutDisplayFunc(renderBundler);
        glutReshapeFunc(reshape);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glutMainLoop();
    }

    if (argc >= 3) {
        bundler->write(argv[2]);
    }

    return 0;
}
