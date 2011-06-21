#ifndef TEST_BUFFER
#define TEST_BUFFER
//data generate function

#include <stdlib.h>
#include <assert.h>
/*
template<typename T, int vectorLen>  
class TestBufferGenerator
{
public:
  //template<typename T, int vectorLen>
  TestBufferGenerator(int bufferSizeMB , int blockDimX, int blockDimY, int blockDimZ)
  {
    srand(1);
    _typesize = sizeof(T);
    _columeNum = vectorLen;

    _elementSize = bufferSizeMB*1024*1024/(_columeNum*_typesize);
    T* _buffer = (T*)malloc(_elementSize*_typesize);

    assert(_buffer);

    //generate  random data
    for(unsigned int i=0; i<_elementSize; i++)
    {
      _buffer[i] = 'a';//T(rand()%( (2<<sizeof(T))-1) );
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
    
    _index += _blockDimX*_blockDimY*_blockDimZ;

    if(_index >= _elementSize - 1 - _blockDimX*_blockDimY*_blockDimZ)
    {
      _offset++;
      if(_offset >= _elementSize - 1 - _blockDimX*_blockDimY*_blockDimZ)
        _offset = 0;
      //shift the starting point of the buffer(make sure each time uplaod different block)
      _index = _offset;
    }

    printf("index %d  ", _index); 
    printf("%d - block[0]", block[0]);
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


};*/

class TestBufferGenerator
{
public:
  //template<typename T, int vectorLen>
  TestBufferGenerator(int bufferSizeMB , int blockDimX, int blockDimY, int blockDimZ)
  {
    srand(1);
    _typesize = sizeof(unsigned char);
    _columeNum = 1;

    _elementSize = bufferSizeMB*1024*1024/(_columeNum*_typesize);
   unsigned char* _buffer = (unsigned char*)malloc(_elementSize*_typesize);


    //generate  random data
    for(unsigned int i=0; i<_elementSize; i++)
    {
      _buffer[i] = 'a';//T(rand()%( (2<<sizeof(T))-1) );
    }

    _blockDimX = blockDimX;
    _blockDimY = blockDimY;
    _blockDimZ = blockDimZ;

    _index = 0;
    _offset  = 0;
    printf("finish data generation! %d\n", _buffer[1]);

    //assert(_buffer);
  }

  ~TestBufferGenerator()
  {
    //free(_buffer);
  }

 unsigned char* getNextBlock()
  {
   unsigned char* block;// = _buffer + _index;
    
    _index += _blockDimX*_blockDimY*_blockDimZ;

    if(_index >= _elementSize - 1 - _blockDimX*_blockDimY*_blockDimZ)
    {
      _offset++;
      if(_offset >= _elementSize - 1 - _blockDimX*_blockDimY*_blockDimZ)
        _offset = 0;
      //shift the starting point of the buffer(make sure each time uplaod different block)
      _index = _offset;
    }

    printf("index %d  ", _index); 
    printf("%d - block[0]", _buffer[0]);
    return block;
  }


private:
  unsigned char* _buffer;
  unsigned int _offset;
  unsigned int _index;
  unsigned int _elementSize; //_index < _elementSize -1
  unsigned int _typesize;
  int _columeNum;

  int _blockDimX, _blockDimY, _blockDimZ;


};


#endif