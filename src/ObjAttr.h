#ifndef _OBJ_ATTR_H_
#define _OBJ_ATTR_H_ 
#include "CPhysic_World.h"

class CPlugin;

// Application Object attribute
typedef struct ObjAttr {
  b2Body* pBody;
  std::vector<std::string> vecTag;
  CPlugin* pPlugin;
  // Static Tile Info
  //       tileidx   x     y     w     h    angle
  std::tuple<int, float,float,float,float,float> StaticTileInfo;
  
  int iTileIdx;
  float *fX_M;
  float *fY_M;
  float *fW_M;
  float *fH_M;
  float *fAngle_M;

} ObjAttr_t;
#endif /* ifndef _OBJ_ATTR_H_ */
