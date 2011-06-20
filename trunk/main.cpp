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
#include <string>
#include <iostream>
using namespace std;


GLTexture *texBlock;
GLTexture *texBlockPool;

//key variables
//int numBlock = 10;
//int numPass = 100;
//int blockDim = 256;
//int poolD = 2;

//int loadMode = 1;
//int blockMode = 1;

int window_id = 0;

struct paraT
{
int numBlock;
int numPass; //calculate from blockDim
int blockDim;
int poolDim;

int loadMode;
int blockMode;
};

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
  int testSize = 20; //

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
			else if( *it == string("-numBlock"))
			{
				it++;
				para.numBlock = atoi(it->c_str());
				//cout <<"  numBlock:"<< para.numBlock;				
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
        testSize = atoi(it->c_str());
        //cout <<" testSize(GB):"<<testSize;

      }
      else if( *it == string("-help") || *it == string("--help"))
      {
        cout<<"-numBlock  -poolDim  -blockMode  -loadMode  -dim -testSize ; the testSize is total data will go through PCIE, default value is 20GB"<<endl;
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

  //calculate the loop number
  //TODO fix the poolDim confussion
  para.poolDim = (int) (para.poolDim/para.blockDim);
  if(para.poolDim == 0)
    para.poolDim = 1;

  para.numPass = (int)(double(testSize)/(double(para.blockDim*para.blockDim*para.blockDim*para.numBlock)/(1024.0*1024.0*1024.0)));

  //exit(0);

}


paraT para;

int main(int argc, char* argv[])
{
	parameterParser(argc, argv, para);

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
  //printf("\ngenerate test blocks\n");

  

  for(int b=0; b<para.numBlock; b++)
  {
    srand(b);
    unsigned char* buffer = (unsigned char*)malloc(para.blockDim*para.blockDim*para.blockDim);
    for(int k=0; k<para.blockDim; k++)
      for(int j=0; j<para.blockDim; j++)
        for(int i=0; i<para.blockDim; i++)
           buffer[k*para.blockDim*para.blockDim + j*para.blockDim + i]=(unsigned char)rand()%255;
    blockList.push_back(buffer);
    //printf(".");
  }
  //printf("\ndata generation finished!\n");

  texBlock = new GLTexture(para.blockDim*para.poolDim, para.blockDim*para.poolDim, para.blockDim*para.poolDim, GL_LUMINANCE, GL_INTENSITY);
  texBlock->LoadToGPU();

  if(para.loadMode==1)
    texBlock->preAllocateGLPBO(para.blockDim*para.blockDim*para.blockDim);
  else if(para.loadMode==2)
    texBlock->PreAllocateMultiGLPBO(para.blockDim*para.blockDim*para.blockDim);

  /////////////////////////////////upload loop///////////////////////////////
  srand(1);

  SlotTracker3D tracker(para.poolDim,para.poolDim,para.poolDim);

  int offsetX=0, offsetY=0, offsetZ=0;
  
  PortableTimer t;
  t.StartTimer();  
   
  for (int p=0; p<para.numPass; p++)
  {
    GL::CheckErrors();
    printf("what's wrong!\n");
    for(it = blockList.begin(); it != blockList.end(); it++)
    {
      if(para.loadMode==0)
        texBlock->SubloadToGPU(offsetX,offsetY,offsetZ,para.blockDim,para.blockDim,para.blockDim, (void*)*it, GL_UNSIGNED_BYTE);
      else if(para.loadMode==1)
        texBlock->subloadToGPUWithGLBuffer(offsetX,offsetY,offsetZ,para.blockDim,para.blockDim,para.blockDim, (void*)*it);
      else if(para.loadMode==2)
        texBlock->SubloadToGPUWithMultiGLBuffer(offsetX,offsetY,offsetZ,para.blockDim,para.blockDim,para.blockDim, (void*)*it);
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
  }
  t.EndTimer();
  /////////////////////////////////////////////////////////////////////////////
  timeElapse = t.GetTimeSecond()/(float)(para.numPass*blockList.size());
  //printf("\n the average time: %f\n", timeElapse);
  //printf("blockDim:%d - poolSize:%d - loadMode:%d - blockMode:%d - speed: %fMB/s\n",para.blockDim, para.poolDim, para.loadMode, para.blockMode, (float)(para.blockDim*para.blockDim*para.blockDim/(1024.0f*1024.0f))/timeElapse);
  printf(" %d ;  %d ;  %d ;  %d ;  %f \n",para.blockDim, para.poolDim, para.loadMode, para.blockMode, (float)(para.blockDim*para.blockDim*para.blockDim/(1024.0f*1024.0f))/timeElapse);
  //glFlush();
  delete texBlock;
  for(it = blockList.begin(); it != blockList.end(); it++)
    delete (*it);


}
