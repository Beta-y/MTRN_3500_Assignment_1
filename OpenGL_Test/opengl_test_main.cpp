#include <windows.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <GL/glut.h>

void myDisplay(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0f, 0.0f, 1.0f);

    ////设置点的大小
    //glPointSize(20);
    ////先画出3个点，以好观察。
    //glBegin(GL_POINTS);
    //glVertex3f(0, 0, 0);
    //glVertex3f(0, 0.5, 0);
    //glVertex3f(0.5, 0, 0);
    //glEnd();

    //画线
    glBegin(GL_LINES);
    //glTranslated(0.5, 0, 0);
    glLineWidth(2.5);
    glColor3f(1.0, 0.0, 0.0);
    /*
    x: col, y: row, z: in-screen
    min: -1, max : 1;
    */
    glVertex3f(-1, 1, 0);// Point1: LeftTop
    glVertex3f(1, -1, 0);// Point2: RightDown
    glEnd();

    glFlush();
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(400, 400);
    glutInitWindowPosition(200, 200);
    glutCreateWindow("Point");
    glutDisplayFunc(myDisplay);
    glutMainLoop();
    return(0);
}