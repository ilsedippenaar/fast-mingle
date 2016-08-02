#include "main.h"

using namespace std;

void RenderScene(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLineWidth(20.0f);
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(15, 0, 0);
    glEnd();
    glFlush();
}

void SetupRC(void)
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}


int main(int argc, char *argv[]) {
    int	nPts;
    ANNpointArray dataPts;
    ANNpoint queryPt;
    ANNidxArray	nnIdx;
    ANNdistArray dists;
    ANNkd_tree*	kdTree;

    nPts = 100;
    queryPt = annAllocPt(4);
    dataPts = annAllocPts(nPts, 4);
    nnIdx = new ANNidx[10];
    dists = new ANNdist[10];

    kdTree = new ANNkd_tree(dataPts, nPts, 4);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutCreateWindow("Simple");
    glutDisplayFunc(RenderScene);
    SetupRC();
    glutMainLoop();

    return 0;
}
