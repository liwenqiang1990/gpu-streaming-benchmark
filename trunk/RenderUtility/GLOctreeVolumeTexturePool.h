#ifndef GL_OCTREE_VOLUME_H
#define GL_OCTREE_VOLUME_H

#include <cstdio>

#include <vector>
#include <map>

#include "RenderUtility/OGLUtility/GLTexture.h" 
#include "sparseVoxelOctree.h"
#include "SlotTracker3D.h"


template<typename T>
class sparseVoxelOctree;

#define GPU_NODE_POOL_DEFAULT_SIZE  1
#define GPU_BLOCK_POOL_DEFAULT_SIZE 700
#define GPU_BLOCK_DEFAULT_SIZE 18	 //blockDim = blockSize+blockBoundarySize

struct nodeTexel
{
  unsigned int first;
  unsigned int second;
};

template<typename T>
class GLOctreeVolumeTexturePool
{
public:
  GLOctreeVolumeTexturePool(int GPUPoolCapMB=GPU_BLOCK_POOL_DEFAULT_SIZE , int blockDim=GPU_BLOCK_DEFAULT_SIZE);
  ~GLOctreeVolumeTexturePool();

  bool AddSVO(sparseVoxelOctree<T>* svo, unsigned int indentifier); //if don't have space return NULL
  sparseVoxelOctree<T>* GetSVOByIndentifier(unsigned int indentifier);
  void UpLoadSVOGPU(unsigned int indentifier);
  void ClearSVOGPU(unsigned int indentifier);
  //void ClearAll();
  void ClearAllGPUPool() { _nodeTracker.FreeAll(); _blockTracker.FreeAll(); _isSVOLoaded = false; }
  void ClearAllCPU();
  bool IsCurrentSVOLoadedGPU() {return _isSVOLoaded;}

  GLenum getBlockPoolTexID(){return _blockPool->GetTextureID();}
  GLenum getBlockPoolTexType(){return _blockPool->GetTextureType();}
  int getNodePoolDim(){return _nodeClusterDim*2;}
  int getBlockPoolDim(){return _blockPoolDim;}
  GLenum getNodePoolTexID() {return _nodePool->GetTextureID();}
  GLenum getNodePoolTexType(){return _nodePool->GetTextureType();}


  void getCurrentRootNodePos(unsigned int &z, unsigned int &y, unsigned int &x)
  {_currentSVO->getGPURootPos(z, y, x);}
  sparseVoxelOctree<T>* getCurrentSVO() {return _currentSVO;}
  void UpdateCurrentSVO(unsigned int indentifier) 
  { 
    if(_currentSVO == GetSVOByIndentifier(indentifier)) 
      return;
    else
    {
      _currentSVO = GetSVOByIndentifier(indentifier);
      _isSVOLoaded = false;
    }
  }


  //temp use for single volume debug
  GLOctreeVolumeTexturePool(const char* filename, int sizeZ, int sizeY, int sizeX, GLenum texType, GLenum internalType, GLenum dataType);
  GLenum getTexID(){return _texObject->GetTextureID();}		  
  GLenum getTexType(){return _texObject->GetTextureType();}
  int getDimX(){return _poolDimX;}
  int getDimY(){return _poolDimY;}
  int getDimZ(){return _poolDimZ;}



private:
  //helperFunctions
  int _getAvailableQuota();
	
  void _getChildrenClusterFromNode(octNode<T> * node, nodeTexel* nodeCluster);

  nodeTexel _createNodeTexel(int blockPointerZ, int blockPointerY, int blockPointerX,
			     int childrenClusterPointerZ, int childrenClusterPointerY, int childrenClusterPointerX, int subD, int type);

private:
  //svo
  std::map<int ,  sparseVoxelOctree<T>*> _svoLookUp; 
  sparseVoxelOctree<T>* _currentSVO;
  std::list<sparseVoxelOctree<T>*> RLUList;//TODO!!! big memory leaking with LRU

  GLTexture* _nodePool;
  GLTexture* _blockPool;
  SlotTracker3D _nodeTracker;
  SlotTracker3D _blockTracker;
  int _GPUPoolCapMB;

  int _poolDimX, _poolDimY, _poolDimZ;
  int _nodeClusterDim; //_nodeDim, 2*_nodeClusterDim = _nodeDim;
  int _blockPoolDim;

  //temp debug
  T* _buffer;
  GLTexture* _texObject;

  int PoolDim;
  bool _isSVOLoaded;
  bool _isPoolUsed;
	

};

template <typename T>
GLOctreeVolumeTexturePool<T>::GLOctreeVolumeTexturePool(int GPUPoolCapMB, int blockDim)
:  _currentSVO(NULL),
  _nodePool(NULL),
  _blockPool(NULL),
  _GPUPoolCapMB(GPUPoolCapMB),
  _isSVOLoaded(false),
  _isPoolUsed(false)

{
  //int blockDim = svo->getBlockSize()+svo->getBlockBoundarySize();
  float blockByteSize = float(blockDim*blockDim*blockDim*sizeof(T))/float(1024*1024);
  long totalBlockCount = long(float(_GPUPoolCapMB)/blockByteSize);

  PoolDim=1;
  while (PoolDim*PoolDim*PoolDim < totalBlockCount)  PoolDim++;

  int PoolVoxelDim = PoolDim*blockDim;

  //TODO currently hand code for segViewer
  _blockTracker.SetDim(PoolDim, PoolDim, PoolDim);
  _blockPoolDim = PoolDim;
  printf("blockPoolDim: %d\n", PoolDim);

  _blockPool = new GLTexture(PoolVoxelDim, PoolVoxelDim, PoolVoxelDim, GL_LUMINANCE_INTEGER_EXT,
			     GL_INTENSITY32UI_EXT, GL_NEAREST/*GL_LINEAR*/);
  _blockPool->LoadToGPU();	 //allocate empty texture in GPU
  _poolDimX = _poolDimY = _poolDimZ = PoolVoxelDim;

  //////////////////////init nodePool////////////////////////////////////////////////////////
  //TODO the parameter should depends on the PoolDim
  _nodeClusterDim = 30; //nodeCluster
  _nodeTracker.SetDim(_nodeClusterDim,_nodeClusterDim,_nodeClusterDim);
  _nodePool = new GLTexture(_nodeClusterDim*2,_nodeClusterDim*2,_nodeClusterDim*2, GL_LUMINANCE_ALPHA_INTEGER_EXT,
			    GL_LUMINANCE_ALPHA32UI_EXT, GL_NEAREST);
  _nodePool->LoadToGPU();


	
}

template <typename T>
GLOctreeVolumeTexturePool<T>::~GLOctreeVolumeTexturePool()
{
  if(_nodePool)
    delete _nodePool;
  if(_blockPool)
    delete _blockPool;

  
  //TODO free SVOs
}

template <typename T>
bool GLOctreeVolumeTexturePool<T>::AddSVO(sparseVoxelOctree<T>* svo, unsigned int indentifier)
{
  //DFS   update

  svo->DFS();
  svo->DFSUpdateNodeArray();	
	
  if(_getAvailableQuota() < svo->getBlockCount())
    return false;

  _isSVOLoaded = false;  
  if(_svoLookUp.find(indentifier) == _svoLookUp.end())
  {
    printf("\nAdd svo: %d",indentifier);
    _svoLookUp.insert(std::pair<int, sparseVoxelOctree<T>*>(indentifier, svo));
  }
  else
  {
    printf("\n delete svo");
    delete svo;
  }
  //UpLoadSVOGPU(indentifier);

  return true;
}

template <typename T>
sparseVoxelOctree<T>* GLOctreeVolumeTexturePool<T>::GetSVOByIndentifier(unsigned int indentifier)
{
  typename std::map<int , sparseVoxelOctree<T>*>::iterator it;
  if( (it = _svoLookUp.find(indentifier)) != _svoLookUp.end() )
    return _svoLookUp[indentifier];
  else
    return NULL;
}

//template <typename T>
//void GLOctreeVolumeTexturePool<T>::ClearAll()
//{
//  std::map<int , sparseVoxelOctree<T>*>::iterator it;
//  for ( it=_svoLookUp.begin() ; it != _svoLookUp.end(); it++ )
//    ClearSVOGPU((*it).first);
//  _currentSVO=0;
//}

template <typename T>
void GLOctreeVolumeTexturePool<T>::ClearAllCPU()
{
  //without typename gcc will give error
  typename std::map<int , sparseVoxelOctree<T>*>::iterator it;
  for ( it=_svoLookUp.begin() ; it != _svoLookUp.end(); it++ )
  {
    sparseVoxelOctree<T> *octree = (*it).second;
    if(octree)
      delete octree;
  }
  _svoLookUp.clear();

}

template <typename T>
void GLOctreeVolumeTexturePool<T>::ClearSVOGPU(unsigned int indentifier)
{
  if(!_isPoolUsed)
    return;
  _currentSVO = GetSVOByIndentifier(indentifier);
  if(!_currentSVO)
  {
    printf("Can't find Sparse Voxel Octree with indentifier: %d", indentifier);
    return;
  }
  
  _currentSVO->startNodeInteration();
  octNode<T>* currentNode;
  volume3D<T>* currentVolume;
  while( (currentNode = _currentSVO->getNextNode()) != NULL)
  {
    //do blockPool update
    if( (currentVolume=currentNode->getVolume()) != NULL )
    {
      //clear tracker
      int z, y, x;
      x = currentNode->nodeInfo.blockPointer[0];
      y = currentNode->nodeInfo.blockPointer[1];
      z = currentNode->nodeInfo.blockPointer[2];

      _blockTracker.FreeSlotAt(z, y, x);
    }

    //do nodePool update
    if( !currentNode->isLeaf())
    {
      //countNode++;
      int cz, cy, cx; //c for cluster
      cx = currentNode->nodeInfo.childrenNodeClusterPointer[0];
      cy = currentNode->nodeInfo.childrenNodeClusterPointer[1];
      cz = currentNode->nodeInfo.childrenNodeClusterPointer[2];

      _nodeTracker.FreeSlotAt(cz, cy, cx);
    }
  } 

  _isSVOLoaded = false;

}


template <typename T>
void GLOctreeVolumeTexturePool<T>::UpLoadSVOGPU(unsigned int indentifier)
{
  //up load all block
  _currentSVO = GetSVOByIndentifier(indentifier);
  if(!_currentSVO)
  {
    printf("Can't find Sparse Voxel Octree with indentifier: %d", indentifier);
    return;
  }

  _currentSVO->startNodeInteration();
  octNode<T>* currentNode;
  volume3D<T>* currentVolume;
  printf("start upload block\n");
  int count = 0;
  int countNode = 0;

  nodeTexel *childrenNodeCluster = new nodeTexel[8];

  while( (currentNode = _currentSVO->getNextNode()) != NULL)
  {
    //do blockPool update
    if( (currentVolume=currentNode->getVolume()) != NULL )
    {
      count++;
      //upload
      int z, y, x;
      _blockTracker.GetNextEmptySlot(z, y, x);

      currentNode->nodeInfo.blockPointer[0] = x;
      currentNode->nodeInfo.blockPointer[1] = y;
      currentNode->nodeInfo.blockPointer[2] = z;

      int OffsetZ = z*currentVolume->getDimZ();
      int OffsetY = y*currentVolume->getDimY();
      int OffsetX = x*currentVolume->getDimX();
      //printf("%4d Offset X:%3d Y:%3d Z:%3d, dim: %d\n", count, OffsetX, OffsetY, OffsetZ, currentVolume->getDimX());
      _blockPool->SubloadToGPU(OffsetX, OffsetY, OffsetZ, currentVolume->getDimX(), 
			       currentVolume->getDimY(), currentVolume->getDimZ(), (void*)currentVolume->getDataBuffer(), GL_UNSIGNED_INT );
    }

    //do nodePool update
    if( !currentNode->isLeaf())
    {
      countNode++;
      int cz, cy, cx; //c for cluster
      _nodeTracker.GetNextEmptySlot(cz, cy, cx);

      currentNode->nodeInfo.childrenNodeClusterPointer[0] = cx;
      currentNode->nodeInfo.childrenNodeClusterPointer[1] = cy;
      currentNode->nodeInfo.childrenNodeClusterPointer[2] = cz;

      _getChildrenClusterFromNode(currentNode, childrenNodeCluster);

      _nodePool->SubloadToGPU(cx*2, cy*2, cz*2, 2, 2, 2, childrenNodeCluster, GL_UNSIGNED_INT);
      //printf("cx cy cz: %d-%d-%d level: %d\n", cx, cy, cz, currentNode->getLevel());


      if(currentNode->isRoot())
      {
	      _currentSVO->setGPURootPos(cz, cy, cx);
      }
    }
  }
  delete childrenNodeCluster;
  printf("end upload block count:%d -- node count:%d\n", count, countNode);	
  _isSVOLoaded = true;
  _isPoolUsed = true;
	

}




/////////////////helper function//////////////////
template<typename T>
int GLOctreeVolumeTexturePool<T>::_getAvailableQuota() 
{
  //TODO
  return PoolDim*PoolDim*PoolDim;

}


template<typename T>
void  GLOctreeVolumeTexturePool<T>::_getChildrenClusterFromNode(octNode<T> *node, nodeTexel* nodeCluster)
{
  int index;
  octNode<T>* childNode;
  int blockPointerZ, blockPointerY, blockPointerX;
  int childrenClusterPointerZ,childrenClusterPointerY,childrenClusterPointerX;
  int subD, type; //type = false empty node

  int emptyCluster = 0;
  for(int k=0; k<2; k++)
    for(int j=0; j<2; j++)
      for(int i=0; i<2; i++)
      {
	index = k*2*2+j*2+i;
	childNode = node->getChild(k,j,i);
	if(childNode) //non empty children
	{
	  type = childNode->getVolume()? 1: 0;
	  subD = childNode->isLeaf()? 0 : 1;
	  blockPointerX = childNode->nodeInfo.blockPointer[0];
	  blockPointerY = childNode->nodeInfo.blockPointer[1];
	  blockPointerZ = childNode->nodeInfo.blockPointer[2];;
	  if(subD)
	  {
	    childrenClusterPointerX = childNode->nodeInfo.childrenNodeClusterPointer[0];
	    childrenClusterPointerY = childNode->nodeInfo.childrenNodeClusterPointer[1];
	    childrenClusterPointerZ = childNode->nodeInfo.childrenNodeClusterPointer[2];
	  }
	  else
	  {
	    childrenClusterPointerX = emptyCluster; childrenClusterPointerY = emptyCluster; childrenClusterPointerZ = emptyCluster;
	  }					
	}
	else
	{
	  type = 0;
	  subD = 0;
	  blockPointerX = 0; blockPointerY = 0; blockPointerZ = 0;
	  childrenClusterPointerX = emptyCluster; childrenClusterPointerY = emptyCluster; childrenClusterPointerZ = emptyCluster;
	}
				
	nodeCluster[index] = _createNodeTexel(blockPointerZ, blockPointerY, blockPointerX, 
					      childrenClusterPointerZ,childrenClusterPointerY, childrenClusterPointerX, subD, type);
	//printf("index: %d children index %d%d%d nodeCPointer: (%d %d %d), subD: %d, type: %d\n",index, i,j,k,childrenClusterPointerX,childrenClusterPointerY,childrenClusterPointerZ, subD, type);
      }

}

template<typename T>
nodeTexel  GLOctreeVolumeTexturePool<T>::_createNodeTexel(int blockPointerZ, int blockPointerY, int blockPointerX,
							  int childrenClusterPointerZ, int childrenClusterPointerY, int childrenClusterPointerX, int subD, int type)
{
  nodeTexel node;
  node.first = (subD<<31) | (type<<30) | (childrenClusterPointerZ<<20) | (childrenClusterPointerY<<10) | childrenClusterPointerX;
  node.second = (blockPointerZ<<20) | (blockPointerY<<10) | blockPointerX;
  return node;
}



/////////////////debug/////////////////
//raycaster->loadBlockPool("../../Data/test-comb-singleID-zyx-768X256X256.raw",768,256,256,GL_LUMINANCE_INTEGER_EXT, GL_INTENSITY32UI_EXT, GL_UNSIGNED_INT);
template<typename T>
GLOctreeVolumeTexturePool<T>::GLOctreeVolumeTexturePool(const char* filename, int sizeZ, int sizeY, int sizeX, GLenum texType, GLenum internalType, GLenum dataType)
{
  _poolDimX = sizeX; _poolDimY = sizeY; _poolDimZ = sizeZ;
  printf("single 3D texture data ... \n");
  _blockPool = new GLTexture(sizeX, sizeY, sizeZ, texType, internalType);
  _blockPool->ReadTextureFromFile(filename, dataType,1);
  //_texObject->LoadToGPU(buffer, dataType);
  _blockPool->LoadToGPU();
  //_texObject->LoadToGPUWithGLBuffer();
  _blockPool->SetFilterType(GL_NEAREST);

  GL::CheckErrors();
}

#endif 


