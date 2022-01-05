#ifndef _CPHYSIC_WORLD_H_
#define _CPHYSIC_WORLD_H_
#include <stdio.h>
#include "box2d/box2d.h"
#include "TMX_Reader/CTMX_Reader.h"

//#include"ObjAttr.h"

class WorldContactListener;
class CPhysic_World {
public:
  CPhysic_World(TMX_Ctx & TMX_context,
      float fWorldScale_Pixel_per_Meter=18.f) {
    m_fScale_Pixel_per_Meter = fWorldScale_Pixel_per_Meter;
    if ( Create_World(TMX_context)) {
      printf("\033[1;31m[%s][%d] :x: Err on create world \033[m\n",
          __FUNCTION__,__LINE__);
      return;
    }

  }
  ~CPhysic_World() {
    printf("\033[1;33m[%s][%d] :x: chk \033[m\n",__FUNCTION__,__LINE__);
    Destroy_World();
  }

  int Create_World(TMX_Ctx & TMX_context); 

  b2Body* Create_Element(TMX_Ctx & TMX_context,int iTileIdx,float fX_M,float fY_M);

  int Destroy_World();

  void SpinWorld(double dbTimeStep,  
                 int iVelocityIter=6, int iPositionIter=2); 

  b2World* m_pWorld;
  //        body         tile index, width, height
  std::map<b2Body*,std::tuple<int,  float,float>> m_mapBodies;
  //                  tileidx   x     y    w      h    angle
  std::vector<std::tuple<int, float,float,float,float,float>> m_vecBackground;

  std::map<std::string,b2BodyDef*>      m_mapBodyDef;
  std::map<std::string,b2PolygonShape*> m_mapPolygonShape;
  std::map<std::string,b2EdgeShape*>    m_mapEdgeshape;

  std::map<std::string,std::vector<b2Body*>> m_mapTags;
  float m_fScale_Pixel_per_Meter;
  WorldContactListener* m_pContactListener = nullptr;
};

#endif /* ifndef _CPHYSIC_WORLD_H_ */
