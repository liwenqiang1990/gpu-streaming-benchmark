#ifndef _SLOT_TRACKER_H
#define _SLOT_TRACKER_H

#include <list>

#include "octNode.h"

/*
  work as GPU 3D Pool's CPU manager, tracking empty blocks

  function:
  Return next empty slot
  Free certain slot


*/

class SlotTracker3D
{
public:
  SlotTracker3D();
  SlotTracker3D(int z, int y, int x);
  void SetDim(int z, int y, int x); //init slot dim
  void GetNextEmptySlot(int &z, int &y, int &x);
  void RLUGetNextEmptySlot(int &z, int &y, int &x);
  void UpdateSlot(int z, int y, int x);
  void FreeSlotAt(int z, int y, int x);
  void FreeAll();


  //debug
  int IsSlotFree(int z, int y, int x);


private:
  int _dimZ, _dimY, _dimX;
	
  struct SlotPos
  {
    int z, y, x;		 
    int occupy; //0 is empty
  };		 
  //SlotPos* = NULL indicate empty slot
  std::list<SlotPos>   _slotList;
  std::list<SlotPos>::iterator _emptySlotStartPos;
  std::vector<std::list<SlotPos>::iterator> _posToSlotLookUpArray;

};

inline SlotTracker3D::SlotTracker3D()
{

}

inline SlotTracker3D::SlotTracker3D(int z, int y, int x)
{
  SetDim(z, y, x);
}

inline void SlotTracker3D::SetDim(int z, int y, int x)
{
  _dimZ = z, _dimY = y,  _dimX = x;
  _posToSlotLookUpArray.empty();

  for(int k=0; k<_dimZ; k++)
    for(int j=0; j<_dimY; j++)
      for(int i=0; i<_dimX; i++)
      {
	      SlotPos slot; slot.occupy = 0; 
	      slot.x = i; slot.y = j; slot.z = k;
	      _slotList.push_back(slot);
	      _posToSlotLookUpArray.push_back(--_slotList.end()); //iterator to the last element
      }

  _emptySlotStartPos = _slotList.begin();

}

inline void SlotTracker3D::GetNextEmptySlot(int &z, int &y, int &x) //this slot will be marked as occupied
{
  if(_emptySlotStartPos == _slotList.end())
  {
    z = -1; y=-1; x=-1;
    return;
  }

  x = _emptySlotStartPos->x;
  y = _emptySlotStartPos->y;
  z = _emptySlotStartPos->z;
  _emptySlotStartPos->occupy = 1;
  std::list<SlotPos>::iterator nonEmpty = _emptySlotStartPos;
  _emptySlotStartPos ++;
  _slotList.splice(_slotList.begin(), _slotList, nonEmpty);

}

inline void SlotTracker3D::FreeSlotAt(int z, int y, int x)
{
  if ( z>_dimZ || y>_dimY || x>_dimX)
    return;
		
  std::list<SlotPos>::iterator slot = _posToSlotLookUpArray[z*_dimY*_dimX + y*_dimX + x];
  slot->occupy = 0;
  _slotList.splice(_slotList.end(), _slotList, slot);

  //
  if(_emptySlotStartPos == _slotList.end())
    _emptySlotStartPos = --_slotList.end();


}

inline void SlotTracker3D::FreeAll()
{
  _slotList.clear();
  _posToSlotLookUpArray.clear();
  SetDim(_dimZ, _dimY, _dimX); //init slot dim
}



//debug
inline int SlotTracker3D::IsSlotFree(int z, int y, int x)
{
  if ( z>_dimZ || y>_dimY || x>_dimX)
    return -1;

  return _posToSlotLookUpArray[z*_dimY*_dimX + y*_dimX + x]->occupy;
}




#endif
