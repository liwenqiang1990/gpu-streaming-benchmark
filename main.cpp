#include <GL/glew.h>
#include <GL/glut.h>
#include "RenderUtility/GL.h"
#include "RenderUtility/GLTexture.h"
#include "RenderUtility/SlotTracker3D.h"
//#include "RenderUtility/MemoryAlloc.h"
#include "PortableTimer.h"

#include <stdio.h>
#include <stdlib.h>

#include <vector>
using namespace std;

#pragma comment(lib, "glut32.lib")


GLTexture *texBlock;
GLTexture *texBlockPool;

//key variables
int numBlock = 200;
int numPass = 200;

int window_id = 0;

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(800, 800);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
	window_id = glutCreateWindow("benchmark");

  glewInit();
  glInitStatus status;
  GL::InitGLStatus(status); //enable unaligned texture 

  float timeElapse = 0.0f;

  vector<unsigned char*> blockList;
  vector<unsigned char*>::iterator it;
  //create random dataset/////////////////////
  printf("\ngenerate test blocks\n");

  
  int blockDim = 64;

  for(int b=0; b<numBlock; b++)
  {
    srand(b);
    unsigned char* buffer = (unsigned char*)malloc(blockDim*blockDim*blockDim);
    for(int k=0; k<blockDim; k++)
      for(int j=0; j<blockDim; j++)
        for(int i=0; i<blockDim; i++)
           buffer[k*blockDim*blockDim + j*blockDim + i]=(unsigned char)rand()%255;
    blockList.push_back(buffer);
    printf(".");
  }
  printf("\ndata generation finished!\n");

  int poolD = 5;
  texBlock = new GLTexture(blockDim*poolD, blockDim*poolD, blockDim*poolD, GL_LUMINANCE, GL_INTENSITY);
  texBlock->LoadToGPU();

  texBlock->preAllocateGLPBO(blockDim*blockDim*blockDim);

  /////////////////////////////////upload loop///////////////////////////////
  srand(1);

  SlotTracker3D tracker(5,5,5);

  int offsetX=0, offsetY=0, offsetZ=0;
  
  PortableTimer t; t.Init();
  t.StartTimer();  
   
  for (int p=0; p<numPass; p++)
  {
    //GL::CheckErrors();
    for(it = blockList.begin(); it != blockList.end(); it++)
    {
      texBlock->SubloadToGPU(offsetX,offsetY,offsetZ,blockDim,blockDim,blockDim, (void*)*it, GL_UNSIGNED_BYTE);
      //texBlock->subloadToGPUWithGLBuffer(offsetX,offsetY,offsetZ,blockDim,blockDim,blockDim, (void*)*it);
      //linear order
      /*
      tracker.GetNextEmptySlot(offsetZ, offsetY, offsetX);
      if(offsetZ==-1)
      {
        tracker.FreeAll();
        tracker.GetNextEmptySlot(offsetZ, offsetY, offsetX);
      }*/
      //random order
      //offsetX = rand()%5*blockDim; offsetY = rand()%5*blockDim; offsetZ = rand()%5*blockDim;


    }
  }
  t.EndTimer();
  /////////////////////////////////////////////////////////////////////////////
  timeElapse = t.GetTimeSecond()/(float)(numPass*blockList.size());
  printf("\n the average time: %f\n", timeElapse);
  printf(" the loading speed: %fMB/s\n", (blockDim*blockDim*blockDim/(1024.0f*1024.0f))/timeElapse);

  glFlush();
  delete texBlock;

}