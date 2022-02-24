#ifndef _CPHYSIC_WORLD_H_
#define _CPHYSIC_WORLD_H_
#include <stdio.h>
#include "box2d/box2d.h"
#include "TMX_Reader/CTMX_Reader.h"

#include"ObjAttr.h"

#define WORLDSCALE_PIXEL_PER_METER (18.f)
class WorldContactListener;
class CPhysic_World {
public:
  CPhysic_World(TMX_Ctx & TMX_context, CObjDirectory &ObjDirectory,
      float fWorldScale_Pixel_per_Meter=WORLDSCALE_PIXEL_PER_METER) {
    m_fScale_Pixel_per_Meter = fWorldScale_Pixel_per_Meter;
    if ( Create_World(TMX_context,ObjDirectory)) {
      printf("\033[1;31m[%s][%d] :x: Err on create world \033[m\n",
          __FUNCTION__,__LINE__);
      return;
    }

  }

  ~CPhysic_World() {
    printf("\033[1;33m[%s][%d] :x: chk \033[m\n",__FUNCTION__,__LINE__);
    Destroy_World();
  }

  int Create_World(TMX_Ctx & TMX_context,
                   CObjDirectory &ObjDirectory); 


  int Create_BodyDefs(std::map<std::string,b2BodyDef*>& mapBodyDef);
  int Create_EdgeShapes(std::map<std::string,b2EdgeShape*>&  mapEdgeshape);
  int Create_PolygonShape(std::map<std::string,b2PolygonShape*>& mapPolygonShape);
  std::string Create_Element(TMX_Ctx & TMX_context,int iTileIdx,
                         float fX_M,float fY_M,
                         CObjDirectory &ObjDirectory);


  int Register_Background(std::map<std::string,ObjAttr_t*>& mapObjs,
                          std::string &strObjName,
                          std::string &strTag, std::string &strPhysicType,
                          int &iTileIdx,
                          float &fX_M,float &fY_M,float &fW_M,float &fH_M,
                          float &fAngle);



  int Register_Obj(std::map<std::string,ObjAttr_t*>& mapObjs,
                   std::string &strObjName,
                   b2Body* pBody,
                   std::vector<std::string> &vecTag, std::string &strPhysicType,
                   int &iLayerIdx,
                   int &iTileIdx,
                   float &fW_M,float &fH_M,
                   float &fAngle,
                   std::map<std::string,std::vector<ObjAttr_t*>> &mapTagObj
                   );

  int Destroy_World();

  void SpinWorld(double dbTimeStep,  
                 int iVelocityIter=6, int iPositionIter=2); 

  b2World* m_pWorld;
  //                  tileidx   x     y    w      h    angle
  std::vector<std::tuple<int, float,float,float,float,float>> m_vecBackground;

  std::map<std::string,b2BodyDef*>      m_mapBodyDef;
  std::map<std::string,b2PolygonShape*> m_mapPolygonShape;
  std::map<std::string,b2EdgeShape*>    m_mapEdgeshape;

  // Map variable that is mapping tag - Objects
           // Tag          Object
  std::map<std::string,std::vector<ObjAttr_t*>> m_mapTagObj;

  float m_fScale_Pixel_per_Meter;
  int m_iObjCnt =0;
};

#endif /* ifndef _CPHYSIC_WORLD_H_ */
