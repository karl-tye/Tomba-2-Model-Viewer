#ifndef TOMBAOBJEDITOR_SMST_H
#define TOMBAOBJEDITOR_SMST_H

#include <GL/gl.h>

typedef struct {
    GLfloat x, y, z;
} vertex;

typedef struct model {
    short triCount;
    short quadCount;
    vertex* triangles;
    vertex* quads;
} model;

vertex* getQuadVerts(FILE *SMST);

vertex* getTriVerts(FILE *SMST);

model* getModel(FILE *smst);

model* loadSMST(char* file_name, short* size, int** headersCopy);

short* assignDuplicateList(short** duplicateList, int size, int vCount);

short** findDuplicateVertices(model* model, int* listCount);

int convertVertexToHighestDuplicate(int v, short* duplicateList);

void writeVertexChange(vertex* v, int header, short vertexNum, FILE* SMST, short triCount);

#endif //TOMBAOBJEDITOR_SMST_H
