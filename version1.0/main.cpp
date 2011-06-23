#include <GL/glew.h>
#include <GL/glut.h>
#include "RenderUtility/GL.h"
#include "RenderUtility/GLTexture.h"
#include "RenderUtility/SlotTracker3D.h"
#include "PortableTimer.h"
#include "TestBufferGenerator.h"

#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <string>
#include <iostream>


using namespace std;

GLTexture *texBlock;

int window_id = 0;

struct paraT
{

int numChannel;
int blockDim;
int poolDim;

int typeByteSize;

int loadMode;
int blockMode;

int cpuBufferSizeMB;
int testSizeGB;

//calculated from other parameters
int numPass; //calculate from blockDim
double sizePerBlockMB;
};

struct timerGroup
{
  PortableTimer memcpyTimer;
  PortableTimer TexSubTimer;
  PortableTimer PBOTimer;
}tGroup;

enum GLTextureType
{
  Uchar,
  UcharRGB, 
  UcharRGBA,
  Float,
  FloatRGB,
  FloatRGBA,
  Int,
  IntRGB,
  IntRGBA
};

///////////////////////////////////////////////////////////////////////////////////////////////////////

void parameterParser(int argc, char* argv[], paraT &para)
{
  //check for empty parameter
  if(argc == 1)
  {
    printf("please add parameters, more info at -help\n");
    exit(0);
  }

  //string argument;
  vector<string> argList;
  vector<string>::iterator it;

	for(int i=1; i<argc; i++)
	{
		argList.push_back(string(argv[i]));
	}
	//cout << argList <<endl;
  //test 10G
  para.cpuBufferSizeMB = 500;

	for(it=argList.begin(); it!=argList.end(); it++)
	{
		if( (*it).at(0) == '-')
		{
			//cout<<*it<<endl;
			if( *it== string("-dim"))
			{
				it++;
				para.blockDim = atoi(it->c_str());
				//cout <<"dim:"<<para.blockDim;
			}
			else if( *it == string("-poolDim"))
			{
				it++;
				para.poolDim = atoi(it->c_str());
				//cout <<"  poolDim:"<<para.poolDim;
			}
			else if( *it == string("-loadMode"))
			{
				it++;
				para.loadMode = atoi(it->c_str());
				//cout <<"  loadMode:"<<para.loadMode;
			}
			else if( *it == string("-blockMode"))
			{
				it++;
				para.blockMode = atoi(it->c_str());
				//cout <<"  blockMode:"<<para.blockMode;
			}
      else if( *it == string("-testSize"))
      {
        it++;
        para.testSizeGB = atoi(it->c_str());
        //cout <<" testSize(GB):"<<testSize;

      }
      else if( *it == string("-cpuBuffer"))
      {
        it++;
        para.cpuBufferSizeMB = atoi(it->c_str());
      }
      else if( *it == string("-numChannel"))
      {
        it++;
        para.numChannel = atoi(it->c_str());
      }
      else if( *it == string("-typeByteSize"))
      {
        it++;
        para.typeByteSize = atoi(it->c_str());
      }
      else if( *it == string("-help") || *it == string("--help"))
      {
        cout<<"-dim  -poolDim  -blockMode  -loadMode  -testSize -cpuBuffer ; the testSize is total data will go through PCIE, default value is 20GB, cpuBuffer is the random texture generated "<<endl;
        exit(0);
      }
			else
      {
				cout<<"error in parameter:"<<*it<<endl;			
        exit(0); 
      }        
		}
		else
    {
			cout<<"error in parameter:"<<endl;
      exit(0);
    }
		
	}

  //calculate other parameter in the para structure
  //calculate the loop number
  //TODO fix the poolDim confussion
  para.poolDim = (int) (para.poolDim/para.blockDim);
  if(para.poolDim == 0)
    para.poolDim = 1;

  para.numPass = (int)(double(para.testSizeGB)/(double(para.blockDim*para.blockDim*para.blockDim)/(1024.0*1024.0*1024.0)));

  para.sizePerBlockMB = (double)(para.blockDim*para.blockDim*para.blockDim*para.numChannel*para.typeByteSize/(1024.0*1024.0));

  //exit(0);

}


inline void testFunc_uchar(paraT &para, SlotTracker3D &tracker, int offsetX, int offsetY, int offsetZ, void* buffer)
{
  int elementByteSize = para.typeByteSize*para.numChannel;

     if(para.loadMode==0)
     {
       tGroup.TexSubTimer.StartTimer();
        texBlock->SubloadToGPU(offsetX,offsetY,offsetZ,para.blockDim,para.blockDim,para.blockDim, buffer, GL_UNSIGNED_BYTE);
        tGroup.TexSubTimer.EndTimer();
     }
      else if(para.loadMode==1)
        texBlock->subloadToGPUWithGLBuffer(offsetX,offsetY,offsetZ,para.blockDim,para.blockDim,para.blockDim, buffer, elementByteSize);
      else if(para.loadMode==2)
        texBlock->SubloadToGPUWithMultiGLBuffer(offsetX,offsetY,offsetZ,para.blockDim,para.blockDim,para.blockDim, buffer, elementByteSize);
      else
        printf("loadMode error!!!\n");
      if(para.blockMode==0)
        ;
      else if(para.blockMode==1)
      {
        tracker.GetNextEmptySlot(offsetZ, offsetY, offsetX);
        if(offsetZ==-1)
        {
          tracker.FreeAll();
          tracker.GetNextEmptySlot(offsetZ, offsetY, offsetX);
        }
      }
      //random order
      else if(para.blockMode==2)
        offsetX = rand()%para.poolDim*para.blockDim; offsetY = rand()%para.poolDim*para.blockDim; offsetZ = rand()%para.poolDim*para.blockDim;
}



void initGLTexture(paraT &para, GLTextureType texType)
{ 
  switch(texType)
  {
  case Uchar:
    texBlock = new GLTexture(para.blockDim*para.poolDim, para.blockDim*para.poolDim, para.blockDim*para.poolDim, GL_LUMINANCE, GL_INTENSITY);
    texBlock->LoadToGPU();

    break;
  case UcharRGB:
    break;
  }

  if(para.loadMode==1)
    texBlock->preAllocateGLPBO(para.blockDim*para.blockDim*para.blockDim*para.numChannel*para.typeByteSize);
  else if(para.loadMode==2)
    texBlock->PreAllocateMultiGLPBO(para.blockDim*para.blockDim*para.blockDim*para.numChannel*para.typeByteSize);
}

int main(int argc, char* argv[])
{
  paraT para;

  //default values
  para.cpuBufferSizeMB = 400;
  para.testSizeGB = 10;
  para.numChannel = 1;
  para.typeByteSize = 1;

  //pass cmd parameters
  parameterParser(argc, argv, para);

  //init GL & glut & glew
  glutInit(&argc, argv);
  glutInitWindowSize(800, 800);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
  window_id = glutCreateWindow("benchmark");
  glewInit();
  glInitStatus status;
  GL::InitGLStatus(status); //enable unaligned texture 

  //allocate testBuffers in CPU memory
  TestBufferGenerator<unsigned char,1> *CBuffer = new TestBufferGenerator<unsigned char,1>(para.cpuBufferSizeMB,para.blockDim,para.blockDim,para.blockDim);

  //create texture in GPU memory
  initGLTexture(para, Uchar);
 
  /////////////////////////////////upload loop///////////////////////////////

  SlotTracker3D tracker(para.poolDim,para.poolDim,para.poolDim);
  int offsetX=0, offsetY=0, offsetZ=0;

  //start timing
  double timeElapse = 0.0;
  PortableTimer t;
  t.Clear();
  t.StartTimer();  
  tGroup.memcpyTimer.Clear();
  tGroup.PBOTimer.Clear();
  tGroup.TexSubTimer.Clear();
   
  for (int p=0; p<para.numPass; p++)
  {
    GL::CheckErrors();
    testFunc_uchar(para, tracker, offsetX, offsetY, offsetZ, (void*)(CBuffer->getNextBlock()));
  }
  t.EndTimer();

  ////////////////////////////////////////////////////////////////////////////////// 
  printf(" %d; %d; %d; %d; %f; ",para.blockDim, para.poolDim, para.loadMode, para.blockMode, double(para.sizePerBlockMB*para.numPass)/t.GetAllTimeSecond());
  printf("PBO-%.2f%%; " , tGroup.PBOTimer.GetAllTimeSecond()/t.GetAllTimeSecond()*100);  
  printf("memcpy-%.2f%%; ", tGroup.memcpyTimer.GetAllTimeSecond()/t.GetAllTimeSecond()*100);
  printf("subTex-%.2f%%; ", tGroup.TexSubTimer.GetAllTimeSecond()/t.GetAllTimeSecond()*100);
  printf("memcpySpeed-%4f\n", double(para.sizePerBlockMB*para.numPass)/tGroup.memcpyTimer.GetAllTimeSecond());
    //printf("blockDim:%d - poolSize:%d - loadMode:%d - blockMode:%d - speed: %fMB/s\n",para.blockDim, para.poolDim, para.loadMode, para.blockMode, (float)(para.blockDim*para.blockDim*para.blockDim/(1024.0f*1024.0f))/timeElapse);

  //clean up
  delete texBlock;
  delete CBuffer;
}
