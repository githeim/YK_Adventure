#include "CPhysic_World.h"
#include "algorithm"

static float GetDistance(float fX1,float fY1, float fX2, float fY2) {
  // Calculating distance
  return std::sqrt(pow(fX2 - fX1, 2) +
      pow(fY2 - fY1, 2) * 1.0);
};

class WorldContactListener : public b2ContactListener
{
public:
  WorldContactListener(CPhysic_World* pWorld):m_pPhysic_World(pWorld) {
  }
  ~WorldContactListener() {
  }
  
  void BeginContact(b2Contact* contact) {

  }

  void EndContact(b2Contact* contact) {
#if 0 // :x: for test
    std::tuple Data = m_pPhysic_World->m_mapBodies[contact->GetFixtureA()->GetBody()];
    printf("\033[1;33m[%s][%d] :x: Fix A =%d \033[m\n",
        __FUNCTION__,__LINE__,std::get<0>(Data));

    Data = m_pPhysic_World->m_mapBodies[contact->GetFixtureB()->GetBody()];
    printf("\033[1;33m[%s][%d] :x: Fix B =%d \033[m\n",
        __FUNCTION__,__LINE__,std::get<0>(Data));

    auto pManifold = contact->GetManifold();
      
    printf("\033[1;32m[%s][%d] :x: manifold type =%d, points =%d \033[m\n",
        __FUNCTION__,__LINE__,pManifold->type,pManifold->pointCount);
    printf("\033[1;33m[%s][%d] :x: chk %d %d \033[m\n",
        __FUNCTION__,__LINE__,pManifold->localPoint.x,pManifold->localPoint.y);
    for (auto point : pManifold->points) {
      printf("\033[1;33m[%s][%d] :x: points  %f %f  \033[m\n",
          __FUNCTION__,__LINE__,point.localPoint.x,point.localPoint.y);
    }
    
    printf("\033[1;33m[%s][%d] :x: normal %f %f \033[m\n",
        __FUNCTION__,__LINE__,pManifold->localNormal.x,pManifold->localNormal.y);
#endif // :x: for test




  }

  CPhysic_World* m_pPhysic_World;
};
int CPhysic_World::Create_World(TMX_Ctx &TMX_context,
                                std::map<std::string,ObjAttr_t>& mapObjs) {
  b2Vec2 gravity(0.0f,-10.0f);
  m_pWorld = new b2World(gravity);
  m_pContactListener = new WorldContactListener(this);
  m_pWorld->SetContactListener(m_pContactListener);

  // Create body define
  Create_BodyDefs(m_mapBodyDef);


  // Create edge shape
  Create_EdgeShapes(m_mapEdgeshape);

  Create_PolygonShape(m_mapPolygonShape);


  float fLeftEdge_M;
  float fTopEdge_M;
  int iIdxCnt=0;

  // 기본 타일 사이즈 (18 pixel = 1M)
  
  // 기본 타일 사이즈 pixel
  float fBaseTileWidth = (float)TMX_context.iTileWidth;
  float fBaseTileHeight =(float)TMX_context.iTileHeight;
  // 기본 타일 사이즈 meter
  float fBaseTileWidth_M = (float)TMX_context.iTileWidth/(float)TMX_context.iTileWidth;
  float fBaseTileHeight_M =(float)TMX_context.iTileHeight/(float)TMX_context.iTileHeight;

  // 레이어 별로 아이템을 읽어서 body를 생성한다
  for (auto item : TMX_context.mapLayers) {
    printf("\033[1;33m[%s][%d] :x: Layer %s \033[m\n",
        __FUNCTION__,__LINE__,item.first.c_str());
    auto &Layer = item.second;
    // 물리 영역 좌상단
    fLeftEdge_M = - ( Layer.iWidth/2.0f )* fBaseTileWidth_M; ;
    fTopEdge_M  =   ( Layer.iHeight/2.0f)* fBaseTileHeight_M;
    iIdxCnt = 0;

    for ( auto iIdx : Layer.vecTileIdx) {
      TileSet *pTileSet;
      int iFirstgid;
      CTMX_Reader::GetTileSetFromIdx(TMX_context,iIdx,pTileSet,iFirstgid);

      // 해당 인덱스의 타일 사이즈, base tile을 1M로 상정하고 Meter를 계산
      float fTileSizeWidth_M =(float)(pTileSet->iTileWidth) /fBaseTileWidth;
      float fTileSizeHeight_M=(float)(pTileSet->iTileHeight)/fBaseTileHeight;
      
      // if the tile has specified its own size, it will be apply to 
      // physic engine
      std::tuple<int,int,int,int> Tile_XYWH_size;
      if (!CTMX_Reader::GetSpecifiedTileSize(TMX_context,iIdx,Tile_XYWH_size)) {
        fTileSizeWidth_M  = (float)std::get<2>(Tile_XYWH_size)/fBaseTileWidth;
        fTileSizeHeight_M = (float)std::get<3>(Tile_XYWH_size)/fBaseTileHeight;
      }

      // X,Y coordination in Index
      int iX_idx= iIdxCnt % Layer.iWidth;
      int iY_idx= iIdxCnt / Layer.iWidth;
      // X,Y coordination in meter
      // 좌표는 기본 타일 사이즈를 기준으로 계산한다
      float fX_M = fLeftEdge_M+ iX_idx* fBaseTileWidth_M  + fBaseTileWidth_M /2.f;
      // 해당 타일이 기본 타일보다 커도 바닥 위치를 일치시켜 배치한다
      float fY_M = fTopEdge_M - iY_idx* fBaseTileHeight_M - fBaseTileHeight_M/2.f
        + ((fTileSizeHeight_M /2)-(fTileSizeHeight_M/2));
      if (iIdx !=0) {
        Create_Element(TMX_context, iIdx,
            fX_M,fY_M);
      }

      iIdxCnt++;
    } 

  }
#if 0 // :x: for test big ground
  b2Body* pBody =m_pWorld->CreateBody(m_mapBodyDef["ground_tile"]);
  b2PolygonShape* pPolygon = new b2PolygonShape();
  pPolygon->SetAsBox(30/2.f, 2/2.f);

  pBody->CreateFixture(pPolygon,0.f);
  pBody->SetTransform(b2Vec2(0,5),0.f); 
  m_mapBodies[pBody] = std::tuple<int,float,float>(123,30,2);
#endif // :x: for test

  for (auto item : m_mapTags) {
    for (auto pBody : item.second) {
    printf("\033[1;36m[%s][%d] :x: m_mapTags %s %p \033[m\n",
        __FUNCTION__,__LINE__,item.first.c_str(),pBody);
    }

  }
  return 0;
}

int CPhysic_World::Destroy_World() {
  for (auto BodyDef : m_mapBodyDef) {
    delete BodyDef.second;
  }
  
  for (auto PolygonShape : m_mapPolygonShape) {
    delete PolygonShape.second;
  }
  for (auto EdgeShape : m_mapEdgeshape) {
    delete EdgeShape.second;
  }

  for (auto body : m_mapBodies) {
    m_pWorld->DestroyBody(body.first);
  }
  if (m_pContactListener)
    delete m_pContactListener;

  delete m_pWorld;
  return 0;
}

void CPhysic_World::SpinWorld(double dbTimeStep, 
    int iVelocityIter, int iPositionIter ) {
  m_pWorld->Step(dbTimeStep, iVelocityIter, iPositionIter);
  return;
}
b2Body* CPhysic_World::Create_Element(TMX_Ctx &TMX_context, int iTileIdx,
                                  float fX_M,float fY_M) {
  std::string strPhysicType;
  // 기본 타일 사이즈 pixel
  float fBaseTileWidth = (float)TMX_context.iTileWidth;
  float fBaseTileHeight =(float)TMX_context.iTileHeight;

  b2Body* pBody = nullptr;
  TileSet *pTileSet;
  int iFirstgid;
  CTMX_Reader::GetTileSetFromIdx(TMX_context,iTileIdx,pTileSet,iFirstgid);
  // 해당 인덱스의 타일 사이즈, base tile을 1M로 상정하고 Meter를 계산
  float fTileSizeWidth_M =(float)(pTileSet->iTileWidth) /fBaseTileWidth;
  float fTileSizeHeight_M=(float)(pTileSet->iTileHeight)/fBaseTileHeight;

  // if the tile has specified its own size, it will be apply to 
  // physic engine
  std::tuple<int,int,int,int> Tile_XYWH_size;
  if (!CTMX_Reader::GetSpecifiedTileSize(TMX_context,iTileIdx,Tile_XYWH_size)) {
    fTileSizeWidth_M  = (float)std::get<2>(Tile_XYWH_size)/fBaseTileWidth;
    fTileSizeHeight_M = (float)std::get<3>(Tile_XYWH_size)/fBaseTileHeight;
  }

  if ( !CTMX_Reader::GetPhysicType(TMX_context,iTileIdx,strPhysicType)) {
    if (strPhysicType == "Background") {
      m_vecBackground.push_back(
          std::tuple<int, float,float,float,float,float>(
            iTileIdx,fX_M,fY_M,1.33,1.33,0));
    }

    if (strPhysicType == "Dynamic") {
      pBody = m_pWorld->CreateBody(m_mapBodyDef["dynamic_tile"]);
      std::string strTag;
      if (!CTMX_Reader::GetTag(TMX_context, iTileIdx, strTag)) {
        printf("\033[1;32m[%s][%d] :x: Tag %s \033[m\n",__FUNCTION__,__LINE__,strTag.c_str());
        m_mapTags[strTag].push_back(pBody); 
      }
      // Find the matched size of polygon
      auto Polygon = std::find_if(m_mapPolygonShape.begin(),m_mapPolygonShape.end(),
          [&fTileSizeWidth_M,&fTileSizeHeight_M](auto item) {
          b2PolygonShape* pShape = item.second;
          b2Vec2* vertices = pShape->m_vertices;
          float fWidth = GetDistance(vertices[0].x,vertices[0].y,
              vertices[1].x,vertices[1].y);
          float fHeight = GetDistance(vertices[1].x,vertices[1].y,
              vertices[2].x,vertices[2].y);
          if (fWidth == fTileSizeWidth_M && fHeight == fTileSizeHeight_M)
            return true;
          return false;
          });
      if (Polygon == m_mapPolygonShape.end())
      {
        // if there is no same polygon size, then make the new polygon
        std::string strPolygonName=std::to_string(fTileSizeWidth_M) + "_"+
          std::to_string(fTileSizeHeight_M);

        b2PolygonShape* pPolygon = new b2PolygonShape();
        pPolygon->SetAsBox(fTileSizeWidth_M/2.f, fTileSizeHeight_M/2.f);
        m_mapPolygonShape[strPolygonName] = pPolygon;
        pBody->CreateFixture(m_mapPolygonShape[strPolygonName],30.f);
      }
      else{
        pBody->CreateFixture(Polygon->second,30.f);
      }
      pBody->SetTransform(b2Vec2(fX_M,fY_M),0.f); 
      m_mapBodies[pBody] = std::tuple<int,float,float>(
          iTileIdx,fTileSizeWidth_M,fTileSizeHeight_M);
    }
    else if (strPhysicType == "Ground") {
      //printf("\033[1;33m[%s][%d] :x: Ground \033[m\n",__FUNCTION__,__LINE__);
      b2Body* pBody =m_pWorld->CreateBody(m_mapBodyDef["ground_tile"]);
      // :x: using edge shape for tile structure
      pBody->CreateFixture(m_mapEdgeshape["1X1_Box_Up"],0.f);
      pBody->CreateFixture(m_mapEdgeshape["1X1_Box_Down"],0.f);
      pBody->CreateFixture(m_mapEdgeshape["1X1_Box_Left"],0.f);
      pBody->CreateFixture(m_mapEdgeshape["1X1_Box_Right"],0.f);

      pBody->SetTransform(b2Vec2(fX_M,fY_M),0.f); 
      m_mapBodies[pBody] = std::tuple<int,float,float>(iTileIdx,1.0f,1.0f);
    }

  }

  return pBody;
}

int CPhysic_World::Create_BodyDefs(std::map<std::string,b2BodyDef*> &mapBodyDef) {
  // Define body define
  b2BodyDef* pBodyDef;
  pBodyDef = new b2BodyDef();
  pBodyDef->type = b2_staticBody;
  mapBodyDef["ground_tile"] = pBodyDef;

  pBodyDef = new b2BodyDef();
  pBodyDef->type = b2_dynamicBody;
  mapBodyDef["dynamic_tile"] = pBodyDef;

  return 0;
}

int CPhysic_World::Create_EdgeShapes(
    std::map<std::string,b2EdgeShape*>&  mapEdgeshape) {
  // Define shapes
  // basic 1 m X 1 m box ; 18 pixel X 18 pixel
  b2EdgeShape* pBasic_1X1_Box_Up = new b2EdgeShape();
  b2EdgeShape* pBasic_1X1_Box_Down = new b2EdgeShape();
  b2EdgeShape* pBasic_1X1_Box_Left = new b2EdgeShape();
  b2EdgeShape* pBasic_1X1_Box_Right = new b2EdgeShape();
  b2Vec2 v0(-0.5f, 0.5f);
  b2Vec2 v1( 0.5f, 0.5f);
  b2Vec2 v2( 0.5f,-0.5f);
  b2Vec2 v3(-0.5f,-0.5f);

  pBasic_1X1_Box_Up->SetOneSided(v2,v1,v0,v3);
  pBasic_1X1_Box_Down->SetOneSided(v0,v3,v2,v1);
  pBasic_1X1_Box_Left->SetOneSided(v1,v0,v3,v2);
  pBasic_1X1_Box_Right->SetOneSided(v3,v2,v1,v0);
  mapEdgeshape["1X1_Box_Up"]    = pBasic_1X1_Box_Up;
  mapEdgeshape["1X1_Box_Down"]  = pBasic_1X1_Box_Down;
  mapEdgeshape["1X1_Box_Left"]  = pBasic_1X1_Box_Left;
  mapEdgeshape["1X1_Box_Right"] = pBasic_1X1_Box_Right;

  return 0;
}

int CPhysic_World::Create_PolygonShape(
                    std::map<std::string,b2PolygonShape*>& mapPolygonShape) {
  // Character 1.33 m X 1.33 m box ; 24 pixel X 24 pixel
  b2PolygonShape* pCharacter_Box = new b2PolygonShape();
  pCharacter_Box->SetAsBox(float(24.f/2.f/m_fScale_Pixel_per_Meter),
                           float(24.f/2.f/m_fScale_Pixel_per_Meter));
  mapPolygonShape["Character_Box"] = pCharacter_Box;
  return 0;
}
