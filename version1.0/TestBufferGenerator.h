#ifndef TEST_BUFFER
#define TEST_BUFFER
//data generate function

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

template<typename T, int vectorLen>  
class TestBufferGenerator
{
public:
  TestBufferGenerator(int bufferSizeMB , int blockDimX, int blockDimY, int blockDimZ)
  {
    srand(1);
    _typesize = sizeof(T);
    _columeNum = vectorLen;

    _elementSize = bufferSizeMB*1024*1024/(_columeNum*_typesize);
    _buffer = (T*)malloc(_elementSize*_typesize);

    assert(_buffer);

    //generate  random data
    for(unsigned int i=0; i<_elementSize; i++)
    {
      _buffer[i] = T(rand()%( (2<<sizeof(T))-1) );
    }

    _blockDimX = blockDimX;
    _blockDimY = blockDimY;
    _blockDimZ = blockDimZ;

    _index = 0;
    _offset  = 0;
    printf("finish data generation!\n");
  }

  ~TestBufferGenerator()
  {
    free(_buffer);
  }

  T* getNextBlock()
  {
    T* block = _buffer + _index;
    
    _index += _blockDimX*_blockDimY*_blockDimZ*_columeNum;

    if(_index >= _elementSize - 1 - _blockDimX*_blockDimY*_blockDimZ*_columeNum)
    {
      //_offset++;
      if(_offset >= _elementSize - 1 - _blockDimX*_blockDimY*_blockDimZ*_columeNum)
      {
        printf("offset set to zero!\n");
        _offset = 0;
      }
      //shift the starting point of the buffer(make sure each time uplaod different block)
      _index = _offset;
    }

    return block;
  }


private:
  T* _buffer;
  unsigned int _offset;
  unsigned int _index;
  unsigned int _elementSize; //_index < _elementSize -1
  unsigned int _typesize;
  int _columeNum;

  int _blockDimX, _blockDimY, _blockDimZ;


};

#endif
