#ifndef _OBJ_ATTR_H_
#define _OBJ_ATTR_H_ 

#include <functional>
#include <vector>
#include <map>
#include "box2d/box2d.h"
class CPlugin;

// Application Object attribute
typedef struct ObjAttr {
  std::string strObjName;
  // sprite idx
  int iTileIdx; 

  // Dynamic Tile info
  b2Body* pBody;

  std::vector<std::string> vecTag;
  std::string strPhysicType;
  CPlugin* pPlugin;

  // Static Tile Info
  struct TileInfo_t {
    float fX_M,fY_M,fW_M,fH_M,fAngle;
  } TileInfo;
  
  // For background tiles
  float GetStaticTile_fX_M()  { return TileInfo.fX_M;}
  float GetStaticTile_fY_M()  { return TileInfo.fY_M;}
  float GetStaticTile_fAngle(){ return TileInfo.fAngle;}

  // For Dynamic tiles
  float GetBody_fX_M()  { return pBody->GetPosition().x;}
  float GetBody_fY_M()  { return pBody->GetPosition().y;}
  float GetBody_fAngle(){ return pBody->GetAngle();}
 
  float GetTile_fW_M()  { return TileInfo.fW_M;}
  float GetTile_fH_M()  { return TileInfo.fH_M;}

  std::function<float()> fX_M  ;
  std::function<float()> fY_M  ;
  std::function<float()> fW_M  ;
  std::function<float()> fH_M  ;
  std::function<float()> fAngle;

} ObjAttr_t;

class CObjDirectory {
  public:
  bool Find_by_Name(std::string &strObjName, ObjAttr_t* pObj);
  bool Find_by_Tag(std::string &strTag,std::vector<ObjAttr_t*> &vecObj);
  bool Find_by_Body(b2Body* pBody,ObjAttr_t* pObj);
  std::map<std::string,ObjAttr_t*> m_mapObjs;

           // Tag          body
  std::map<std::string,std::vector<b2Body*>> m_mapTags;
  // Map variable that is mapping tag - Objects
           // Tag          Object
  std::map<std::string,std::vector<ObjAttr_t*>> m_mapTagObj;




};
#endif /* ifndef _OBJ_ATTR_H_ */
