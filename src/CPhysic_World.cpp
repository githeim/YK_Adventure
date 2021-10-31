#include "CPhysic_World.h"


int CPhysic_World::Create_World(TMX_Ctx &TMX_context) {
  b2Vec2 gravity(0.0f,-10.0f);
  m_pWorld = new b2World(gravity);

  // Define body define
  b2BodyDef* pBodyDef;
  pBodyDef = new b2BodyDef();
  pBodyDef->type = b2_staticBody;
  m_mapBodyDef["ground_tile"] = pBodyDef;

  pBodyDef = new b2BodyDef();
  pBodyDef->type = b2_dynamicBody;
  m_mapBodyDef["dynamic_tile"] = pBodyDef;

  // Define shapes
  // basic 1 m X 1 m box ; 18 pixel X 18 pixel
  b2PolygonShape* pBasic_1X1_Box = new b2PolygonShape();
  pBasic_1X1_Box->SetAsBox(1.0f/2.f, 1.0f/2.f);
  m_mapPolygonShape["1X1_Box"] = pBasic_1X1_Box;

  // Character 1.33 m X 1.33 m box ; 24 pixel X 24 pixel
  b2PolygonShape* pCharacter_Box = new b2PolygonShape();
  pCharacter_Box->SetAsBox(float(24.f/2.f/m_fScale_Pixel_per_Meter),
                           float(24.f/2.f/m_fScale_Pixel_per_Meter));
  m_mapPolygonShape["Character_Box"] = pCharacter_Box;

  float fLeftEdge_M;
  float fTopEdge_M;
  int iIdxCnt=0;
#if 0 // :x: for test
  float fTileSizeWidth_M  =1.0f;
  float fTileSizeHeight_M =1.0f;
  // Create Ground layer
  MapLayer &GroundLayer = TMX_context.mapLayers["Ground_Layer"];
  fLeftEdge_M = - ( GroundLayer.iWidth/2.0f )* 1.0f; ;
  fTopEdge_M  =   ( GroundLayer.iHeight/2.0f);
  iIdxCnt = 0;
  for ( auto iIdx : GroundLayer.vecTileIdx) {
    // X,Y coordination in IDX
    int iX_idx= iIdxCnt % GroundLayer.iWidth;
    int iY_idx= iIdxCnt / GroundLayer.iWidth;
    // X,Y coordination in meter
    float fX_M = fLeftEdge_M+ iX_idx* fTileSizeWidth_M  + fTileSizeWidth_M /2.f;
    float fY_M = fTopEdge_M - iY_idx* fTileSizeHeight_M - fTileSizeHeight_M/2.f;
    if (iIdx !=0) {
      //printf("\033[1;33m[%s][%d] :x: iIdx[%d] %d (%d %d)(%f M %f M)\033[m\n",
      //    __FUNCTION__,__LINE__,iIdxCnt,iIdx,iX_idx,iY_idx,fX_M,fY_M);
      std::string strPhysicType;
      if ( !CTMX_Reader::GetPhysicType(TMX_context,iIdx,strPhysicType)) {
        if (strPhysicType == "Ground") {
          //printf("\033[1;33m[%s][%d] :x: Ground \033[m\n",__FUNCTION__,__LINE__);
          b2Body* pBody =m_pWorld->CreateBody(m_mapBodyDef["ground_tile"]);
          pBody->CreateFixture(m_mapPolygonShape["1X1_Box"],0.f);
          pBody->SetTransform(b2Vec2(fX_M,fY_M),0.f); 

          m_mapBodies[pBody] = std::tuple<int,float,float>(iIdx,1.0f,1.0f);
        }
      }
    }
    iIdxCnt++;
  }

  // Create Character Layer

  MapLayer &CharacterLayer = TMX_context.mapLayers["Character_Layer"];
  fLeftEdge_M = - ( CharacterLayer.iWidth/2.0f )* 1.0f; ;
  fTopEdge_M  =   ( CharacterLayer.iHeight/2.0f);
  iIdxCnt = 0;
                                                                                 
  float fCharacterSizeWidth_M=1.33f;
  float fCharacterSizeHeight_M=1.33f;
  for ( auto iIdx : CharacterLayer.vecTileIdx) {
    // X,Y coordination in IDX
    int iX_idx= iIdxCnt % GroundLayer.iWidth;
    int iY_idx= iIdxCnt / GroundLayer.iWidth;
    // X,Y coordination in meter
    float fX_M = fLeftEdge_M+ iX_idx* fTileSizeWidth_M  + fTileSizeWidth_M /2.f;
    // 캐릭터 사이즈는 1.33M, 그러나 좌표는 1M 기준으로 설정한다
    // 바닥 위치를 일치시켜 배치한다
    float fY_M = fTopEdge_M - iY_idx* fTileSizeHeight_M - fTileSizeHeight_M/2.f
                 + ((fCharacterSizeHeight_M /2)-(fTileSizeHeight_M/2));
    if (iIdx !=0) {
      //printf("\033[1;33m[%s][%d] :x: char iIdx[%d] %d (%d %d)(%f M %f M)\033[m\n",
      //    __FUNCTION__,__LINE__,iIdxCnt,iIdx,iX_idx,iY_idx,fX_M,fY_M);

      std::string strPhysicType;
      if ( !CTMX_Reader::GetPhysicType(TMX_context,iIdx,strPhysicType)) {
        if (strPhysicType == "Dynamic") {
          //printf("\033[1;33m[%s][%d] :x: Dynamic \033[m\n",__FUNCTION__,__LINE__);
          printf("\033[1;33m[%s][%d] :x: chk %f %f\033[m\n",__FUNCTION__,__LINE__,fX_M,fY_M);
          printf("\033[1;32m[%s][%d] :x: chk %d %d\033[m\n",__FUNCTION__,__LINE__,iX_idx,iY_idx);

          b2Body* pBody =m_pWorld->CreateBody(m_mapBodyDef["dynamic_tile"]);
          pBody->CreateFixture(m_mapPolygonShape["Character_Box"],30.f);
          pBody->SetTransform(b2Vec2(fX_M,fY_M),0.f); 
          m_mapBodies[pBody] = std::tuple<int,float,float>(
              iIdx,fCharacterSizeWidth_M,fCharacterSizeHeight_M);
        }
      }

    }
    iIdxCnt++;
  }
#else
  // 기본 타일 사이즈 (18 pixel = 1M)
  
  // 기본 타일 사이즈 pixel
  float fBaseTileWidth = (float)TMX_context.iTileWidth;
  float fBaseTileHeight =(float)TMX_context.iTileHeight;
  // 기본 타일 사이즈 meter
  float fBaseTileWidth_M = (float)TMX_context.iTileWidth/(float)TMX_context.iTileWidth;
  float fBaseTileHeight_M =(float)TMX_context.iTileHeight/(float)TMX_context.iTileHeight;

  // 레이어 별로 아이템을 읽어서 body를 생성한다
  for (auto item : TMX_context.mapLayers) {
    printf("\033[1;33m[%s][%d] :x: chk %s \033[m\n",__FUNCTION__,__LINE__,item.first.c_str());
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
        std::string strPhysicType;
        if ( !CTMX_Reader::GetPhysicType(TMX_context,iIdx,strPhysicType)) {
          if (strPhysicType == "Dynamic") {

            std::tuple<int,int,int,int> Tile_XYWH_size;
            if (!CTMX_Reader::GetSpecifiedTileSize(TMX_context,iIdx,Tile_XYWH_size)) {
              printf("\033[1;33m[%s][%d] :x: chk %d %d %d %d\033[m\n",
                  __FUNCTION__,__LINE__,
                  std::get<0>(Tile_XYWH_size),
                  std::get<1>(Tile_XYWH_size),
                  std::get<2>(Tile_XYWH_size),
                  std::get<3>(Tile_XYWH_size)
                  );
              fTileSizeWidth_M  = (float)std::get<2>(Tile_XYWH_size)/fBaseTileWidth;
              fTileSizeHeight_M = (float)std::get<3>(Tile_XYWH_size)/fBaseTileHeight;
              // 좌표는 기본 타일 사이즈를 기준으로 계산한다
              fX_M = fLeftEdge_M+ iX_idx* fBaseTileWidth_M  + fBaseTileWidth_M /2.f;
              // 해당 타일이 기본 타일보다 커도 바닥 위치를 일치시켜 배치한다
              fY_M = fTopEdge_M - iY_idx* fBaseTileHeight_M - fBaseTileHeight_M/2.f
                + ((fTileSizeHeight_M /2)-(fTileSizeHeight_M/2));
            }

            b2Body* pBody =m_pWorld->CreateBody(m_mapBodyDef["dynamic_tile"]);
            std::string strTag;
            if (!CTMX_Reader::GetTag(TMX_context, iIdx, strTag)) {
              printf("\033[1;32m[%s][%d] :x: Tag %s \033[m\n",__FUNCTION__,__LINE__,strTag.c_str());
              m_mapTags[strTag].push_back( pBody); 
            }

            pBody->CreateFixture(m_mapPolygonShape["Character_Box"],30.f);
            pBody->SetTransform(b2Vec2(fX_M,fY_M),0.f); 
            m_mapBodies[pBody] = std::tuple<int,float,float>(
                iIdx,fTileSizeWidth_M,fTileSizeHeight_M);
          }
          else if (strPhysicType == "Ground") {
            //printf("\033[1;33m[%s][%d] :x: Ground \033[m\n",__FUNCTION__,__LINE__);
            b2Body* pBody =m_pWorld->CreateBody(m_mapBodyDef["ground_tile"]);
            pBody->CreateFixture(m_mapPolygonShape["1X1_Box"],0.f);
            pBody->SetTransform(b2Vec2(fX_M,fY_M),0.f); 
            m_mapBodies[pBody] = std::tuple<int,float,float>(iIdx,1.0f,1.0f);
          }
          
        }
      }
      iIdxCnt++;
    } 

  }
#endif // :x: for test

  return 0;
}

int CPhysic_World::Destroy_World() {
  for (auto BodyDef : m_mapBodyDef) {
    delete BodyDef.second;
  }
  
  for (auto PolygonShape : m_mapPolygonShape) {
    delete PolygonShape.second;
  }

  for (auto body : m_mapBodies) {
    m_pWorld->DestroyBody(body.first);
  }
  delete m_pWorld;
  return 0;
}

void CPhysic_World::SpinWorld(double dbTimeStep, 
    int iVelocityIter, int iPositionIter ) {
  m_pWorld->Step(dbTimeStep, iVelocityIter, iPositionIter);
  return;
}
