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

GLenum bufferHint;//stream draw dynamic draw

struct paraT
{

int blockDim;
int poolDim;

int textureType;
int loadMode;
int blockMode;

int cpuBufferSizeMB;
int testSizeGB;


//calculated from other parameters
int numPass; //calculate from blockDim
double sizePerBlockMB;
int typeByteSize;
int numChannel;
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

//adaptor of TestBufferGenerator
class CPUBuffer
{
public:
  CPUBuffer(paraT &para)
  {
    textureType = para.textureType;
    CBufferUchar = NULL;
    CBufferUcharRGB = NULL;
    CBufferUcharRGBA = NULL;

    CBufferFloat = NULL;
    CBufferFloatRGB = NULL;
    CBufferFloatRGBA = NULL;

    CBufferInt = NULL;
    CBufferIntRGB = NULL;
    CBufferIntRGBA = NULL;

    switch(para.textureType)
   {
   case Uchar:
      CBufferUchar = new TestBufferGenerator<unsigned char,1>(para.cpuBufferSizeMB,para.blockDim,para.blockDim,para.blockDim); break;
   case UcharRGB:
      CBufferUcharRGB = new TestBufferGenerator<unsigned char,3>(para.cpuBufferSizeMB,para.blockDim,para.blockDim,para.blockDim); break;
   case UcharRGBA:
     CBufferUcharRGBA  = new TestBufferGenerator<unsigned char,4>(para.cpuBufferSizeMB,para.blockDim,para.blockDim,para.blockDim); break;

   case Float:
      CBufferFloat = new TestBufferGenerator<float,1>(para.cpuBufferSizeMB,para.blockDim,para.blockDim,para.blockDim); break;
   case FloatRGB:
      CBufferFloatRGB = new TestBufferGenerator<float,3>(para.cpuBufferSizeMB,para.blockDim,para.blockDim,para.blockDim); break;
   case FloatRGBA:
      CBufferFloatRGBA = new TestBufferGenerator<float,4>(para.cpuBufferSizeMB,para.blockDim,para.blockDim,para.blockDim); break;   
   
   case Int:
      CBufferInt = new TestBufferGenerator<int,1>(para.cpuBufferSizeMB,para.blockDim,para.blockDim,para.blockDim); break;
   case IntRGB:
      CBufferIntRGB = new TestBufferGenerator<int,3>(para.cpuBufferSizeMB,para.blockDim,para.blockDim,para.blockDim); break;
   case IntRGBA:
      CBufferIntRGBA = new TestBufferGenerator<int,4>(para.cpuBufferSizeMB,para.blockDim,para.blockDim,para.blockDim); break;
   }
  }
  ~CPUBuffer()
  {
    if(CBufferUchar)
      delete CBufferUchar;
    if(CBufferUcharRGB)
      delete CBufferUcharRGB;
    if(CBufferUcharRGBA)
      delete CBufferUcharRGBA;

    if(CBufferFloat)
      delete CBufferFloat;
    if(CBufferFloatRGB)
      delete CBufferFloatRGB;
    if(CBufferFloatRGBA)
      delete CBufferFloatRGBA;

    if(CBufferInt)
      delete CBufferInt;
    if(CBufferIntRGB)
      delete CBufferIntRGB;
    if(CBufferIntRGBA)
      delete CBufferIntRGBA;

  }

  void* GetNextBuffer()
  {
    if(Uchar == textureType)
      return (void*)CBufferUchar->getNextBlock();
    else if(UcharRGB == textureType)
      return (void*)CBufferUcharRGB->getNextBlock();
    else if(UcharRGBA == textureType)
      return (void*)CBufferUcharRGBA->getNextBlock();
    else if(Float == textureType)
      return (void*)CBufferFloat->getNextBlock();
    else if(FloatRGB == textureType)
      return (void*)CBufferFloatRGB->getNextBlock();
    else if(FloatRGBA == textureType)
      return (void*)CBufferFloatRGBA->getNextBlock();
    else if(Int == textureType)
      return (void*)CBufferInt->getNextBlock();
    else if(IntRGB == textureType)
      return (void*)CBufferIntRGB->getNextBlock();
    else if(IntRGBA == textureType)
      return (void*)CBufferIntRGBA->getNextBlock();
    else
      return NULL;
  }
private:
  int textureType;
  TestBufferGenerator<unsigned char,1> *CBufferUchar;
  TestBufferGenerator<unsigned char,3> *CBufferUcharRGB;
  TestBufferGenerator<unsigned char,4> *CBufferUcharRGBA;

  TestBufferGenerator<float,1> *CBufferFloat;
  TestBufferGenerator<float,3> *CBufferFloatRGB;
  TestBufferGenerator<float,4> *CBufferFloatRGBA;

  TestBufferGenerator<int,1> *CBufferInt;
  TestBufferGenerator<int,3> *CBufferIntRGB;
  TestBufferGenerator<int,4> *CBufferIntRGBA;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////

void parameterParser(int argc, char* argv[], paraT &para)
{
  //check for empty parameter
  if(argc == 1)
  {
    printf("please add parameters, more info at -help\n");
    //exit(0);
    return;
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
      else if( *it == string("-textureType"))
      {
        it++;
        if( *it == string("Uchar") )
        {
          para.textureType = Uchar;
        }
        else if( *it == string("UcharRGB"))
        {
          para.textureType = UcharRGB;
        }
        else if( *it == string("UcharRGBA"))
        {
          para.textureType = UcharRGBA;
        }
        else if ( *it == string("Float") )
        {
          para.textureType = Float;
        }
        else if ( *it == string("FloatRGB") )
        {
          para.textureType = FloatRGB;
        }
        else if ( *it == string("FloatRGBA") )
        {
          para.textureType = FloatRGBA;
        }
         else if ( *it == string("Int") )
        {
          para.textureType = Int;
        }
        else if ( *it == string("IntRGB") )
        {
          para.textureType = IntRGB;
        }
        else if ( *it == string("IntRGBA") )
        {
          para.textureType = IntRGBA;
        }
      }
      else if( *it == string("-bufferHint"))
      {
        it++;
        if( *it == string("GL_STREAM_DRAW") )
          bufferHint = GL_STREAM_DRAW;
        else if(*it == string("GL_DYNAMIC_DRAW") )
          bufferHint = GL_DYNAMIC_DRAW;
        else 
          printf("Buffer hint error\n");
        break;
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

  //exit(0);

}


inline void testFunc(paraT &para, SlotTracker3D &tracker, int offsetX, int offsetY, int offsetZ, void* buffer)
{
  int elementByteSize = para.typeByteSize*para.numChannel;

     if(para.loadMode==0)
     {
       tGroup.TexSubTimer.StartTimer();
       if(para.textureType == Uchar || para.textureType == UcharRGB || para.textureType == UcharRGBA)
         texBlock->SubloadToGPU(offsetX,offsetY,offsetZ,para.blockDim,para.blockDim,para.blockDim, buffer, GL_UNSIGNED_BYTE);
       else if(para.textureType == Float || para.textureType == FloatRGB || para.textureType == FloatRGBA)
         texBlock->SubloadToGPU(offsetX,offsetY,offsetZ,para.blockDim,para.blockDim,para.blockDim, buffer, GL_FLOAT);
       else if(para.textureType == Int || para.textureType == IntRGB || para.textureType == IntRGBA)
         texBlock->SubloadToGPU(offsetX,offsetY,offsetZ,para.blockDim,para.blockDim,para.blockDim, buffer, GL_INT);

        tGroup.TexSubTimer.EndTimer();
     }
      else if(para.loadMode==1) //same for all case
        texBlock->subloadToGPUWithGLBuffer(offsetX,offsetY,offsetZ,para.blockDim,para.blockDim,para.blockDim, buffer, elementByteSize);
      else if(para.loadMode==2) //same for all case
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



void initGLTexture(paraT &para, int texType)
{ 
  switch(texType)
  {
  case Uchar:
    texBlock = new GLTexture(para.blockDim*para.poolDim, para.blockDim*para.poolDim, para.blockDim*para.poolDim, GL_LUMINANCE, GL_INTENSITY8);
    //glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
    texBlock->LoadToGPU();

    break;
  case UcharRGB:
    texBlock = new GLTexture(para.blockDim*para.poolDim, para.blockDim*para.poolDim, para.blockDim*para.poolDim, GL_RGB, GL_RGB8);
    //glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
    texBlock->LoadToGPU();
    break;
  case UcharRGBA:
    texBlock = new GLTexture(para.blockDim*para.poolDim, para.blockDim*para.poolDim, para.blockDim*para.poolDim, GL_RGBA, GL_RGBA8);
    //glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
    texBlock->LoadToGPU();  
  case Float: 
    texBlock = new GLTexture(para.blockDim*para.poolDim, para.blockDim*para.poolDim, para.blockDim*para.poolDim, GL_RED, GL_INTENSITY32F_ARB, GL_FLOAT);
    //glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
    texBlock->LoadToGPU();  
    break;
  case FloatRGB: 
    texBlock = new GLTexture(para.blockDim*para.poolDim, para.blockDim*para.poolDim, para.blockDim*para.poolDim, GL_RGB, GL_RGB32F_ARB, GL_FLOAT);
    //glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
    texBlock->LoadToGPU();  
      break;
  case FloatRGBA:
    texBlock = new GLTexture(para.blockDim*para.poolDim, para.blockDim*para.poolDim, para.blockDim*para.poolDim, GL_RGBA, GL_RGBA32F_ARB, GL_FLOAT);
    //glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
    texBlock->LoadToGPU();  
      break;
  case Int:
    texBlock = new GLTexture(para.blockDim*para.poolDim, para.blockDim*para.poolDim, para.blockDim*para.poolDim, GL_LUMINANCE_INTEGER_EXT, GL_INTENSITY32I_EXT, GL_INT);
    //glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
    texBlock->LoadToGPU();  
      break;
  case IntRGB:
    texBlock = new GLTexture(para.blockDim*para.poolDim, para.blockDim*para.poolDim, para.blockDim*para.poolDim, GL_RGB_INTEGER_EXT, GL_RGB32I_EXT, GL_INT);
    //glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
    texBlock->LoadToGPU();  
      break;
  case IntRGBA:
    texBlock = new GLTexture(para.blockDim*para.poolDim, para.blockDim*para.poolDim, para.blockDim*para.poolDim, GL_RGBA_INTEGER_EXT, GL_RGBA32I_EXT, GL_INT);
    //glBindBuffer(GL_PIXEL_UNPACK_BUFFER_EXT, 0);
    texBlock->LoadToGPU();  
      break;
  }

  if(para.loadMode==1)
    texBlock->preAllocateGLPBO(para.blockDim*para.blockDim*para.blockDim*para.numChannel*para.typeByteSize);
  else if(para.loadMode==2)
    texBlock->PreAllocateMultiGLPBO(para.blockDim*para.blockDim*para.blockDim*para.numChannel*para.typeByteSize);

}

void processPara(paraT &para)
{
  //generate numChannel and typeByteSize from textureType
  if(para.textureType == Uchar)
  {
    para.numChannel = 1;
    para.typeByteSize = 1;
  }
  else if(para.textureType == UcharRGB)
  {
    para.numChannel = 3;
    para.typeByteSize = 1;
  }
  else if(para.textureType == UcharRGBA)
  {
    para.numChannel = 4;
    para.typeByteSize = 1;
  }    
  else if(para.textureType == Float)
  {
    para.numChannel = 1;
    para.typeByteSize = 4;
  }      
  else if(para.textureType == FloatRGB)
  {
    para.numChannel = 3;
    para.typeByteSize = 4;
  }      
  else if(para.textureType == FloatRGBA)
  {
    para.numChannel = 4;
    para.typeByteSize = 4;
  }      
  else if(para.textureType == Int)
  {
    para.numChannel = 1;
    para.typeByteSize = 4;
  }      
  else if(para.textureType == IntRGB)
  {
    para.numChannel = 3;
    para.typeByteSize = 4;
  }      
  else if(para.textureType == IntRGBA)
  {
    para.numChannel = 4;
    para.typeByteSize = 4;
  }      
  //calculate other parameter in the para structure
  //calculate the loop number
  //TODO fix the poolDim confussion
  para.poolDim = (int) (para.poolDim/para.blockDim);
  if(para.poolDim == 0)
    para.poolDim = 1;

  para.numPass = (int)(double(para.testSizeGB)/(double(para.blockDim*para.blockDim*para.blockDim)/(1024.0*1024.0*1024.0)));

  para.sizePerBlockMB = (double)(para.blockDim*para.blockDim*para.blockDim*para.numChannel*para.typeByteSize)/(1024.0*1024.0);


}

int main(int argc, char* argv[])
{
  paraT para;

  //default values
  para.cpuBufferSizeMB = 200;
  para.testSizeGB = 1;
  para.blockDim = 65;
  para.poolDim = 256;
  para.textureType = Int;
  para.loadMode = 1;
  para.blockMode = 1;
  bufferHint = GL_DYNAMIC_DRAW;

  //pass cmd parameters
  parameterParser(argc, argv, para);
  processPara(para);

  //init GL & glut & glew
  glutInit(&argc, argv);
  glutInitWindowSize(800, 800);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
  window_id = glutCreateWindow("benchmark");
  glewInit();
  glInitStatus status;
  GL::InitGLStatus(status); //enable unaligned texture 

  //create Texture
  initGLTexture(para, para.textureType);

  //create CPU buffer
  CPUBuffer CBuffer(para);

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
    testFunc(para, tracker, offsetX, offsetY, offsetZ, CBuffer.GetNextBuffer());
  }
  t.EndTimer();

  ////////////////////////////////////////////////////////////////////////////////// 
  //pbo memcpy subTex
  printf(" %d; %d; %d; %d; %.4f; ",para.blockDim, para.poolDim, para.loadMode, para.blockMode, double(para.sizePerBlockMB*para.numPass)/t.GetAllTimeSecond());
  printf("%.2f%%; " , tGroup.PBOTimer.GetAllTimeSecond()/t.GetAllTimeSecond()*100);  
  printf("%.2f%%; ", tGroup.memcpyTimer.GetAllTimeSecond()/t.GetAllTimeSecond()*100);
  printf("%.2f%%; ", tGroup.TexSubTimer.GetAllTimeSecond()/t.GetAllTimeSecond()*100);
  printf("%.4f; ", double(para.sizePerBlockMB*para.numPass)/tGroup.memcpyTimer.GetAllTimeSecond());
  if(bufferHint == GL_DYNAMIC_DRAW)
    printf("Dynamic Draw; ");
  else if(bufferHint == GL_STREAM_DRAW)
    printf("Stream Draw; ");

  switch(para.textureType)
  {
  case Uchar:
    printf("Uchar");  break;
  case UcharRGB:
    printf("UcharRGB");  break;
  case UcharRGBA:
    printf("UcharRGBA");  break;
  case Float:
    printf("Float");  break;
  case FloatRGB:
    printf("FloatRGB");  break;
  case FloatRGBA:
    printf("FloatRGBA");  break;
  case Int:
    printf("Int");  break;
  case IntRGB:
    printf("IntRGB");  break;
  case IntRGBA:
    printf("IntRGBA");  break;
  }

  //clean up
  delete texBlock;

}
