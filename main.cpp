#include <GL/glew.h>
#include "RenderUtility/GLTexture.h"
//#include "RenderUtility/MemoryAlloc.h"
#include "PortableTimer.h"
GLTexture *texBlock;
GLTexture *texBlockPool;

//key variables
int numBlock = 100;
int numPass = ;
int blockDim;


float loadGLSubTexture(void* buffer, int sizeZ, int sizeY, int sizeX, int blockSize)
{

	texBlock = new GLTexture(sizeX, sizeY, sizeZ, GL_LUMINANCE, GL_INTENSITY);
	texBlock->LoadToGPU(buffer, GL_UNSIGNED_BYTE);

  //texBlockPool->Bind();
  PortableTimer t;
  t.Init();
  t.StartTimer();  

	texBlock->LoadToGPU();
	//texBlockPool->LoadToGPUWithGLBuffer();
  t.EndTimer();
  printf("time for uploading: %f\n", t.GetTimeSecond());
  
  GL::CheckErrors();
  return t.GetTimeSecond();
}

void loadGLTexturePBO(void* buffer, int sizeZ, int sizeY, int sizeX, int blockSize)
{
	texBlockPool = new GLTexture(sizeX, sizeY, sizeZ, GL_LUMINANCE, GL_INTENSITY);
	texBlockPool->LoadToGPU(buffer, GL_UNSIGNED_BYTE);

  //texBlockPool->Bind();
  PortableTimer t;
  t.Init();
  t.StartTimer();  

	texBlockPool->LoadToGPU();
	//texBlockPool->LoadToGPUWithGLBuffer();
  t.EndTimer();
  printf("time for uploading: %f\n", t.GetTimeSecond());
  
  GL::CheckErrors();
}


int main(int argc, char* argv[])
{
  glewInit();
}