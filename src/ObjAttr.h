#ifndef _OBJ_ATTR_H_
#define _OBJ_ATTR_H_ 

#include <functional>
#include <vector>
#include <map>
#include "box2d/box2d.h"

#define LAYER_BACKGROUND (1)
#define LAYER_GROUND     (2)
#define LAYER_OBJ        (3)
#define LAYER_UI         (4)

class CPlugin;

// Application Object attribute
typedef struct ObjAttr {
  std::string strObjName;
  // sprite idx
  int iTileIdx; 
  // display layer idx
  int iLayerIdx;

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
  ~CObjDirectory();
  bool Find_by_Name(std::string &strObjName, ObjAttr_t* &pObj);
  bool Find_by_Tag(std::string &strTag,std::vector<ObjAttr_t*> &vecObj);
  bool Find_by_Body(b2Body* pBody,ObjAttr_t* &pObj);
  bool UpdateDirectory(std::map<std::string,ObjAttr_t*> &mapObjs);
  bool UpdateDirectory() {
    return UpdateDirectory(m_mapObjs);
  }

  void Clear();

  // Main table of objects
  std::map<std::string,ObjAttr_t*> m_mapObjs;
  // the plugins plugged to Obj
  std::vector<ObjAttr_t*> m_vecObjPluginInstance;

  // Map variable that is mapping tag - Objects
           // Tag          Object
  std::map<std::string,  std::vector<ObjAttr_t*>> m_mapTagObj;
          //layer idx      Object
  std::map<int ,         std::vector<ObjAttr_t*>> m_mapLayerObj;
          //body ptr      Object
  std::map<b2Body*,      ObjAttr_t*> m_mapBody_Obj;


  // The tiles are drown by these sequence
  std::vector<int> m_vecLayerSequence = 
                             {LAYER_GROUND,LAYER_OBJ,LAYER_UI};
                           //  {LAYER_BACKGROUND,LAYER_GROUND,LAYER_OBJ};
                           // {LAYER_BACKGROUND,LAYER_GROUND,LAYER_OBJ,LAYER_UI};
};
#endif /* ifndef _OBJ_ATTR_H_ */
