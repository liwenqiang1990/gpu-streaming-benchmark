#ifndef _LRU_CACHE_H

#include <unordered_map>
#include <list>
#include <stdio.h>
#include <iostream>
//using namespace std;

#include "MemoryAlloc.h"

//const block Size cache implementation, for single thread accessing
// same key different 

template <typename keyT, typename T>
class LRUcache
{
	struct cacheEntry
	{keyT index; T*  buffer;};
	typedef std::list<cacheEntry> List;
	typedef std::tr1::unordered_map<keyT, typename List::iterator> unorderedMap;
	//have to add the typename (prefix with 'typename' to indicate a type) , !!!because I can write a a class list<int> with a static int *iterator* member

public:
	
	LRUcache(long dataBlockSize, long maxMBCacheUsed);
	~LRUcache();
	void AllocateGlobalCacheBuffer();
	
	T* Find(keyT key);
	void Update(keyT key); //used when you don't need the actual data in the cache just to increase the priority of that entry 

	void Add(keyT key /* the second parameter is nextCacheBlock*/);  
	void Add(keyT key, T* buffer);
	void Delete(keyT key);

	long GetSingleCacheBlockSize() {return _dataBlockByteSize;}
	long GetMaxCacheCapacity() {return _maxCacheEntryNum;}
	T* GetLRUBufferLocation() { return _nextCacheBlock;}//return the LRU block or unused block
	//long getCurrentCacheSize();

	///debug
	void outputMap();

protected:
	//helper function
	inline bool isHasKey(keyT key);
	inline void updateNextCacheBlock();
	inline void allocMemoryIfNot();

	//members
	long _dataBlockByteSize; //Byte
	long _maxMBCacheUsed; //MB
	long _maxCacheEntryNum;
	long _cacheEntryNum; //

	List _cacheList; //double linked list
	unorderedMap _cacheMap;

	bool _isAllocated;
	T* _cacheBuffer;
	T* _nextCacheBlock;

};
#endif

//////////////////////////////

template <typename keyT, typename T>
LRUcache<keyT, T>::LRUcache(long dataBlockByteSize, long maxMBCacheUsed)
	:_dataBlockByteSize(dataBlockByteSize),
	_maxMBCacheUsed(maxMBCacheUsed),
	_cacheEntryNum(0),
	_cacheBuffer(NULL),
	_nextCacheBlock(NULL),
	_isAllocated(false)
{
	_maxCacheEntryNum = _maxMBCacheUsed*1024*1024/_dataBlockByteSize;

}

template <typename keyT, typename T>
LRUcache<keyT, T>::~LRUcache()
{
	if(_cacheBuffer)
		MemoryAlloc::PageLockedMallocFree((void*)_cacheBuffer);
}


template <typename keyT, typename T>
void LRUcache<keyT, T>::AllocateGlobalCacheBuffer()
{
// allocate page locked memory for faster caching
	_cacheBuffer = (T*)MemoryAlloc::PageLockedMalloc(_maxMBCacheUsed*1024*1024);
	_nextCacheBlock = _cacheBuffer;//init nextCacheBlock Pointer
}

template <typename keyT, typename T>
T* LRUcache<keyT, T>::Find(keyT key) 
{
	if(!isHasKey(key)) 
		return NULL; 
	Update(key); 
	return (*_cacheMap[key]).buffer; 
}

template <typename keyT, typename T>
void LRUcache<keyT, T>::Update(keyT key) 
{  
	//use splice to swap node but the pointer(iterator) remain same
	_cacheList.splice(_cacheList.begin(),_cacheList,_cacheMap[key]);
} 

template <typename keyT, typename T>
void LRUcache<keyT, T>::Add(keyT key) 
{
	allocMemoryIfNot();

	if(_cacheEntryNum < _maxCacheEntryNum)
	{
		cacheEntry entry;
		entry.index = key; entry.buffer = _nextCacheBlock;

		_cacheList.push_front(entry); 	
		_cacheMap.insert( unorderedMap::value_type(key, _cacheList.begin())); 
		_cacheEntryNum++;//increase cache entry
		Update(key);
	}
	else //pool is already full
	{
		List::iterator lastEntry = _cacheList.end();
		lastEntry--;
		//erase old map to the LRU entry in the list
		_cacheMap.erase( lastEntry->index);
		//put new entry pointer in
		(*lastEntry).index = key;
		_cacheMap.insert(unorderedMap::value_type(key, lastEntry));
		Update(key); //move the
	}

	//update next cache block after occupy current one
	updateNextCacheBlock();
} 

template <typename keyT, typename T>
void LRUcache<keyT, T>::Add(keyT key, T* buffer) 
{
	allocMemoryIfNot();

	memcpy(_nextCacheBlock, buffer, this->_dataBlockByteSize);

	Add(key);

} 

template <typename keyT, typename T>
void LRUcache<keyT, T>::Delete(keyT key) 
{
	if(!isHasKey(key)) 
		return; 
	_cacheList.erase(_cacheMap[key]); _cacheMap.erase(key); 
}

//////////////////////////////////
//helper function

template <typename keyT, typename T>
bool LRUcache<keyT, T>::isHasKey(keyT key)
{
	if(_cacheMap.find(key) == _cacheMap.end())
		return false;
	else
		return true;
}

template <typename keyT, typename T>
void LRUcache<keyT, T>::updateNextCacheBlock()
{
	if(_cacheEntryNum < _maxCacheEntryNum)
		_nextCacheBlock = _cacheBuffer + _cacheEntryNum*(_dataBlockByteSize/sizeof(T));
	else
		_nextCacheBlock = _cacheList.back().buffer;
}

template <typename keyT, typename T>
void LRUcache<keyT, T>::allocMemoryIfNot()
{
	if(!_isAllocated)
	{
		this->AllocateGlobalCacheBuffer();
		_isAllocated = true;
	}
}

/////////debug
template <typename keyT, typename T>
void LRUcache<keyT, T>::outputMap()
{
	unorderedMap::iterator ii = _cacheMap.begin() ;
   for( ;   ii != _cacheMap.end(); ++ii)
   {
	  std::cout << ii->first << ": " << (*ii->second).buffer << std::endl;
   }
}