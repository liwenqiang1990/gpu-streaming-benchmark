#include <GL/glew.h>
#include <GL/glut.h>
#include "RenderUtility/GL.h"
#include "RenderUtility/GLTexture.h"
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
int numBlock = 100;
int numPass = 30;
int blockDim = 128;


float loadGLTexture(void* buffer, int sizeZ, int sizeY, int sizeX, bool isPBO)
{

	texBlock = new GLTexture(sizeX, sizeY, sizeZ, GL_LUMINANCE, GL_INTENSITY);
  
  PortableTimer t; t.Init();
  t.StartTimer();

  if(isPBO==false)
	  texBlock->LoadToGPU(buffer, GL_UNSIGNED_BYTE);
  else
    return texBlock->LoadToGPUWithGLBuffer(buffer, GL_UNSIGNED_BYTE);

  t.EndTimer();
 // printf("time for uploading: %f\n", t.GetTimeSecond());

  GL::CheckErrors();
  return t.GetTimeSecond();
}

void loadGLSubTexture(GLTexture *texBlockPool, void* buffer, int sizeZ, int sizeY, int sizeX, int blockSize)
{
	//texBlockPool = new GLTexture(sizeX, sizeY, sizeZ, GL_LUMINANCE, GL_INTENSITY);
	texBlockPool->LoadToGPU(buffer, GL_UNSIGNED_BYTE);

  //texBlockPool->Bind();
	texBlockPool->LoadToGPU();
	//texBlockPool->LoadToGPUWithGLBuffer();
 
  GL::CheckErrors();
}

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

/////////////////////////////////main loop///////////////////////////////

  for (int p=0; p<numPass; p++)
  {
    //create random dataset
    srand(p);
    unsigned char* buffer = (unsigned char*)malloc(blockDim*blockDim*blockDim);
    for(int k=0; k<blockDim; k++)
      for(int j=0; j<blockDim; j++)
        for(int i=0; i<blockDim; i++)
           buffer[k*blockDim*blockDim + j*blockDim + i]=(unsigned char)rand()%255;


    //main stuff
    
    //float time = loadGLTexture((void*)buffer, blockDim, blockDim, blockDim, false);
    float time = loadGLTexture((void*)buffer, blockDim, blockDim, blockDim, true);

    delete texBlock;
    printf("time for uploading: %f\n", time);
    timeElapse += time;
  }
  timeElapse /= (float)numPass;
  printf("\n the average time: %f\n", timeElapse);
  printf(" the loading speed: %fMB/s\n", (blockDim*blockDim*blockDim/(1024.0f*1024.0f))/timeElapse);

}