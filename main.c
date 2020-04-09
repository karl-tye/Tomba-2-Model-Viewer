#include <GL/gl.h>
#include <GL/freeglut.h>
#include <GL/glut.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "smst.h"

#include "lodepng.h"
#include "tombaex.h"

model* modelList;
int* headers;
model currentModel;
short** duplicateLists;
int listCount;
int currentVertexList;
short modelNum;
short modelCount;
float rot;
GLboolean rotating;
FILE* out;
char*** chunkList;
char** SMSTList;
char* currentFileLocation;
short* SMSTCount;
short currentChunk;
int zoom;
int changeY;
GLdouble modelview[16];

void initMenu();

void menu (int menuentry)
{
    switch (menuentry) {
        case 1:
            currentChunk++;
            currentChunk %= 41;
            initMenu();
            break;
    }
    if (menuentry != 1) {
        currentFileLocation = malloc(sizeof(char)*80);
        if (currentChunk != 0 && currentChunk != 2 && currentChunk < 36)
            sprintf(currentFileLocation, "Extracted/retail-us/chunk_%02d/%02d_sdats/%s",
                currentChunk, currentChunk, SMSTList[menuentry-2]);
        else
            sprintf(currentFileLocation, "Extracted/retail-us/chunk_%02d/%02d_trail/%s",
                    currentChunk, currentChunk, SMSTList[menuentry-2]);
        printf("%s\n", currentFileLocation);
        modelList = loadSMST(currentFileLocation, &modelCount, &headers);
        currentModel = modelList[0];
        currentVertexList = 0;
        duplicateLists = findDuplicateVertices(&currentModel, &listCount);
    }
}

void initMenu()
{
    glutDestroyMenu(1);
    glutCreateMenu (menu);
    glutAddMenuEntry ("Change chunk", 1);
    glutAttachMenu (GLUT_RIGHT_BUTTON);
    SMSTList = chunkList[currentChunk];
    for (int i=0; i<SMSTCount[currentChunk]; i++) {
        glutAddMenuEntry(SMSTList[i], i+2);
    }
}

void flipY(int* y)
{
    float fromMiddle = glutGet(GLUT_WINDOW_WIDTH)/2 - *y;
    *y = glutGet(GLUT_WINDOW_WIDTH)/2 + fromMiddle;
}

int* getVertexLocation(vertex* v)
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    GLdouble projection[16];
    glMatrixMode(GL_PROJECTION);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    GLdouble obj_x, obj_y, obj_z;
    obj_x = v->x; obj_y = v->y; obj_z = v->z;
    GLdouble win_x, win_y, win_z;
    gluProject( obj_x, obj_y, obj_z,
                modelview, projection, viewport,
                &win_x, &win_y, &win_z );

    int* position = malloc(sizeof(int)*2);
    position[0] = (int)win_x;
    position[1] = (int)win_y;

    return position;
}

void printVertex(vertex* v)
{
    printf("Vertex: %f  %f  %f\n", v->x, v->y, v->z);
}

void printDupList(int listNumber)
{
    short* dupList = duplicateLists[listNumber];
    int dup = dupList[0];
    int i = 0;
    vertex* v;
    while (dup != -1) {
        if (dup < (currentModel.triCount*3))
            v = &currentModel.triangles[dup];
        else
            v = &currentModel.quads[dup - (currentModel.triCount*3)];
        //printVertex(v);
        printf("%d ", dup);
        i++;
        dup = dupList[i];
    }
    printf("\n");
}

void saveEditedVertex()
{
    short* dupList = duplicateLists[currentVertexList];
    int dup = dupList[0];
    int i = 0;
    vertex* v;

    if (dup < currentModel.triCount*3)
        v = &currentModel.triangles[dup];
    else
        v = &currentModel.quads[dup-(currentModel.triCount*3)];

    while (dup != -1) {
        out = fopen(currentFileLocation, "r+b");
        writeVertexChange(v, headers[modelNum], dup, out, currentModel.triCount);
        i++;
        dup = dupList[i];
    }
}

void editVertex(short dir)
{
    short* dupList = duplicateLists[currentVertexList];
    int dup = dupList[0];
    int i = 0;
    vertex* v;
    while (dup != -1) {
        if (dup < (currentModel.triCount*3))
            v = &currentModel.triangles[dup];
        else
            v = &currentModel.quads[dup-(currentModel.triCount*3)];
        switch (dir) {
            case 1:
                v->y += 5;
                break;
            case -1:
                v->y -= 5;
                break;
            case 2:
                v->x += 5;
                break;
            case -2:
                v->x -= 5;
                break;
            case 3:
                v->z += 5;
                break;
            case -3:
                v->z -= 5;
                break;
        }
        i++;
        dup = dupList[i];
    }
    saveEditedVertex();
}

vertex* getVertex(int dup)
{
    vertex* v;
    if (dup < (currentModel.triCount*3))
        v = &currentModel.triangles[dup];
    else
        v = &currentModel.quads[dup - (currentModel.triCount*3)];
    return v;
}

float calculateDistance(int* v, int x, int y)
{
    return sqrtf(powf(v[0] - x, 2) + powf(v[1] - y, 2));
}

int findVertexNearestToMouse(int x, int y)
{
    float distance = glutGet(GLUT_WINDOW_WIDTH) + glutGet(GLUT_WINDOW_HEIGHT);
    int closestVertex = -1;
    vertex* v;
    int dup;
    int* vPos;
    flipY(&y);
    for (int i=0; i<listCount; i++) {
        dup = duplicateLists[i][0];
        v = getVertex(dup);
        vPos = getVertexLocation(v);
        if (calculateDistance(vPos, x, y) < distance) {
            distance = calculateDistance(vPos, x, y);
            closestVertex = i;
        }
    }
    printf("Closest vertex: %d\n", closestVertex);
    return closestVertex;
}

void mouse(int button, int state, int x, int y)
{
    switch (button)
    {
        case 0:
            if (rotating || state == GLUT_UP)
                break;
            int closest = findVertexNearestToMouse(x, y);
            currentVertexList = closest;
            break;
        case 4:
            if (state == GLUT_UP)
                break;
            zoom += 10;
            break;
        case 3:
            if (state == GLUT_UP)
                break;
            zoom -= 10;
            break;
    }
}

void specialKeys(int key, int x, int y)
{
    switch (key)
    {
        case GLUT_KEY_LEFT:
            rot += 5;
            break;
        case GLUT_KEY_RIGHT:
            rot -= 5;
            break;
        case GLUT_KEY_UP:
            changeY -= 5;
            break;
        case GLUT_KEY_DOWN:
            changeY += 5;
            break;
        case GLUT_KEY_PAGE_DOWN:
            currentChunk += 5;
            currentChunk %= 41;
            initMenu();
            break;
        case GLUT_KEY_PAGE_UP:
            currentChunk -=5;
            if (currentChunk < 0)
                currentChunk = 40 + currentChunk;
            initMenu();
            break;
    }
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27:
            exit(0);
        case 'r':
            rotating = !rotating;
            break;
        case '.':
            modelNum++;
            modelNum %= modelCount;
            currentModel = modelList[modelNum];
            currentVertexList = 0;
            duplicateLists = findDuplicateVertices(&currentModel, &listCount);
            break;
        case ',':
            modelNum--;
            if (modelNum == -1) modelNum = modelCount - 1;
            currentModel = modelList[modelNum];
            currentVertexList = 0;
            duplicateLists = findDuplicateVertices(&currentModel, &listCount);
            break;
        case 'v':
            currentVertexList++;
            currentVertexList %= listCount;
            printDupList(currentVertexList);
            break;
        case '8':
            editVertex(-1);
            break;
        case '2':
            editVertex(1);
            break;
        case '4':
            editVertex(2);
            break;
        case '6':
            editVertex(-2);
            break;
        case '7':
            editVertex(3);
            break;
        case '9':
            editVertex(-3);
            break;
    }
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective (60, (GLfloat)w / (GLfloat)h, 1.0, 10000.0);
}

void anim()
{
    if (rotating) {
        rot += 0.6;
        if (rot > 360) {
            rot -= 360;
        }
    }
    glutPostRedisplay();
}

void drawFaces()
{
    float colourChange = 0;
    glBegin(GL_QUADS);
    for (int i = 0; i < currentModel.quadCount; i++) {
        glColor3f(0, 0.2+colourChange, 0.8+(colourChange/2));
        glVertex3f(currentModel.quads[(i*4)+2].x, currentModel.quads[(i*4)+2].y, currentModel.quads[(i*4)+2].z);
        glVertex3f(currentModel.quads[i*4].x, currentModel.quads[i*4].y, currentModel.quads[i*4].z);
        glVertex3f(currentModel.quads[(i*4)+1].x, currentModel.quads[(i*4)+1].y, currentModel.quads[(i*4)+1].z);
        glVertex3f(currentModel.quads[(i*4)+3].x, currentModel.quads[(i*4)+3].y, currentModel.quads[(i*4)+3].z);
        colourChange += (0.4/(float)currentModel.quadCount);
    }
    glEnd();

    colourChange = 0;
    vertex triangleVertex;
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < currentModel.triCount * 3; i++) {
        if (i % 3 == 0) {
            glColor3f(0.8+(colourChange/2), 0.3+colourChange, 0);
            colourChange += (0.4/(float)(currentModel.triCount));
        }
        triangleVertex = currentModel.triangles[i];
        glVertex3f(triangleVertex.x, triangleVertex.y, triangleVertex.z);
    }
    glEnd();

}

void drawSelectedVertex()
{
    int vrtx = duplicateLists[currentVertexList][0];
    vertex v;
    if (vrtx < currentModel.triCount*3) {
        v = currentModel.triangles[vrtx];
    }
    else {
        v = currentModel.quads[vrtx-(currentModel.triCount*3)];
    }
    glColor3f(0, 1, 0);
    glTranslatef(v.x, v.y, v.z);
    glutSolidSphere(5, 20, 20);
}

void init()
{
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutCreateWindow ("Tomba! 2 SMST Editor");
    glClearColor(0.0, 0.0, 0.0, 1.0);

    rot = 0;
    modelNum = 0;
    currentModel = modelList[0];
    currentVertexList = 0;
    zoom = 0;
    currentChunk = 0;
    changeY = 0;
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(350 + zoom, 0.0, 0.0,
              0.0, 0.0, 0.0,
              0.0, -1.0, 0.0);
    glEnable(GLUT_MULTISAMPLE);
    glPushMatrix();
        glRotatef(rot, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.0, changeY, 0.0);
        glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
        drawFaces();
        drawSelectedVertex();
    glPopMatrix();

    glutSwapBuffers();
}

int main(int argc, char** argv)
{
    glutInit (&argc, argv);

    // Load SMST
    modelList = loadSMST("Extracted/retail-us/chunk_00/00_trail/82F000-Tomba model.SMST", &modelCount, &headers);
    printf("%d Models\n", modelCount);
    init();

    duplicateLists = findDuplicateVertices(&currentModel, &listCount);

    for (int i=0; i<modelCount; i++)
        printf("%d  ", headers[i]);
    printf("\n");

    SMSTCount = malloc(sizeof(int)*41);
    chunkList = getAllSMSTS(SMSTCount);
    initMenu();

    glutDisplayFunc (display);
    glutKeyboardFunc (keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutReshapeFunc (reshape);
    glutIdleFunc (anim);
    glutMainLoop ();

    return 0;
}


