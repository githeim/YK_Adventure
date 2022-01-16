#ifndef _OBJ_ATTR_H_
#define _OBJ_ATTR_H_ 

class CPlugin;

// Application Object attribute
typedef struct ObjAttr {
  b2Body* pBody;
  std::vector<std::string> vecTag;
  CPlugin* pPlugin;
  // Static Tile Info
  //       tileidx   x     y     w     h    angle
  //std::tuple<int, float,float,float,float,float> StaticTileInfo;
  struct StaticTileInfo_t {
    int iTileIdx;
    float fX_M,fY_M,fW_M,fH_M,fAngle;
  } StaticTileInfo;
  
  int   *piTileIdx;
  float *pfX_M;
  float *pfY_M;
  float *pfW_M;
  float *pfH_M;
  float *pfAngle_M;

} ObjAttr_t;
#endif /* ifndef _OBJ_ATTR_H_ */
