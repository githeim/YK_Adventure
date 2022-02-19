#include"Plugins.h"
#include "CPlugin.h"
#include <memory>
#include <math.h>

#define AUTO_AIMING  (0)
#define MOUSE_AIMING (1)



extern std::shared_ptr<CApp> Get_pApp() ;
extern std::shared_ptr<TMX_Ctx> Get_pTMX_Ctx() ;

static b2Vec2 AngleToVector(float &fAngle) {
  return b2Vec2(cos(fAngle*M_PI/180),sin(fAngle*M_PI/180));
}
float VectorToAngle(b2Vec2 &vec2Input) {
  return (float)(atan2(vec2Input.y,vec2Input.x)*180/M_PI);
}
static float GetDistance(float fX1,float fY1, float fX2, float fY2) {
  // Calculating distance
  return std::sqrt(pow(fX2 - fX1, 2) +
      pow(fY2 - fY1, 2) * 1.0);
};


static b2Vec2 PixelToCoord_M(int iPixel_X, int iPixel_Y,float fScale_Pixel_per_Meter,int iScreenWidth =SCREEN_WIDTH ) {
  float fX_M =  (iPixel_X-(float)(SCREEN_WIDTH/2.f)) / fScale_Pixel_per_Meter;
  float fY_M = -(iPixel_Y -(SCREEN_HEIGHT/2))        / fScale_Pixel_per_Meter;
  return b2Vec2(fX_M,fY_M);
}

void Print_Bodies(b2Body* pBody) {
  for (b2Body* b = pBody; b; b = b->GetNext())
  {
    printf("\033[1;33m[%s][%d] :x: 0x%p -> 0x%p \033[m\n",__FUNCTION__,__LINE__,b,b->GetNext());
  }

}
/**
 * @brief Find the bodies which has the target tags and is in the range
 *
 * @param vecTargetTags[IN] target tag to find
 * @param fRange_M[IN]
 * @param vec2CurPos_M[IN]    current position
 * @param vecOutputBodies[OUT] found bodies
 * @param ObjDirectory[IN]
 *
 * @return 
 */

int Find_Bodies(std::vector<std::string> &vecTargetTags, float fRange_M,
                b2Vec2 &vec2CurPos_M,
                std::vector<b2Body*> &vecOutputBodies,
                CObjDirectory &ObjDirectory) {
  std::map<std::string,  std::vector<ObjAttr_t*>> &mapTagObj = 
                                                       ObjDirectory.m_mapTagObj;
  for ( auto TagItem : mapTagObj) {
    const std::string &strTag = TagItem.first;
    std::vector<ObjAttr_t*> &vecObj = TagItem.second;
    // find the bodies with same tags in TargetTags
    if (std::find( std::begin(vecTargetTags),std::end(vecTargetTags),strTag ) 
         != std::end(vecTargetTags)) {
      for (auto pObj : vecObj) {
        // Find the bodies in the target range
        b2Vec2 vecPos = pObj->pBody->GetPosition();
        if (  
            GetDistance(
              vecPos.x,vecPos.y,vec2CurPos_M.x,vec2CurPos_M.y) 
            < fRange_M)
        {
          vecOutputBodies.push_back(pObj->pBody);
        }
      }
    }
  }
  return 0;
}


/**
 * @brief Change Target Index
 *
 * @param iSizeofTargets[IN]
 * @param iCurTarget[OUT]
 * @param bDirection[IN] change direction , if true takes next target ,
 *                       false it takes previous target
 *
 * @return 
 */
int ChangeTargetIdx(int &iSizeofTargets,int &iCurTarget,bool bDirection=true) {
  if (iSizeofTargets == 0) {
    iCurTarget = -1;
    return 0;
  }
  if (iSizeofTargets == 1) {
    iCurTarget = 0;
    return 0;
  }
  if (bDirection == true) {
    iCurTarget++;
  }
  else {
    iCurTarget--;
  }
  if (iCurTarget > iSizeofTargets -1) {
    iCurTarget = 0;
  }
  if (iCurTarget < 0) {
    iCurTarget = iSizeofTargets -1;
  }
  return 0;
}

int Get_AimingVectors (std::vector<b2Body*> &vecTargetBodies,
                std::vector<b2Vec2> &vecAimings,b2Vec2 &vec2CurPos ) {
  vecAimings.clear();
  if (vecTargetBodies.size() ==0 ) 
    return -1;

  for(b2Body* pBody : vecTargetBodies) {
    b2Vec2 vec2TargetPos = pBody->GetPosition();
    b2Vec2 vec2Aiming = vec2TargetPos - vec2CurPos;
    vec2Aiming.Normalize();
    vecAimings.push_back(vec2Aiming);
  }
  
  return 0;
}




/**
 * @brief check touching ground,
 *        comparing the y , if the diff is smaller than material's size, it is 
 *        touching the ground
 *
 * @param pWorldManifold[IN] the set manifolds
 * @param pPos[IN] material's position
 * @param iPointCount[IN]    the number of contact points
 * @param fSize_M[IN] material's size
 *
 * @return 
 */
bool IsTouchingGround(b2WorldManifold* pWorldManifold,b2Vec2* pPos,int iPointCount, 
                      float fSize_M=1.33f){
  bool bRet=false;
  if (!iPointCount) {
    return false;
  }

  for (int i = 0 ; i < iPointCount; i++) {
    // 접점이 물체의 위치보다 낮은곳에 있으면 ground로 판정
    if (  pPos->y > pWorldManifold->points[i].y ) {
      bRet = true;
      return bRet;
    }
  }
  
  return bRet;
}

bool IsTouchingGround(b2Body* pBody) {
  bool bRet = false;
  b2ContactEdge* pContacts = pBody->GetContactList();
  b2WorldManifold worldManifold;

  if (pContacts) {
    int iPointCount = pContacts->contact->GetManifold()->pointCount;
    pContacts->contact->GetWorldManifold(&worldManifold);
    b2Vec2 vecPos; // player position
    vecPos = pContacts->contact->GetFixtureB()->GetBody()->GetPosition();

    bRet =IsTouchingGround(&worldManifold,&vecPos,iPointCount); 
  }
  else
    bRet = false;
  return bRet;
}

/**
 * @brief Check the body's tag, if the body has same tag to strTargetTag 
 *        it returns true
 * @param ObjDirectory[IN]
 * @param vecTargetTags[IN] Target tags to find
 * @param pBody[IN] Contacted body
 * @param pContactedObj[OUT]
 *
 * @return 
 */
bool Chk_Tag(CObjDirectory &ObjDirectory,
    std::vector<std::string> vecTargetTags,b2Body* pBody,
    ObjAttr_t* &pContactedObj
    )
{
  std::map<std::string,std::vector<ObjAttr_t*>> & mapTagObj = 
                                                       ObjDirectory.m_mapTagObj;
  for ( auto strTargetTag : vecTargetTags) {
    if (mapTagObj.find(strTargetTag) == mapTagObj.end()) {
      continue;
    }
    for ( auto pItem : mapTagObj[strTargetTag]) {
      if ( pBody == pItem->pBody) {
        //printf("\033[1;33m[%s][%d] :x: Tag : %s \033[m\n",
        //    __FUNCTION__,__LINE__,strTargetTag.c_str());
        pContactedObj = pItem;
        return true;
      }
    }
  }
  return false;
}
int Plug_Bullet_Init(CPhysic_World* &pWorld,             
            CObjDirectory &ObjDirectory,
            SDL_Event* &pEvt,double& dbTimeDiff,
            CPlugin* pInstance) {
  pInstance->m_Float_Common["LifeTime"] =0;

  return 0;
}


int Plug_Bullet(CPhysic_World* &pWorld,             
            CObjDirectory &ObjDirectory,
            SDL_Event* &pEvt,double& dbTimeDiff,
            CPlugin* pInstance) {
  b2Body* pBody = pInstance->m_pBody;
  pInstance->m_Float_Common["LifeTime"]+=dbTimeDiff;
  // Damage point of the bullet
  float fBulletDamage= 10.f;
  // Target to Attack
  std::vector<std::string> vecTargetTags ={
    "Enemy_Flyer",
    "Enemy_Ground_Tracker",
  };

  std::shared_ptr<CApp> pApp =Get_pApp();
  if (pInstance->m_Float_Common["LifeTime"] > 10) {
    printf("\033[1;31m[%s][%d] :x: Die Bullet \033[m\n",__FUNCTION__,__LINE__);
    pApp->Set_setObjToRemove(ObjDirectory.m_mapBody_Obj[pBody]);
    return 0;
  }

  b2ContactEdge* pContacts = pBody->GetContactList();
  if (pContacts) {

    do {
      // Fixture A's body ; the body that is applying force
      b2Body* pBodyA = pContacts->contact->GetFixtureA()->GetBody();
      // Fixture B's body ; the body that is received force 
      b2Body* pBodyB = pContacts->contact->GetFixtureB()->GetBody();

      ObjAttr_t* pContactedObj = nullptr;
      // Check the tags
      if (Chk_Tag(ObjDirectory,vecTargetTags,pBodyA,pContactedObj) || 
          Chk_Tag(ObjDirectory,vecTargetTags,pBodyB,pContactedObj) ) {
        // Destroy Obj
        printf("\033[1;33m[%s][%d] :x: COntacted %s \033[m\n",
            __FUNCTION__,__LINE__,pContactedObj->strObjName.c_str());
        std::shared_ptr<CApp> pApp =Get_pApp();
        // Attack contacted body
        pContactedObj->pPlugin->m_Float_Common["Damage"]+=33;
        // Remove Self body
        pApp->Set_setObjToRemove(ObjDirectory.m_mapBody_Obj[pBody]);
      }
      pContacts = pContacts->next;
    } while (pContacts);
  }



  return 0;
}

int Spawn_Bullet(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory,
                 b2Vec2 &vec2CurPos,b2Vec2 &vec2Direction) {
  int iBulletTileIdx;
  // 209 : slug bullet
  int iIdxSlugBullet = 209;
  // 208 : piercing bullet
  int iIdxPiercingBullet = 208;
  iBulletTileIdx= iIdxSlugBullet;
  //iBulletTileIdx= iIdxPiercingBullet;
  int fBulletBaseAngle = -90.f;
  std::shared_ptr<CApp> pApp =Get_pApp();
  std::shared_ptr<TMX_Ctx> pTMX_Ctx =  Get_pTMX_Ctx();
  std::map<std::string,ObjAttr_t*> &mapObjs = ObjDirectory.m_mapObjs;

  std::string strSpawnedObjName;
  float fSpawnDistance_M=1.9f;
  b2Vec2 vec2SpawnPosition = vec2Direction;
  vec2SpawnPosition.Normalize();
  vec2SpawnPosition*=fSpawnDistance_M;
  vec2SpawnPosition+=vec2CurPos;

  b2Body* pSpawnedBody = nullptr;

  strSpawnedObjName = pWorld->Create_Element(
      *(pTMX_Ctx.get()),iBulletTileIdx,vec2SpawnPosition.x,vec2SpawnPosition.y, ObjDirectory);

  if (strSpawnedObjName == "Error") {
    return -1;
  }

  pSpawnedBody = mapObjs[strSpawnedObjName]->pBody; 
  if (pSpawnedBody == pSpawnedBody->GetNext()) {
    printf("\033[1;31m[%s][%d] :x: Error Case!!!! %p \033[m\n",__FUNCTION__,__LINE__,pSpawnedBody);
  }

  pSpawnedBody->SetTransform(vec2SpawnPosition,
                             -(VectorToAngle(vec2Direction)+fBulletBaseAngle));
  //pSpawnedBody->SetBullet(true);
  pSpawnedBody->ApplyLinearImpulseToCenter(vec2Direction, true);

  auto pInstance = new CPlugin();
  pInstance->m_pBody = pSpawnedBody;
  pInstance->OnInit = Plug_Bullet_Init;
  pInstance->OnDeInit = nullptr;
  pInstance->OnExecute = Plug_Bullet;

  mapObjs[strSpawnedObjName]->pPlugin = pInstance;
  mapObjs[strSpawnedObjName]->vecTag.clear();
  pApp->Set_vecObjToAdd(mapObjs[strSpawnedObjName]);

  ObjDirectory.UpdateDirectory();

  return 0;
}

int Spawn_Bullet(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory,
                 b2Vec2 &vec2CurPos,b2Vec2& vec2Direction, float fImpulse) {
  vec2Direction*=fImpulse;
  return Spawn_Bullet(pWorld,ObjDirectory,vec2CurPos,vec2Direction);
}



int Spawn_Bullet(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory,
                 b2Vec2 &vec2CurPos,float fAngle,float fImpulse) {
  b2Vec2 vecDirection = AngleToVector(fAngle);
  return Spawn_Bullet(pWorld,ObjDirectory,vec2CurPos,vecDirection,fImpulse);
}

int Plug_Player01_Init(CPhysic_World* &pWorld,
                        CObjDirectory &ObjDirectory,
                        SDL_Event* &pEvt,double& dbTimeDiff,
                        CPlugin* pInstance) {

  std::map<std::string, float> & Float_Common = pInstance->m_Float_Common;
  pInstance->m_Int_Common["AimingState"] = MOUSE_AIMING;
  pInstance->m_Int_Common["AimingTarget"] = 0;
  Float_Common["Energy"]= 100;
  Float_Common["Damage"]= 0;

  return 0;
}




int Plug_Player01(CPhysic_World* &pWorld,
                  CObjDirectory &ObjDirectory,
                  SDL_Event* &pEvt,double& dbTimeDiff,
                  CPlugin* pInstance) {
  // Find Player 1 Body, it should be one
  b2Body* pBody = pInstance->m_pBody;
  b2Vec2 vec2CurPos_M=pBody->GetPosition();
  b2ContactEdge* pContacts = pBody->GetContactList();

  float fMass = pBody->GetMass();
  float fJumpImpulse = fMass * 7.0;
  float fMoveSpeed = 6.0;

  static  bool bPrevIsGround = false;
  bool bIsGround=false; 
  bool bIsFly=false; 
  b2WorldManifold worldManifold;
  auto vecVelocity = pBody->GetLinearVelocity();

  // Target to Attack
  std::vector<std::string> vecTargetTags ={
    "Enemy_Flyer",
    "Enemy_Ground_Tracker",
  };

  if (pContacts) {
    do {
      // Fixture A's body ; the body that is applying force
      b2Body* pBodyA = pContacts->contact->GetFixtureA()->GetBody();
      // Fixture B's body ; the body that is received force 
      b2Body* pBodyB = pContacts->contact->GetFixtureB()->GetBody();
      
      // Draw A & B position points
      b2Vec2 vecPosA; // contacted object
      vecPosA = pBodyA->GetPosition();
      //Dbg_DrawPoint_Scale(vecPosA.x,vecPosA.y);
      b2Vec2 vecPosB; // player
      vecPosB = pBodyB->GetPosition();
      Dbg_DrawPoint_Scale(vecPosB.x,vecPosB.y);

      int iPointCount = pContacts->contact->GetManifold()->pointCount;

      pContacts->contact->GetWorldManifold(&worldManifold);
      for (int i = 0 ; i < iPointCount; i++) {
        Dbg_DrawPoint_Scale(worldManifold.points[i].x,worldManifold.points[i].y);
      }
      if ( iPointCount != 0)  {
        if (IsTouchingGround(&worldManifold,&vecPosB,iPointCount) == true)
          bIsGround = true;
      }
      
      ObjAttr_t* pContactedObj = nullptr;
      // Check the tags
      if (Chk_Tag(ObjDirectory,vecTargetTags,pBodyA,pContactedObj) || 
          Chk_Tag(ObjDirectory,vecTargetTags,pBodyB,pContactedObj) ) {
        // Destroy Obj
        std::shared_ptr<CApp> pApp =Get_pApp();
        //pApp->Set_setObjToRemove(pContactedObj);
      }
      pContacts = pContacts->next;
    } while (pContacts);
  }
  else
  {
    bIsGround = false;
  }

  // Set Friction , not to slip
  if (bIsGround == true && bIsGround != bPrevIsGround ) {
    pBody->SetLinearVelocity(b2Vec2( 0,vecVelocity.y));
    //pBody->GetFixtureList()->SetFriction(1000.0f);
  }

  bPrevIsGround = bIsGround;

  // 기울어지지 않도록 0도로 고정
  pBody->SetTransform(pBody->GetPosition(),0);

  int iNumkeys;
  const unsigned char *pState=SDL_GetKeyboardState(&iNumkeys);
  
  // :x: same input timing margin is 0.1sec 
  static double dbInputTimeDiff =0;
  static SDL_Keycode Prev_Key_Input;
  dbInputTimeDiff +=dbTimeDiff;
  

  b2Vec2 vec2BulletDirection;

  // Auto Aiming
  std::vector<b2Vec2> vecAutoAimings;
  bool bAutoAiming = false;
  b2Vec2 vec2AutoAiming;
  // Target Search Range (Meter)
  float fRange_M = 35.f;

  std::vector<b2Body*> vecTargetBodies;
  Find_Bodies(vecTargetTags,fRange_M,vec2CurPos_M,vecTargetBodies,ObjDirectory);
  int iTargetBodySize = vecTargetBodies.size();

  if (iTargetBodySize > 0  ) {
    if (pInstance->m_Int_Common["AimingTarget"] > iTargetBodySize-1 ){
      // 사라진 예전 target을 가리킬 경우 0으로 가리킨다
      // if the target index pointed non-existing target it changes to 0
      pInstance->m_Int_Common["AimingTarget"] =0;
    }
    int iTargetIdx = pInstance->m_Int_Common["AimingTarget"];
    b2Body* &pTargetBody = vecTargetBodies[iTargetIdx];
    b2Vec2 vec2TargetPos = pTargetBody->GetPosition();
    SDL_Color Color =COLOR_BLUE;
    Dbg_DrawLine_Scale(
        pBody->GetPosition().x, 
        pBody->GetPosition().y, 
        vec2TargetPos.x, vec2TargetPos.y,Color);
    Get_AimingVectors(vecTargetBodies, vecAutoAimings, vec2CurPos_M);
    bAutoAiming = true;
    vec2AutoAiming = vecAutoAimings[pInstance->m_Int_Common["AimingTarget"]];
  } else {
    bAutoAiming = false;
  }


  // Mouse aiming
  b2Vec2 vec2MouseAiming;
  int x, y;
  SDL_GetMouseState(&x, &y);
  float fScale_Pixel_per_Meter = pWorld->m_fScale_Pixel_per_Meter;
  b2Vec2 vec2Cursor_M = PixelToCoord_M(x,y,fScale_Pixel_per_Meter);
  vec2MouseAiming =   vec2Cursor_M-vec2CurPos_M;
  vec2MouseAiming.Normalize();

  if (pInstance->m_Int_Common["AimingState"] == AUTO_AIMING) {
    if (bAutoAiming == true) {
      vec2BulletDirection = vec2AutoAiming;
    } else
    {
      vec2BulletDirection = vec2MouseAiming;
    }
  } else if (pInstance->m_Int_Common["AimingState"] == MOUSE_AIMING) {
    vec2BulletDirection = vec2MouseAiming;
  }

  

  b2Vec2 vec2BulletRecoil = vec2BulletDirection;// 반동 - recoil
  float fImpulse = 200.f;
  vec2BulletRecoil*=-(fImpulse*0.3f);// recoil
  if (pEvt) {
    // Input Event
    if( pEvt->type == SDL_KEYDOWN )
    {
      if (
          Prev_Key_Input != pEvt->key.keysym.sym ||
          dbInputTimeDiff > 0.1
          ) 
      {
        dbInputTimeDiff = 0;
        switch( pEvt->key.keysym.sym ) {
          case SDLK_a:
            pBody->SetLinearVelocity(b2Vec2( -fMoveSpeed,vecVelocity.y));
            break;
          case SDLK_d:
            pBody->SetLinearVelocity(b2Vec2(  fMoveSpeed,vecVelocity.y));
            break;
          case SDLK_w:
            if (bIsGround || bIsFly ) 
            {
              if (pState[SDL_SCANCODE_A]){
                pBody->ApplyForceToCenter(b2Vec2(-4000,0), true);
              }
              if (pState[SDL_SCANCODE_D]){
                pBody->ApplyForceToCenter(b2Vec2( 4000,0), true);
              }
              pBody->ApplyLinearImpulseToCenter(b2Vec2(0,fJumpImpulse), true);
            }
            break;
          case SDLK_s:
            pBody->ApplyForceToCenter(b2Vec2( 0,-2000), true);
            break;
          case SDLK_e:
            //pBody->ApplyForceToCenter(b2Vec2( 0,-2000), true);
            pBody->ApplyAngularImpulse(30, true);
            printf("\033[1;36m[%s][%d] :x: change \033[m\n",__FUNCTION__,__LINE__);
            break;
          case SDLK_f:
            printf("\033[1;33m[%s][%d] :x: Fire \033[m\n",__FUNCTION__,__LINE__);

            {
              if (!Spawn_Bullet(pWorld,ObjDirectory,vec2CurPos_M,vec2BulletDirection,fImpulse)) {
              // Recoil
              pBody->ApplyLinearImpulseToCenter(vec2BulletRecoil, true);
              }
            }


            break;
          case SDLK_r:
            // toggle
            if (pInstance->m_Int_Common["AimingState"] == AUTO_AIMING ) {
              pInstance->m_Int_Common["AimingState"] = MOUSE_AIMING;
            } else if (pInstance->m_Int_Common["AimingState"] == MOUSE_AIMING ) {
              pInstance->m_Int_Common["AimingState"] = AUTO_AIMING;
            }

            break;
          case SDLK_c:
            ChangeTargetIdx(iTargetBodySize,pInstance->m_Int_Common["AimingTarget"],true);
            // Toggle target
            // free drop
            //pBody->GetFixtureList()->SetSensor(true);
            //Print_Bodies(pWorld->m_pWorld->GetBodyList());
            break;
          case SDLK_v:
            ChangeTargetIdx(iTargetBodySize,pInstance->m_Int_Common["AimingTarget"],false);
            break;



        }
      }
      
      Prev_Key_Input = pEvt->key.keysym.sym;
    }
    if( pEvt->type == SDL_KEYUP ) {
      switch( pEvt->key.keysym.sym ) {
        case SDLK_w:
          break;
        case SDLK_a:
          if (bIsGround)
            pBody->SetLinearVelocity(b2Vec2( 0,vecVelocity.y));
          break;
        case SDLK_d:
          if (bIsGround)
            pBody->SetLinearVelocity(b2Vec2( 0,vecVelocity.y));
          break;
      }

    }
  }

  return 0;

}

/**
 * @brief Find the bodies which has the target tags and is in the range
 *
 * @param vecTargetTags[IN] target tag to find
 * @param fRange_M[IN]
 * @param Position_M[IN]    current position
 * @param vecOutputBodies[OUT] found bodies
 * @param pWorld[IN]
 *
 * @return 
 */
int Find_Bodies(std::vector<std::string> &vecTargetTags, float fRange_M,
                std::tuple<float,float> &Position_M,
                std::vector<b2Body*> &vecOutputBodies,
                CPhysic_World* &pWorld)
{
  std::vector<b2Body*> vecBodies;
  std::map<std::string,std::vector<b2Body*>> &mapTags = pWorld->m_mapTags;

  for ( auto TagItem : mapTags) {
    const std::string &strTag = TagItem.first;
    std::vector<b2Body*> &vecBodies = TagItem.second;
    // find the bodies with same tags in TargetTags
    if (std::find( std::begin(vecTargetTags),std::end(vecTargetTags),strTag ) 
         != std::end(vecTargetTags)) {
      for (auto pBody : vecBodies) {
        // Find the bodies in the target range
        b2Vec2 vecPos = pBody->GetPosition();
        if (  
            GetDistance(
              vecPos.x,vecPos.y,std::get<0>(Position_M),std::get<1>(Position_M)) 
            < fRange_M)
        {
          vecOutputBodies.push_back(pBody);
        }
      }
    }
  }
  return 0;
}

int Plug_Enemy_Flyer_Init(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, 
                              SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance) {
  printf("\033[1;33m[%s][%d] :x: Init Enemy_Flyer \033[m\n",
      __FUNCTION__,__LINE__);
  std::map<std::string, float> & Float_Common = pInstance->m_Float_Common;

  // Initiation
  Float_Common["InputTimeDiff"]= 0.f;
  Float_Common["ReflectionTimeDiff"]= 0.f;

  Float_Common["Energy"]= 100;
  Float_Common["Damage"]= 0;
  return 0;
}


int Plug_Enemy_Flyer_DeInit(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, 
                              SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance) {
  printf("\033[1;33m[%s][%d] :x: DeInit Enemy_Flyer \033[m\n",
      __FUNCTION__,__LINE__);
 
  return 0;
}


int Plug_Enemy_Flyer(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, 
                       SDL_Event* &pEvt,
                       double& dbTimeDiff, CPlugin* pInstance) {
  std::map<std::string, float> & Float_Common = pInstance->m_Float_Common;
  std::map<std::string, std::string> &Str_Common =pInstance->m_Str_Common;
  // character control rate --> 0.5 sec
  float fCtrlRate_SEC = 0.5;
  // character reaction rate --> 0.8 sec
  float fReactionRate_SEC = 0.3;
  // Target Search Range (Meter)
  float fRange_M = 35.f;
  float fMoveSpeed = 3.0;
  float fMoveSpeed_X = 0;
  float fMoveSpeed_Y = 0;

  b2Body* pBody = pInstance->m_pBody;

  // Keep Flying ; set gravity 0
  pBody->SetGravityScale(0);
  // fix the angle 
  pBody->SetTransform(pBody->GetPosition(),0);

  Float_Common["InputTimeDiff"] +=dbTimeDiff;
  if (Float_Common["InputTimeDiff"] > fCtrlRate_SEC ) 
  {
    Float_Common["InputTimeDiff"] = 0;

    if (Str_Common["Direction_X"] == "Minus") {
      fMoveSpeed_X -= fMoveSpeed;
    }
    else if (Str_Common["Direction_X"] == "Plus") {
      fMoveSpeed_X += fMoveSpeed;
    } else {
      fMoveSpeed_X = 0;
    }

    if (Str_Common["Direction_Y"] == "Minus") {
      fMoveSpeed_Y -= fMoveSpeed;
    }
    else if (Str_Common["Direction_Y"] == "Plus") {
      fMoveSpeed_Y += fMoveSpeed;
    } else {
      fMoveSpeed_Y = 0;
    }
    pBody->SetLinearVelocity(b2Vec2( fMoveSpeed_X,fMoveSpeed_Y));
    
  }
  Float_Common["ReflectionTimeDiff"] +=dbTimeDiff;
  if (Float_Common["ReflectionTimeDiff"] > fReactionRate_SEC ) {
    Float_Common["ReflectionTimeDiff"] = 0;

    // Find Targets

    // Targets are players
    std::vector<std::string> vecTargetTags = { "Player01","Player02" };
    b2Vec2 vec2CurPos_M = pBody->GetPosition();
    std::vector<b2Body*> vecTargetBodies;
    Find_Bodies(vecTargetTags,fRange_M,vec2CurPos_M,vecTargetBodies,ObjDirectory);

    // set the direction to the first target 
    if (vecTargetBodies.size() > 0) {
      b2Body* &pTargetBody = vecTargetBodies[0];
      b2Vec2 vec2TargetPos = pTargetBody->GetPosition();
      SDL_Color Color =COLOR_RED;
      Dbg_DrawLine_Scale(
          pBody->GetPosition().x, 
          pBody->GetPosition().y, 
          vec2TargetPos.x, vec2TargetPos.y,Color);
      if ( vec2TargetPos.x < pBody->GetPosition().x ) {
        Str_Common["Direction_X"] = "Minus";
      } else 
      {
        Str_Common["Direction_X"] = "Plus";
      }
      if ( vec2TargetPos.y < pBody->GetPosition().y ) {
        Str_Common["Direction_Y"] = "Minus";
      } else 
      {
        Str_Common["Direction_Y"] = "Plus";
      }
    } else
    {
      Str_Common["Direction_X"] = "Stop";
      Str_Common["Direction_Y"] = "Stop";
    }
  }

  if ( Float_Common["Damage"] > 0) {
    Float_Common["Energy"] -= Float_Common["Damage"];
    Float_Common["Damage"]=0;
    return 0;
  }
  if ( Float_Common["Energy"] < 0) {
    std::shared_ptr<CApp> pApp =Get_pApp();
    pApp->Set_setObjToRemove(ObjDirectory.m_mapBody_Obj[pBody]);
    return 0;
  }

  return 0;
}

int Plug_Enemy_Ground_Tracker_Init(CPhysic_World* &pWorld,
                                  CObjDirectory &ObjDirectory, SDL_Event* &pEvt,
                                  double& dbTimeDiff, CPlugin* pInstance) {
  printf("\033[1;33m[%s][%d] :x: Init  \033[m\n", __FUNCTION__,__LINE__);
  std::map<std::string, float> & Float_Common = pInstance->m_Float_Common;
  std::map<std::string, std::string> &Str_Common =pInstance->m_Str_Common;

  // Initiation
  Float_Common["InputTimeDiff"]= 0.f;
  Float_Common["ReflectionTimeDiff"]= 0.f;
  Float_Common["Energy"]= 100;
  Float_Common["Damage"]= 0;

  // Initiate direction
  Str_Common["Direction_X"] = "Stop";
  Str_Common["Direction_Y"] = "Stop";
  return 0;
}

int Plug_Enemy_Ground_Tracker_DeInit(CPhysic_World* &pWorld,
                              CObjDirectory &ObjDirectory, SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance) {
  printf("\033[1;33m[%s][%d] :x: DeInit Enemy_Ground_Tracker\033[m\n",
      __FUNCTION__,__LINE__);

  return 0;
}


int Plug_Enemy_Ground_Tracker(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, 
                              SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance) {
  std::map<std::string, float> & Float_Common = pInstance->m_Float_Common;
  std::map<std::string, std::string> &Str_Common =pInstance->m_Str_Common;
  // character control rate --> 0.5 sec
  float fCtrlRate_SEC = 0.5;
  // character reaction rate --> 0.8 sec
  float fReactionRate_SEC = 0.8;
  // Target Search Range (Meter)
  float fRange_M = 15.f;


  b2Body* pBody = pInstance->m_pBody;

  float fMass = pBody->GetMass();
  float fJumpImpulse = fMass * 5.0;
  float fMoveSpeed = 2.0;
  float fMoveSpeed_X = 0;

  

  auto vecVelocity = pBody->GetLinearVelocity();
  // 기울어지지 않도록 0도로 고정
  pBody->SetTransform(pBody->GetPosition(),0);
  bool bIsGround = IsTouchingGround(pBody);

  Float_Common["InputTimeDiff"] +=dbTimeDiff;
  if (Float_Common["InputTimeDiff"] > fCtrlRate_SEC ) 
  {
    Float_Common["InputTimeDiff"] = 0;

    if (Str_Common["Direction_X"] == "Minus") {
      fMoveSpeed_X -= fMoveSpeed;
    }
    else if (Str_Common["Direction_X"] == "Plus") {
      fMoveSpeed_X += fMoveSpeed;
    }
    pBody->SetLinearVelocity(b2Vec2(  fMoveSpeed_X,vecVelocity.y));
    if (Str_Common["Direction_Y"] == "Plus") {
      if (bIsGround) 
        pBody->ApplyLinearImpulseToCenter(b2Vec2(  0,fJumpImpulse), true);
      Str_Common["Direction_Y"] = "Stop";
    }

  }


  Float_Common["ReflectionTimeDiff"] +=dbTimeDiff;
  if (Float_Common["ReflectionTimeDiff"] > fReactionRate_SEC ) {
    Float_Common["ReflectionTimeDiff"] = 0;

    // Find Targets

    // Targets are players
    std::vector<std::string> vecTargetTags = { "Player01","Player02" };
    std::tuple<float,float> Cur_Pos_M = 
                              { pBody->GetPosition().x,pBody->GetPosition().y };
    std::vector<b2Body*> vecTargetBodies;
    Find_Bodies(vecTargetTags,fRange_M,Cur_Pos_M,vecTargetBodies,pWorld);

    // set the direction to the first target 
    if (vecTargetBodies.size() > 0) {
      b2Body* &pTargetBody = vecTargetBodies[0];
      b2Vec2 vec2TargetPos = pTargetBody->GetPosition();
      SDL_Color Color =COLOR_BLUE;
      Dbg_DrawLine_Scale(
          pBody->GetPosition().x, 
          pBody->GetPosition().y, 
          vec2TargetPos.x, vec2TargetPos.y,Color);

      if ( vec2TargetPos.x < pBody->GetPosition().x ) {
        Str_Common["Direction_X"] = "Minus";
      } else 
      {
        Str_Common["Direction_X"] = "Plus";
      }

      if ( vec2TargetPos.y > pBody->GetPosition().y  
          ) {
        Str_Common["Direction_Y"] = "Plus";
      } 
    } 
    else {
      Str_Common["Direction_X"] = "Stop";
      Str_Common["Direction_Y"] = "Stop";
    }
  }

  if ( Float_Common["Damage"] > 0) {
    Float_Common["Energy"] -= Float_Common["Damage"];
    Float_Common["Damage"] =0;;
    return 0;
  }
  if ( Float_Common["Energy"] < 0) {
    std::shared_ptr<CApp> pApp =Get_pApp();
    pApp->Set_setObjToRemove(ObjDirectory.m_mapBody_Obj[pBody]);
    return 0;
  }


  return 0;
}


int Plug_Spawner(CPhysic_World* &pWorld,
                 CObjDirectory &ObjDirectory, SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance) {
  std::map<std::string, float> & Float_Common = pInstance->m_Float_Common;
  std::map<std::string, int> &Int_Common=pInstance->m_Int_Common;
  std::map<std::string,ObjAttr_t*> &mapObjs = ObjDirectory.m_mapObjs;
  Int_Common["SpawnLimit"]=6;
  b2Body* pBody = pInstance->m_pBody;
  float fSpawnRate_SEC = 1.0;

  b2Body* pSpawnedBody = nullptr;
  std::string strSpawnedObjName;
  pBody->SetGravityScale(0);

  pBody->SetLinearVelocity(b2Vec2( 0,0));
  b2Vec2 vec2CurPos =   pBody->GetPosition();


  Float_Common["SpawnTimeDiff"] +=dbTimeDiff;
  if (Float_Common["SpawnTimeDiff"] > fSpawnRate_SEC ) {
    Float_Common["SpawnTimeDiff"] = 0;
    Int_Common["SpawnCount"] ++;

    if (Int_Common["SpawnCount"] < Int_Common["SpawnLimit"]) {
      std::shared_ptr<CApp> pApp =Get_pApp();
      std::shared_ptr<TMX_Ctx> pTMX_Ctx =  Get_pTMX_Ctx();

      strSpawnedObjName = pWorld->Create_Element(
          *(pTMX_Ctx.get()),205,vec2CurPos.x+3,vec2CurPos.y-3, ObjDirectory);
      pSpawnedBody = mapObjs[strSpawnedObjName]->pBody; 

      auto pInstance = new CPlugin();
      pInstance->m_pBody = pSpawnedBody;
      pInstance->OnInit = Plug_Enemy_Flyer_Init;
      pInstance->OnDeInit = Plug_Enemy_Flyer_DeInit;
      pInstance->OnExecute = Plug_Enemy_Flyer;
      mapObjs[strSpawnedObjName]->pPlugin = pInstance;
      pApp->Set_vecObjToAdd(mapObjs[strSpawnedObjName]);

      printf("\033[1;33m[%s][%d] :x: Spawn [%s]\033[m\n",__FUNCTION__,__LINE__,
                                                     strSpawnedObjName.c_str());
    }

  }

  return 0;
}

int Plug_FPS_Drawer_Init(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance) {
  printf("\033[1;33m[%s][%d] :x: Init \033[m\n",__FUNCTION__,__LINE__);

  std::map<std::string, float> & Float_Common = pInstance->m_Float_Common;
  std::map<std::string, void*> & Ptr_Common = pInstance->m_Ptr_Common;

  Float_Common["DisplayTimeDiff"] = 0;
  Ptr_Common["TxtSurface"] = nullptr;
  Ptr_Common["TxtTexture"] = nullptr;
  return 0;
}

int Plug_FPS_Drawer_DeInit(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance) {
  std::map<std::string, void*> & Ptr_Common = pInstance->m_Ptr_Common;

  if (Ptr_Common["TxtSurface"]) 
    SDL_DestroyTexture ((SDL_Texture*)Ptr_Common["TxtSurface"]);
  if (Ptr_Common["TxtTexture"]) 
    SDL_FreeSurface((SDL_Surface*)Ptr_Common["TxtTexture"]);
  Ptr_Common["TxtSurface"] = nullptr;
  Ptr_Common["TxtTexture"] = nullptr;
  printf("\033[1;33m[%s][%d] :x: DeInit \033[m\n",__FUNCTION__,__LINE__);
  return 0;
}

int Plug_FPS_Drawer(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance) {
  std::map<std::string, float> & Float_Common = pInstance->m_Float_Common;
  std::map<std::string, void*> & Ptr_Common = pInstance->m_Ptr_Common;
  float fDisplayRate_SEC = .05f;

  static SDL_Surface*& pTxtSurface = (SDL_Surface*&)Ptr_Common["TxtSurface"];
  static SDL_Texture*& pTxtTexture = (SDL_Texture*&)Ptr_Common["TxtTexture"];

  std::shared_ptr<CApp> pApp =Get_pApp();
  SDL_Renderer* pRenderer = Get_pApp()->m_pRenderer;
  TTF_Font*     pFont     = Get_pApp()->m_pFont;

  if (Float_Common.find("DisplayTimeDiff") == Float_Common.end())
    Float_Common["DisplayTimeDiff"] = 0.f;


  Float_Common["DisplayTimeDiff"] +=dbTimeDiff;
  if (Float_Common["DisplayTimeDiff"] > fDisplayRate_SEC ) {
    Float_Common["DisplayTimeDiff"] = 0;

    if (pTxtSurface != nullptr && pTxtSurface != nullptr) {
      SDL_DestroyTexture (pTxtTexture);
      SDL_FreeSurface(pTxtSurface);
    }


    // Update FPS
    float fFPS = Get_FPS();
    std::string strFPS = std::string("FPS : ") + std::to_string(fFPS);

    pTxtSurface = TTF_RenderText_Solid(pFont, strFPS.c_str(), SDL_Color {255,255,255,255} );
    pTxtTexture = SDL_CreateTextureFromSurface(pRenderer, pTxtSurface);

  }

  if (pTxtSurface != nullptr && pTxtSurface != nullptr) {
    SDL_Rect DstRect { 20,20,pTxtSurface->w,pTxtSurface->h};
    SDL_RenderCopyEx(pRenderer,pTxtTexture,NULL,&DstRect,0,NULL,SDL_FLIP_NONE);
  }


  return 0;
}
