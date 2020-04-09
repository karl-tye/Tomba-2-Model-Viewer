#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "smst.h"

vertex* getQuadVerts(FILE *SMST)
{
    vertex* v = malloc(sizeof(vertex)*4);
    fseek(SMST, 3, SEEK_CUR);
    short k = fgetc(SMST);
    if (k != 60 && k != 62) {
        printf("Error getting quad vertex from SMST!\n");
        printf("Location in SMST: %ld\n", ftell(SMST)-1);
        exit(-1);
    }
    fseek(SMST, 16, SEEK_CUR);

    short x, y, z;
    fread(&z, 2, 1, SMST);
    fread(&y, 2, 1, SMST);
    fread(&x, 2, 1, SMST);
    v[0].x = x; v[0].y = y; v[0].z = z;

    fread(&x, 2, 1, SMST);
    fread(&z, 2, 1, SMST);
    fread(&y, 2, 1, SMST);
    v[1].x = x; v[1].y = y; v[1].z = z;

    fread(&z, 2, 1, SMST);
    fread(&y, 2, 1, SMST);
    fread(&x, 2, 1, SMST);
    v[2].x = x; v[2].y = y; v[2].z = z;

    fread(&x, 2, 1, SMST);
    fread(&z, 2, 1, SMST);
    fread(&y, 2, 1, SMST);
    v[3].x = x; v[3].y = y; v[3].z = z;

    return v;
}

vertex* getTriVerts(FILE *SMST)
{
    vertex* v = malloc(sizeof(vertex)*3);
    fseek(SMST, 3, SEEK_CUR);
    short k = fgetc(SMST);
    if (k != 52 && k != 54) {
        printf("Error getting triangle vertex from SMST!\n");
        printf("Location in SMST: %ld\n", ftell(SMST)-1);
        exit(-1);
    }
    fseek(SMST, 12, SEEK_CUR);

    short x, y, z;
    fread(&z, 2, 1, SMST);
    fread(&y, 2, 1, SMST);
    fread(&x, 2, 1, SMST);
    v[0].x = x; v[0].y = y; v[0].z = z;

    fread(&x, 2, 1, SMST);
    fread(&z, 2, 1, SMST);
    fread(&y, 2, 1, SMST);
    v[1].x = x; v[1].y = y; v[1].z = z;

    fread(&z, 2, 1, SMST);
    fread(&y, 2, 1, SMST);
    fread(&x, 2, 1, SMST);
    v[2].x = x; v[2].y = y; v[2].z = z;

    fseek(SMST, 2, SEEK_CUR);
    return v;
}

model* getModel(FILE *smst) {
    model* model = malloc(sizeof(short)*2 + sizeof(vertex*)*2);
    fread(&model->triCount, 2, 1, smst);
    fread(&model->quadCount, 2, 1, smst);

    fseek(smst, 12, SEEK_CUR);
    vertex* modelTris = malloc(sizeof(vertex)*model->triCount*3);
    for (int i = 0; i< model->triCount; i++) {
        vertex* triangle = getTriVerts(smst);
        memcpy(&modelTris[i*3], triangle, sizeof(vertex)*3);
        free(triangle);
    }
    vertex* modelQuads = malloc(sizeof(vertex)*model->quadCount*4);
    for (int i = 0; i< model->quadCount; i++) {
        vertex* quad = getQuadVerts(smst);
        memcpy(&modelQuads[i*4], quad, sizeof(vertex)*4);
        free(quad);
    }

    model->quads = modelQuads;
    model->triangles = modelTris;
    return model;
}

model* loadSMST(char* file_name, short* size, int** headersCopy)
{
    FILE* smst = fopen(file_name, "rb");

    if (smst == NULL) {
        printf("Error opening %s\n", file_name);
        exit(-1);
    }

    short* candidate = malloc(sizeof(short)*2);
    fread(candidate, 2, 2, smst);

    int* headers = malloc(sizeof(int)*candidate[1]);
    *headersCopy = malloc(sizeof(int)*candidate[1]);
    fread(headers, 4, candidate[1], smst);
    memcpy(size, &candidate[1], sizeof(short));
    model* modelList = malloc(sizeof(model)*candidate[1]);

    for (int i=0; i<candidate[1]; i++) {
        fseek(smst, headers[i], SEEK_SET);
        model* aModel = getModel(smst);
        memcpy(&modelList[i], aModel, sizeof(struct model));
        free(aModel);
    }

    memcpy(*headersCopy, headers, sizeof(int)*candidate[1]);
    free(headers);
    free(candidate);
    fclose(smst);
    return modelList;
}

short** createDuplicateList(short* duplicates, int vCount, int* listCount)
{
    short** dupLists = malloc(sizeof(short*)*(vCount));
    bool* listCreated = malloc(sizeof(bool)*vCount);
    for (int i =0; i<vCount; i++)
        listCreated[i] = false;
    int totalLists = 0;
    int next;
    for (int i=0; i<vCount; i++) {
        if (listCreated[i] == true)
            continue;
        short* list = malloc(sizeof(short)*vCount);
        listCreated[i] = true;
        list[0] = i;
        next = duplicates[i];
        int count = 1;
        while (next != -1) {
            list[count] = next;
            listCreated[next] = true;
            count++;
            next = duplicates[next];
        }
        list[count] = -1;
        short* trimmedList = malloc(sizeof(short)*(count+1));
        memcpy(trimmedList, list, sizeof(short)*(count+1));
        dupLists[totalLists] = trimmedList;
        free(list);
        totalLists++;
    }
    *listCount = totalLists;
    free(listCreated);
    return dupLists;
}

short** findDuplicateVertices(model* model, int* listCount)
{
    int vCount = model->triCount*3 + model->quadCount*4;
    short* duplicates = malloc(sizeof(short) * vCount);
    vertex v;
    for (int i=0; i<vCount; i++) {
        if (i < (model->triCount*3))
            v = model->triangles[i];
        else
            v = model->quads[i-(model->triCount*3)];
        duplicates[i] = -1;
        for (int j=i+1; j<vCount; j++) {
            if (j < (model->triCount*3)) {
                vertex triangle = model->triangles[j];
                if (i != j && triangle.x == v.x && triangle.y == v.y && triangle.z == v.z) {
                    duplicates[i] = j;
                    break;
                }
            } else {
                vertex quad = model->quads[j-(model->triCount*3)];
                if (i != j && quad.x == v.x && quad.y == v.y && quad.z == v.z) {
                    duplicates[i] = j;
                    break;
                }
            }
        }
    }
    short** duplicateList = createDuplicateList(duplicates, vCount, listCount);
    return duplicateList;
}

void moveFilePointerToVertex(FILE* SMST, short vertexNum, short triCount) {
    if (vertexNum < triCount * 3){
        fseek(SMST, (vertexNum/3) * 36, SEEK_CUR);
        fseek(SMST, 16, SEEK_CUR);
        fseek(SMST, (vertexNum%3)*6, SEEK_CUR);
    } else {
        vertexNum -= triCount*3;
        fseek(SMST, triCount*36, SEEK_CUR);
        printf("%d \n", vertexNum);
        fseek(SMST, (vertexNum/4)*44, SEEK_CUR);
        fseek(SMST, 20, SEEK_CUR);
        fseek(SMST, (vertexNum%4)*6, SEEK_CUR);
    }
}

void writeVertexChange(vertex* v, int header, short vertexNum, FILE* SMST, short triCount)
{
    printf("Attempting to write to vertex %d.\n", vertexNum);
    fseek(SMST, header+16, SEEK_SET);
    moveFilePointerToVertex(SMST, vertexNum, triCount);
    short x, y, z;
    x = (short)v->x;
    y = (short)v->y;
    z = (short)v->z;
    if (vertexNum < triCount*3)
        vertexNum %= 3;
    else {
        vertexNum -= triCount*3;
        vertexNum %= 4;
    }
    if (vertexNum%2 == 0) {
        fwrite(&z, 2, 1, SMST);
        fwrite(&y, 2, 1, SMST);
        fwrite(&x, 2, 1, SMST);
    } else {
        fwrite(&x, 2, 1, SMST);
        fwrite(&z, 2, 1, SMST);
        fwrite(&y, 2, 1, SMST);
    }
    fclose(SMST);
}