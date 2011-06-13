#include "LRUcachePool.h"
#include <iostream>
#include <list>
using namespace std;
extern "C" void LRUcacheTest()
{
/*	//test swapping two node using slice
	list<int> ilist;
	ilist.push_back(1);
	ilist.push_back(2);
	ilist.push_back(3);
	ilist.push_back(4);

	list<int>::iterator i = ilist.begin();
	list<int>::iterator j = ilist.end();
	j--;
	cout<<*i<<" "<<*j<<endl;
	list<int>::iterator k = ilist.begin();
	for (; k!=ilist.end();k++)
	cout<<" "<<*k;
	cout<<endl;

	ilist.splice(ilist.begin(),ilist,j);
	cout<<*i<<" "<<*j<<endl;
	k = ilist.begin();
	for (; k!=ilist.end();k++)
	cout<<" "<<*k;
	cout<<endl;
*/	
	
  LRUcache<long, float> cache = LRUcache<long, float>(1024*256*sizeof(float), 50);
  cache.AllocateGlobalCacheBuffer();
  cout<<"cacheCapacity: "<<cache.GetMaxCacheCapacity()<<" Entry"<<endl;
  //float *value = new float[1024*256];
  long index = 1l;
  for(int i=0; i<100; i++)
  {
    float *value = cache.GetLRUBufferLocation();
    memset(value, 0, 256*sizeof(float));
    cache.Add(index);
    cout<<cache.Find(index)<<endl;;
    index++;
    cache.outputMap();
    if(index>50)
      index = 1;
  }

}
