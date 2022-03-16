#include"Plugins.h"
#include "CPlugin.h"
#include <memory>
#include <math.h>

#define AUTO_AIMING  (0)
#define MOUSE_AIMING (1)

// CHARACTER_IDX
#define TSX_IDX_CHARACTERS (265)
#define IDX_FLYER (TSX_IDX_CHARACTERS+24)
#define IDX_PIERCING_BULLET (TSX_IDX_CHARACTERS+27)
#define IDX_SLUG_BULLET (TSX_IDX_CHARACTERS+28)
#define IDX_SHELL (TSX_IDX_CHARACTERS+29)
#define IDX_FLAME_SMALL (TSX_IDX_CHARACTERS+30)
#define IDX_FLAME_BIG (TSX_IDX_CHARACTERS+31)
#define IDX_SMOKE_SMALL (TSX_IDX_CHARACTERS+32)
#define IDX_SMOKE_MIDDLE (TSX_IDX_CHARACTERS+33)

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

double AngleBetweenVec2(b2Vec2 &vec2A,b2Vec2 &vec2B) {
  return (atan2(vec2B.y,vec2B.x) - atan2(vec2A.y,vec2A.x))*180/M_PI;
}

void Print_Bodies(b2Body* pBody) {
  for (b2Body* b = pBody; b; b = b->GetNext())
  {
    printf("\033[1;33m[%s][%d] :x: 0x%p -> 0x%p \033[m\n",__FUNCTION__,__LINE__,b,b->GetNext());
  }

}

/**
 * @brief Do screen scroll according to the current position 
 *
 * @param vec2CurPos_M[IN]
 * @param pApp[OUT]
 */
void Do_Scroll(b2Vec2& vec2CurPos_M,std::shared_ptr<CApp> &pApp) {
  float fX_Pixel,fY_Pixel;
  float fScale = Get_DrawingScale(); 
  MeterToPixel(vec2CurPos_M.x,vec2CurPos_M.y,fX_Pixel,fY_Pixel);
  pApp->Set_DisplayOffSet(-(fX_Pixel*fScale-SCREEN_WIDTH/2),-(fY_Pixel*fScale-SCREEN_HEIGHT/2) );
  return;
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

/**
 * @brief Get the list of contacted target bodies
 *
 * @param ObjDirectory[IN]
 * @param vecTargetTags[IN]  
 * @param pContacts[IN]
 * @param vecContactedTargeBodies[OUT]
 *
 * @return the number contacted bodies
 */
int Get_Contacted_TargetBody(CObjDirectory &ObjDirectory, 
    std::vector<std::string> &vecTargetTags,
    b2ContactEdge* &pContacts, std::vector<ObjAttr_t*> &vecContactedTargeBodies) 
{
  vecContactedTargeBodies.clear();
  if (pContacts == nullptr || vecTargetTags.size() == 0) 
    return vecContactedTargeBodies.size();
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
        printf("\033[1;33m[%s][%d] :x: COntacted %s \033[m\n",
            __FUNCTION__,__LINE__,pContactedObj->strObjName.c_str());
        vecContactedTargeBodies.push_back(pContactedObj);
      }
      pContacts = pContacts->next;
    } while (pContacts);
  }

  return vecContactedTargeBodies.size();
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

int Plug_Missile_Init(CPhysic_World* &pWorld,             
            CObjDirectory &ObjDirectory,
            SDL_Event* &pEvt,double& dbTimeDiff,
            CPlugin* pInstance) {
  b2Body* pBody = pInstance->m_pBody;
  std::map<std::string, float> & Float_Common = pInstance->m_Float_Common;
  std::map<std::string, b2Vec2> & Vec2_Common = pInstance->m_Vec2_Common;
  std::map<std::string, int> & Int_Common = pInstance->m_Int_Common;
  Float_Common["Direction_Degree"] = 90;
  Float_Common["Thrust"] = 60000;  // newtons(N) per sec
  Float_Common["ReactionTimeDiff"] = 0;
  Float_Common["Rotation_Rate"] = 1440; // degree per sec ;
                                        // this is related to the hit rate
  Int_Common["DamageFlag"]=false;
  Float_Common["FirePower"] = 1000;
  Float_Common["LifeTime"] = 8;
  Float_Common["Energy"]= 1;
  Float_Common["Damage"]= 0;

  Float_Common["HitFlameTime"] = 0.2;
  Vec2_Common["HitPos"]={0,0};


  Float_Common["AfterImage_Interval"] =0.1;
  Float_Common["AfterImage_Time"] =0;
  Int_Common["AfterImage"]=10;
  for (int i =0 ; i< Int_Common["AfterImage"]; i++) {
      char pBuf[256]={}; 
      sprintf(pBuf,"AfterImage%02d",i);
      Vec2_Common[pBuf]={0,0};
  }

  pBody->SetGravityScale(0);

  return 0;
}

int Plug_Missile(CPhysic_World* &pWorld,             
            CObjDirectory &ObjDirectory,
            SDL_Event* &pEvt,double& dbTimeDiff,
            CPlugin* pInstance) {
  std::map<std::string, float> & Float_Common = pInstance->m_Float_Common;
  std::map<std::string, int> & Int_Common = pInstance->m_Int_Common;
  std::map<std::string, b2Vec2> & Vec2_Common = pInstance->m_Vec2_Common;
  std::map<std::string, std::vector<std::string>> & Str_Vec_Common = 
                                                    pInstance->m_Str_Vec_Common;
  int iIdxSmoke = IDX_SMOKE_MIDDLE;
  float fFirePower= Float_Common["FirePower"];
  //float fReactionRate_SEC = 0.001;
  float fReactionRate_SEC = 0.003;
  b2Body* pBody = pInstance->m_pBody;
  b2Vec2 vec2CurPos_M=pBody->GetPosition();
  std::shared_ptr<CApp> pApp =Get_pApp();

  b2Vec2 vec2Direction = AngleToVector(Float_Common["Direction_Degree"]); 
  pBody->SetTransform(vec2CurPos_M,-(Float_Common["Direction_Degree"]-90));
  vec2Direction*=(Float_Common["Thrust"]*dbTimeDiff);
  pBody->ApplyForceToCenter(vec2Direction, true); 


  float fRange_M = 55.f;

  // Target to Attack
//  std::vector<std::string> vecTargetTags ={
//    "Enemy_Flyer",
//    "Enemy_Ground_Tracker",
//  };
  std::vector<std::string> &vecTargetTags = Str_Vec_Common["Targets"];

  Float_Common["ReactionTimeDiff"] +=dbTimeDiff;
  float fTargetAngle;
  if (Float_Common["ReactionTimeDiff"] > fReactionRate_SEC ) 
  {
    Float_Common["ReactionTimeDiff"] = 0;
  
    std::vector<b2Body*> vecTargetBodies;
    Find_Bodies(vecTargetTags,fRange_M,vec2CurPos_M,vecTargetBodies,ObjDirectory);
    int iTargetBodySize = vecTargetBodies.size();
    SDL_Color Color_RED =COLOR_RED;
    SDL_Color Color_YELLOW =COLOR_YELLOW;
    if (iTargetBodySize > 0  ) {
      for (auto pTargetBody : vecTargetBodies) {
        b2Vec2 vec2TargetPos = pTargetBody->GetPosition();
        b2Vec2 vec2FirstTargetPos = vecTargetBodies[0]->GetPosition();
        Dbg_DrawLine_Scale(
            vec2CurPos_M.x, 
            vec2CurPos_M.y, 
            vec2TargetPos.x, vec2TargetPos.y,Color_YELLOW);
        Dbg_DrawLine_Scale(
            vec2CurPos_M.x, 
            vec2CurPos_M.y, 
            vec2FirstTargetPos.x, vec2FirstTargetPos.y,Color_RED);

      }
      b2Vec2 vec2TargetDir =  vecTargetBodies[0]->GetPosition()-vec2CurPos_M;
      vec2TargetDir.Normalize();
      fTargetAngle = AngleBetweenVec2(vec2TargetDir,vec2Direction);

      if (fTargetAngle  > 0) {
        Float_Common["Direction_Degree"] -=Float_Common["Rotation_Rate"]*dbTimeDiff;
      } else if (fTargetAngle  <= 0) {
        Float_Common["Direction_Degree"] +=Float_Common["Rotation_Rate"]*dbTimeDiff;
      }
    }
  }
 
  std::vector<ObjAttr_t *> vecContactedTargeBodies;
  b2ContactEdge* pContacts = pBody->GetContactList();

  if (Int_Common["DamageFlag"]== false) {
    if ( Get_Contacted_TargetBody(ObjDirectory, vecTargetTags, pContacts, vecContactedTargeBodies) ) {
      for (auto pContactedObj : vecContactedTargeBodies) {
        // Attack contacted body
        pContactedObj->pPlugin->m_Float_Common["Damage"]+=fFirePower;
        Vec2_Common["HitPos"]=pContactedObj->pBody->GetPosition();
        Int_Common["DamageFlag"] = true;
        Play_Sound("boom");
        Float_Common["LifeTime"] = Float_Common["HitFlameTime"];
      }
    }
  }
  
  Float_Common["LifeTime"] -=dbTimeDiff;
  if(Float_Common["LifeTime"] <Float_Common["HitFlameTime"])
  {
    if (Int_Common["DamageFlag"]== false) {
      Int_Common["DamageFlag"] = true;
      Vec2_Common["HitPos"]=pBody->GetPosition();
    }
    pApp->Draw_Sprite_Scale(
        Vec2_Common["HitPos"].x-(1.33/2),
        Vec2_Common["HitPos"].y+(1.33/2), IDX_FLAME_BIG);
    if (Float_Common["LifeTime"] < 0) {
      printf("\033[1;31m[%s][%d] :x: Die Missile \033[m\n",__FUNCTION__,__LINE__);
      Play_Sound("boom");
      pApp->Set_setObjToRemove(ObjDirectory.m_mapBody_Obj[pBody]);
      return 0;
    } 
  }


  

  // draw after image
  Float_Common["AfterImage_Time"]+=dbTimeDiff;
  if (Float_Common["AfterImage_Time"] > Float_Common["AfterImage_Interval"]) {
    Float_Common["AfterImage_Time"] =0;

    for (int i =Int_Common["AfterImage"]-1 ; i > 0  ;i--) {
      char pBuf1[256]={}; 
      char pBuf2[256]={}; 
      sprintf(pBuf1,"AfterImage%02d",i);
      sprintf(pBuf2,"AfterImage%02d",i-1);
      Vec2_Common[pBuf1]=Vec2_Common[pBuf2];
    }
    Vec2_Common["AfterImage00"] =pBody->GetPosition();

  }
  for (int i = 1 ; i < Int_Common["AfterImage"] ;i++) {
    char pBuf[256]={}; 
    sprintf(pBuf,"AfterImage%02d",i);
    pApp->Draw_Sprite_Scale(
        Vec2_Common[pBuf].x-(1.33/2),
        Vec2_Common[pBuf].y+(1.33/2), iIdxSmoke);
  }

  if ( Float_Common["Damage"] > 0) {
    Float_Common["Energy"] -= Float_Common["Damage"];
    Float_Common["Damage"]=0;

    return 0;
  }
  if ( Float_Common["Energy"] < 0) {
    pApp->Set_setObjToRemove(ObjDirectory.m_mapBody_Obj[pBody]);
    return 0;
  }

  

  return 0;
}
 
int Plug_Bullet_Init(CPhysic_World* &pWorld,             
            CObjDirectory &ObjDirectory,
            SDL_Event* &pEvt,double& dbTimeDiff,
            CPlugin* pInstance) {
  pInstance->m_Float_Common["LifeTime"] = 2;
  b2Body* pBody = pInstance->m_pBody;
  std::map<std::string, float> & Float_Common = pInstance->m_Float_Common;
  std::map<std::string, int> & Int_Common = pInstance->m_Int_Common;
  std::map<std::string, b2Vec2> & Vec2_Common = pInstance->m_Vec2_Common;

  // To show muzzle flame
  Float_Common["MuzzleFlameTime"] = 0.1;
  Vec2_Common["StartPos"]=pBody->GetPosition();

  // Damage
  Int_Common["DamageFlag"]=false;
  Float_Common["FirePower"] = 10.f;
  Float_Common["Damage"] = 0.f;

  Float_Common["HitFlameTime"] = 0.2;
  Vec2_Common["HitPos"]={0,0};


  // Target to Attack
 // Str_Vec_Common["Target"]={"Enemy_Flyer",
 //                           "Enemy_Ground_Tracker"};
  //Str_Vec_Common["Target"]={"Player01",
  //                          };

  return 0;
}


int Plug_Bullet(CPhysic_World* &pWorld,             
            CObjDirectory &ObjDirectory,
            SDL_Event* &pEvt,double& dbTimeDiff,
            CPlugin* pInstance) {
  b2Body* pBody = pInstance->m_pBody;
  std::map<std::string, float> & Float_Common = pInstance->m_Float_Common;
  std::map<std::string, b2Vec2> & Vec2_Common = pInstance->m_Vec2_Common;
  std::map<std::string, int> & Int_Common = pInstance->m_Int_Common;
  std::map<std::string, std::vector<std::string>> & Str_Vec_Common = 
                                                    pInstance->m_Str_Vec_Common;
  std::shared_ptr<CApp> pApp =Get_pApp();
  Float_Common["LifeTime"]-=dbTimeDiff;
  // Damage point of the bullet
  float fFirePower= Float_Common["FirePower"];


  Float_Common["MuzzleFlameTime"] -=dbTimeDiff;
  if (Float_Common["MuzzleFlameTime"] > 0) {
    pApp->Draw_Sprite_Scale(
        Vec2_Common["StartPos"].x-(1.33/5),
        Vec2_Common["StartPos"].y+(1.33/5), IDX_FLAME_SMALL);
  }


  if (Float_Common["LifeTime"] < 0) {
    printf("\033[1;31m[%s][%d] :x: Die Bullet \033[m\n",__FUNCTION__,__LINE__);
    pApp->Set_setObjToRemove(ObjDirectory.m_mapBody_Obj[pBody]);
    return 0;
  }


  // To show hit flame
  if (Int_Common["DamageFlag"]== true) {
    Float_Common["HitFlameTime"]-=dbTimeDiff;
    if (Float_Common["HitFlameTime"]> 0) {
      pApp->Draw_Sprite_Scale(
          Vec2_Common["HitPos"].x-(1.33/2),
          Vec2_Common["HitPos"].y+(1.33/2), IDX_FLAME_BIG);
    }
  }
  
  std::vector<ObjAttr_t *> vecContactedTargeBodies;
  b2ContactEdge* pContacts = pBody->GetContactList();

  if (Int_Common["DamageFlag"]== false) {
    if ( Get_Contacted_TargetBody(ObjDirectory, Str_Vec_Common["Targets"], 
                                  pContacts, vecContactedTargeBodies) ) {
      for (auto pContactedObj : vecContactedTargeBodies) {
        // Bullet damage inflict damage to only one target
        Int_Common["DamageFlag"]= true;
        // Attack contacted body
        pContactedObj->pPlugin->m_Float_Common["Damage"]+=fFirePower;
        Vec2_Common["HitPos"]=pContactedObj->pBody->GetPosition();
        Play_Sound("boom");
        break;
      }
      // Remove Self body
      Float_Common["LifeTime"] = Float_Common["HitFlameTime"];
      //pApp->Set_setObjToRemove(ObjDirectory.m_mapBody_Obj[pBody]);
    }
  }

  return 0;
}
int Spawn_Missile(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory,
                 b2Vec2 &vec2CurPos,b2Vec2 &vec2Direction,
                 std::vector<std::string> &vecTargets) {
  int iBulletTileIdx;
  int iIdxShellBullet = IDX_SHELL; 
  iBulletTileIdx= iIdxShellBullet;
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
  //pSpawnedBody->ApplyLinearImpulseToCenter(vec2Direction, true);

  auto pInstance = new CPlugin(strSpawnedObjName);
  pInstance->m_pBody = pSpawnedBody;
  pInstance->OnInit = Plug_Missile_Init;
  pInstance->OnDeInit = nullptr;
  pInstance->OnExecute = Plug_Missile;
  pInstance->m_Str_Vec_Common["Targets"] = vecTargets;

  mapObjs[strSpawnedObjName]->pPlugin = pInstance;
  mapObjs[strSpawnedObjName]->vecTag.clear();
  mapObjs[strSpawnedObjName]->vecTag.push_back("Missile_Player");
  pApp->Set_vecObjToAdd(mapObjs[strSpawnedObjName]);

  ObjDirectory.UpdateDirectory();

  return 0;
}

int Spawn_Bullet(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory,
                 b2Vec2 &vec2CurPos,b2Vec2 &vec2Direction,
                 std::vector<std::string> &vecTargets) {
  int iBulletTileIdx;
  // 209 : slug bullet
  int iIdxSlugBullet = IDX_SLUG_BULLET;
  // 208 : piercing bullet
  //int iIdxPiercingBullet = IDX_PIERCING_BULLET;
  iBulletTileIdx= iIdxSlugBullet;
  float fBulletBaseAngle = -90.f;
  std::shared_ptr<CApp> pApp =Get_pApp();
  std::shared_ptr<TMX_Ctx> pTMX_Ctx =  Get_pTMX_Ctx();
  std::map<std::string,ObjAttr_t*> &mapObjs = ObjDirectory.m_mapObjs;

  std::string strSpawnedObjName;
  //float fSpawnDistance_M=1.9f;
  float fSpawnDistance_M=1.5f;
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

  auto pInstance = new CPlugin(strSpawnedObjName);
  pInstance->m_pBody = pSpawnedBody;
  pInstance->OnInit = Plug_Bullet_Init;
  pInstance->OnDeInit = nullptr;
  pInstance->OnExecute = Plug_Bullet;
  pInstance->m_Str_Vec_Common["Targets"]=vecTargets;

  mapObjs[strSpawnedObjName]->pPlugin = pInstance;
  mapObjs[strSpawnedObjName]->vecTag.clear();
  pApp->Set_vecObjToAdd(mapObjs[strSpawnedObjName]);

  ObjDirectory.UpdateDirectory();

  Play_Sound("tang");
  return 0;
}

int Spawn_Bullet(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory,
                 b2Vec2 &vec2CurPos,b2Vec2& vec2Direction, float fImpulse,
                 std::vector<std::string> &vecTargets) {
  vec2Direction*=fImpulse;
  return Spawn_Bullet(pWorld,ObjDirectory,vec2CurPos,vec2Direction,vecTargets);
}



int Spawn_Bullet(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory,
                 b2Vec2 &vec2CurPos,float fAngle,float fImpulse,
                 std::vector<std::string> &vecTargets) {
  b2Vec2 vecDirection = AngleToVector(fAngle);
  return Spawn_Bullet(pWorld,ObjDirectory,vec2CurPos,vecDirection,
                      fImpulse,vecTargets);
}


int Plug_Player01_Init(CPhysic_World* &pWorld,
                        CObjDirectory &ObjDirectory,
                        SDL_Event* &pEvt,double& dbTimeDiff,
                        CPlugin* pInstance) {

  std::map<std::string, int> & Int_Common = pInstance->m_Int_Common;
  std::map<std::string, float> & Float_Common = pInstance->m_Float_Common;
  std::map<std::string,std::vector<std::string>> & Str_Vec_Common = 
                                                    pInstance->m_Str_Vec_Common;
  Int_Common["PlayerIdx"]=1;
  //Int_Common["AimingState"] = MOUSE_AIMING;
  Int_Common["AimingState"] = AUTO_AIMING;
  Int_Common["AimingTarget"] = 0;
  // Joystick Input index
  Int_Common["JoystickInputIdx"]=0;
  Float_Common["Energy"]= 100;
  Float_Common["TotalEnergy"] = 100;
  Float_Common["Damage"]= 0;

  Float_Common["BulletImpulse"]= 200.f;
  Float_Common["RecoilRate"]= 0.1f;

  Str_Vec_Common["Targets"]= { "Enemy_Flyer",
                               "Enemy_Ground_Tracker"};
  return 0;
}
int Plug_Player02_Init(CPhysic_World* &pWorld,
                        CObjDirectory &ObjDirectory,
                        SDL_Event* &pEvt,double& dbTimeDiff,
                        CPlugin* pInstance) {

  std::map<std::string, int> & Int_Common = pInstance->m_Int_Common;
  std::map<std::string, float> & Float_Common = pInstance->m_Float_Common;
  std::map<std::string,std::vector<std::string>> & Str_Vec_Common = 
                                                    pInstance->m_Str_Vec_Common;
  Int_Common["PlayerIdx"]=2;
  //Int_Common["AimingState"] = MOUSE_AIMING;
  Int_Common["AimingState"] = AUTO_AIMING;
  Int_Common["AimingTarget"] = 0;
  // Joystick Input index
  Int_Common["JoystickInputIdx"]=1;
  Float_Common["Energy"]= 100;
  Float_Common["TotalEnergy"] = 100;
  Float_Common["Damage"]= 0;

  Float_Common["BulletImpulse"]= 200.f;
  Float_Common["RecoilRate"]= 0.1f;

  Str_Vec_Common["Targets"]= { "Enemy_Flyer",
                               "Enemy_Ground_Tracker"};
  return 0;
}




int Plug_Player01(CPhysic_World* &pWorld,
                  CObjDirectory &ObjDirectory,
                  SDL_Event* &pEvt,double& dbTimeDiff,
                  CPlugin* pInstance) {
  std::map<std::string, float> & Float_Common = pInstance->m_Float_Common;
  std::map<std::string,std::vector<std::string>> & Str_Vec_Common = 
                                                    pInstance->m_Str_Vec_Common;
  std::map<std::string, int> & Int_Common = pInstance->m_Int_Common;
  std::shared_ptr<CApp> pApp =Get_pApp();

  // Find Player 1 Body, it should be one
  b2Body* pBody = pInstance->m_pBody;
  b2Vec2 vec2CurPos_M=pBody->GetPosition();


  b2ContactEdge* pContacts = pBody->GetContactList();

  float fMass = pBody->GetMass();
  float fJumpImpulse = fMass * 7.0;
  float fMoveSpeed = 6.0;

  // Joystick input Idx
  int iJoyIdx = Int_Common["JoystickInputIdx"]; 
  static  bool bPrevIsGround = false;
  bool bIsGround=false; 
  bool bIsFly=false; 
  b2WorldManifold worldManifold;
  auto vecVelocity = pBody->GetLinearVelocity();

  // Target to Attack
  std::vector<std::string> &vecTargetTags =Str_Vec_Common["Targets"];


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
  b2Vec2 vec2MissileDirection(0,3);
  

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
  b2Vec2 vec2Cursor_M = PixelToMeter(x,y,fScale_Pixel_per_Meter);
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
  float fImpulse = Float_Common["BulletImpulse"];
  vec2BulletRecoil*=-(fImpulse*Float_Common["RecoilRate"]);// recoil
  if (pEvt) {
    
    // Input Event
    if( pEvt->type == SDL_JOYAXISMOTION && pEvt->jaxis.which == iJoyIdx) {
      printf("\033[1;32m[%s][%d] :x: Joy[%d] %d %d \033[m\n",
          __FUNCTION__,__LINE__,pEvt->jaxis.which,pEvt->jaxis.axis,pEvt->jaxis.value);
      
      if (pEvt->jaxis.axis == 0 ) { // axis X 
        if (pEvt->jaxis.value < 0) {
          printf("\033[1;33m[%s][%d] :x: Left \033[m\n",__FUNCTION__,__LINE__);

            pBody->SetLinearVelocity(b2Vec2( -fMoveSpeed,vecVelocity.y));
        }
        if (pEvt->jaxis.value > 0) {
          printf("\033[1;33m[%s][%d] :x: Right \033[m\n",__FUNCTION__,__LINE__);
            pBody->SetLinearVelocity(b2Vec2(  fMoveSpeed,vecVelocity.y));
        }
        if (pEvt->jaxis.value == 0) {
          if (bIsGround)
            pBody->SetLinearVelocity(b2Vec2(  0,vecVelocity.y));
        }
      }

      if (pEvt->jaxis.axis == 1 ) { // axis Y  
        if (pEvt->jaxis.value < 0) {
          printf("\033[1;33m[%s][%d] :x: Up \033[m\n",__FUNCTION__,__LINE__);

        }
        if (pEvt->jaxis.value > 0) {
          printf("\033[1;33m[%s][%d] :x: Down \033[m\n",__FUNCTION__,__LINE__);
        }

      }
    }
  
    if( pEvt->type == SDL_JOYBUTTONDOWN && pEvt->jbutton.which == iJoyIdx) {
      printf("\033[1;33m[%s][%d] :x: Joy [%d] Button %d \033[m\n",
          __FUNCTION__,__LINE__,pEvt->jbutton.which,pEvt->jbutton.button);
      switch(pEvt->jbutton.button) {
        case 0:
          // Button 1 
          // Shot Missile
          if (!Spawn_Missile(pWorld,ObjDirectory,vec2CurPos_M,
                vec2MissileDirection,vecTargetTags)) {
          }
          break;
        case 1:
          // Button 2 
          // Shot Bullet
          if (!Spawn_Bullet(pWorld,ObjDirectory,vec2CurPos_M,
                vec2BulletDirection,fImpulse,vecTargetTags)) {
            // Recoil
            pBody->ApplyLinearImpulseToCenter(vec2BulletRecoil, true);
          }
          break;
        case 2:
          // Button 3 
          // Jump
          if (bIsGround || bIsFly ) 
          {
            pBody->ApplyLinearImpulseToCenter(b2Vec2(0,fJumpImpulse), true);
          }
          break;
        case 4:
          // Button L1
          // change target 
          ChangeTargetIdx(iTargetBodySize,pInstance->m_Int_Common["AimingTarget"],true);
          break;
        case 5:
          // Button R1
          // change target 
          ChangeTargetIdx(iTargetBodySize,pInstance->m_Int_Common["AimingTarget"],false);
          break;
      }
    }

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
            printf("\033[1;36m[%s][%d] :x: change \033[m\n",
                __FUNCTION__,__LINE__);
            break;
          case SDLK_f:
            printf("\033[1;32m[%s][%d] :x: Mouse %d %d \033[m\n",__FUNCTION__,__LINE__,x,y);
            printf("\033[1;33m[%s][%d] :x: Fire ,cur (%f %f)->(%f,%f) aim (%f %f) \033[m\n",
                __FUNCTION__,__LINE__,vec2CurPos_M.x,vec2CurPos_M.y,
                                      vec2Cursor_M.x,vec2Cursor_M.y,
                                      vec2BulletDirection.x,vec2BulletDirection.y);


            {
              if (!Spawn_Bullet(pWorld,ObjDirectory,vec2CurPos_M,
                    vec2BulletDirection,fImpulse,vecTargetTags)) {
              // Recoil
              pBody->ApplyLinearImpulseToCenter(vec2BulletRecoil, true);
              }
            }


            break;
          case SDLK_g:
             if (!Spawn_Missile(pWorld,ObjDirectory,vec2CurPos_M,
                    vec2MissileDirection,vecTargetTags)) {

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
  if ( Float_Common["Damage"] > 0) {
    Float_Common["Energy"] -= Float_Common["Damage"];
    Float_Common["Damage"]=0;
  
    return 0;
  }

  if ( Float_Common["Energy"] < 0) {
    pApp->Set_setObjToRemove(ObjDirectory.m_mapBody_Obj[pBody]);
    return 0;
  } else {
    // Draw Energy
    float fObjWidth =
                ObjDirectory.m_mapObjs[pInstance->m_strObjName]->GetTile_fW_M();
    float fObjHeight =
                ObjDirectory.m_mapObjs[pInstance->m_strObjName]->GetTile_fH_M();
    b2Vec2 vec2Offset(-(fObjWidth/2),-(fObjHeight/2)-0.2);
    float fEnergyRate =Float_Common["Energy"]/Float_Common["TotalEnergy"]; 
    Draw_Energy_Bar(fEnergyRate,
        vec2CurPos_M,vec2Offset,fObjWidth*1);
  }
#if 0 // :x: for test

  // Draw Energy
  float fObjWidth =
    ObjDirectory.m_mapObjs[pInstance->m_strObjName]->GetTile_fW_M();
  float fObjHeight =
    ObjDirectory.m_mapObjs[pInstance->m_strObjName]->GetTile_fH_M();

  b2Vec2 vec2Offset(-(fObjWidth/2),-(fObjHeight/2)-0.2);
  float fEnergyRate =Float_Common["Energy"]/Float_Common["TotalEnergy"]; 
  Draw_Energy_Bar(fEnergyRate,
      vec2CurPos_M,vec2Offset,fObjWidth*1.0);
#endif // :x: for test

  return 0;

}


int Plug_Enemy_Flyer_Init(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, 
                              SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance) {
  printf("\033[1;33m[%s][%d] :x: Init Enemy_Flyer \033[m\n",
      __FUNCTION__,__LINE__);
  std::map<std::string, float> & Float_Common = pInstance->m_Float_Common;
  std::map<std::string, std::vector<std::string>> & Str_Vec_Common = 
                                                    pInstance->m_Str_Vec_Common;


  // Initiation
  Float_Common["InputTimeDiff"]= 0.f;
  Float_Common["ReactionTimeDiff"]= 0.f;

  // character reaction time
  Float_Common["ReactionTime"]= 0.8f;

  Float_Common["TotalEnergy"]= 100;
  Float_Common["Energy"] = Float_Common["TotalEnergy"];
  Float_Common["Damage"] = 0;

  Float_Common["BulletImpulse"]= 200.f;
  Float_Common["RecoilRate"]= 0.015f;

 

  Str_Vec_Common["Targets"]= { "Player01","Player02",
                               "Missile_Player"
                               };

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
  std::map<std::string, std::vector<std::string>> & Str_Vec_Common = 
                                                    pInstance->m_Str_Vec_Common;

  std::shared_ptr<CApp> pApp =Get_pApp();
  // character control rate --> 0.5 sec
  float fCtrlRate_SEC = 0.5;
  float fReactionRate_SEC =Float_Common["ReactionTime"];
  // Target Search Range (Meter)
  float fRange_M = 35.f;
  float fMoveSpeed = 3.0;
  float fMoveSpeed_X = 0;
  float fMoveSpeed_Y = 0;

  b2Body* pBody = pInstance->m_pBody;
  b2Vec2 vec2CurPos_M = pBody->GetPosition();

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
  Float_Common["ReactionTimeDiff"] +=dbTimeDiff;
  if (Float_Common["ReactionTimeDiff"] > fReactionRate_SEC ) {
    Float_Common["ReactionTimeDiff"] = 0;

    // Find Targets

    // Targets are players
    std::vector<std::string> &vecTargetTags = Str_Vec_Common["Targets"];
    std::vector<b2Body*> vecTargetBodies;
    Find_Bodies(vecTargetTags,fRange_M,vec2CurPos_M,vecTargetBodies,ObjDirectory);

    // set the direction to the first target 
    if (vecTargetBodies.size() > 0) {
      b2Body* &pTargetBody = vecTargetBodies[0];
      b2Vec2 vec2TargetPos = pTargetBody->GetPosition();
      SDL_Color Color =COLOR_RED;
      Dbg_DrawLine_Scale(
          vec2CurPos_M.x, 
          vec2CurPos_M.y, 
          vec2TargetPos.x, vec2TargetPos.y,Color);
      if ( vec2TargetPos.x < vec2CurPos_M.x ) {
        Str_Common["Direction_X"] = "Minus";
      } else 
      {
        Str_Common["Direction_X"] = "Plus";
      }
      if ( vec2TargetPos.y < vec2CurPos_M.y ) {
        Str_Common["Direction_Y"] = "Minus";
      } else 
      {
        Str_Common["Direction_Y"] = "Plus";
      }

#if 1 // :x: for test
      // Fire To target
      b2Vec2 vec2CurPos_M = pBody->GetPosition();
      b2Vec2 vec2Aim = vec2TargetPos-vec2CurPos_M;
      float fImpulse = Float_Common["BulletImpulse"];
      b2Vec2 vec2BulletRecoil =vec2Aim;
      vec2BulletRecoil*=-(fImpulse*Float_Common["RecoilRate"]);// recoil
      vec2Aim.Normalize();
      if (!Spawn_Bullet(pWorld,ObjDirectory,vec2CurPos_M,
            vec2Aim,fImpulse,vecTargetTags)) {
        // Recoil
        pBody->ApplyLinearImpulseToCenter(vec2BulletRecoil, true);
      }
#endif // :x: for test
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
    pApp->Set_setObjToRemove(ObjDirectory.m_mapBody_Obj[pBody]);
    Play_Sound("boom");
    return 0;
  } else {
    // Draw Energy
    float fObjWidth =
                ObjDirectory.m_mapObjs[pInstance->m_strObjName]->GetTile_fW_M();
    float fObjHeight =
                ObjDirectory.m_mapObjs[pInstance->m_strObjName]->GetTile_fH_M();
    b2Vec2 vec2Offset(-(fObjWidth/2),-(fObjHeight/2)-0.2);
    float fEnergyRate =Float_Common["Energy"]/Float_Common["TotalEnergy"]; 
    Draw_Energy_Bar(fEnergyRate,
        vec2CurPos_M,vec2Offset,fObjWidth*1);
  }
  return 0;
}

int Plug_Enemy_Ground_Tracker_Init(CPhysic_World* &pWorld,
                                  CObjDirectory &ObjDirectory, SDL_Event* &pEvt,
                                  double& dbTimeDiff, CPlugin* pInstance) {
  printf("\033[1;33m[%s][%d] :x: Init  \033[m\n", __FUNCTION__,__LINE__);
  std::map<std::string, float> & Float_Common = pInstance->m_Float_Common;
  std::map<std::string, std::string> &Str_Common =pInstance->m_Str_Common;
  std::map<std::string, std::vector<std::string>> & Str_Vec_Common = 
                                                    pInstance->m_Str_Vec_Common;

  // Initiation
  Float_Common["InputTimeDiff"]= 0.f;
  Float_Common["ReactionTimeDiff"]= 0.f;
  Float_Common["ReactionTime"]= 0.7f;
  Float_Common["TotalEnergy"]= 100;
  Float_Common["Energy"] = Float_Common["TotalEnergy"];
  Float_Common["Damage"]= 0;

  Float_Common["BulletImpulse"]= 200.f;
  Float_Common["RecoilRate"]= 0.015f;

  // Initiate direction
  Str_Common["Direction_X"] = "Stop";
  Str_Common["Direction_Y"] = "Stop";

  Str_Vec_Common["Targets"]= { "Player01","Player02",
    "Missile_Player"
                               };
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
  std::map<std::string, std::vector<std::string>> & Str_Vec_Common = 
                                                    pInstance->m_Str_Vec_Common;

  // character control rate --> 0.5 sec
  float fCtrlRate_SEC = 0.5;
  // character reaction rate --> 0.8 sec
  float fReactionRate_SEC = Float_Common["ReactionTime"];
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


  Float_Common["ReactionTimeDiff"] +=dbTimeDiff;
  if (Float_Common["ReactionTimeDiff"] > fReactionRate_SEC ) {
    Float_Common["ReactionTimeDiff"] = 0;

    // Find Targets

    // Targets are players
    std::vector<std::string>& vecTargetTags = Str_Vec_Common["Targets"];

    b2Vec2 vec2Cur_Pos_M = pBody->GetPosition();
    std::vector<b2Body*> vecTargetBodies;
    Find_Bodies(vecTargetTags,fRange_M,vec2Cur_Pos_M,vecTargetBodies,ObjDirectory);

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
#if 1 // :x: for test
      // Fire To target
      b2Vec2 vec2CurPos_M = pBody->GetPosition();
      b2Vec2 vec2Aim = vec2TargetPos-vec2CurPos_M;
      float fImpulse = Float_Common["BulletImpulse"];
      b2Vec2 vec2BulletRecoil =vec2Aim;
      vec2BulletRecoil*=-(fImpulse*Float_Common["RecoilRate"]);// recoil
      vec2Aim.Normalize();
      if (!Spawn_Bullet(pWorld,ObjDirectory,vec2CurPos_M,
            vec2Aim,fImpulse,vecTargetTags)) {
        // Recoil
        pBody->ApplyLinearImpulseToCenter(vec2BulletRecoil, true);
      }
#endif // :x: for test
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
    Play_Sound("boom");

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
  int iSpawnIdx = IDX_FLYER;
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
          *(pTMX_Ctx.get()),iSpawnIdx,
          vec2CurPos.x+3,vec2CurPos.y-3, ObjDirectory);
      pSpawnedBody = mapObjs[strSpawnedObjName]->pBody; 

      auto pInstance = new CPlugin(strSpawnedObjName);
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
int Draw_Energy_Bar(float fEnergyRate, 
                    b2Vec2 &vec2Pos_Pixel,
                    float fBarSizeWidth_Pixel,float fBarSizeHeight_Pixel
                    ) {
  int iSizeHeight_Pixel=fBarSizeHeight_Pixel;
  float fBarSize = fEnergyRate *fBarSizeWidth_Pixel;
  SDL_Color BorderColor = COLOR_DARK_GRAY;
  SDL_Color BarColor = COLOR_RED;
  if (fEnergyRate>0.8) {
    BarColor = COLOR_GREEN;
  } else 
  if (fEnergyRate>0.4) {
    BarColor = COLOR_YELLOW;
  } 
  std::shared_ptr<CApp> pApp =Get_pApp();


  for (int i=0 ; i < iSizeHeight_Pixel ; i++) {
    float fLineY=vec2Pos_Pixel.y-(iSizeHeight_Pixel/2)+i;
    pApp->Draw_Line_Pixel(
        vec2Pos_Pixel.x,
        fLineY,
        vec2Pos_Pixel.x+fBarSizeWidth_Pixel,
        fLineY,
        BorderColor);

    if (i >0 && i < iSizeHeight_Pixel-1 ) {
    pApp->Draw_Line_Pixel(
        vec2Pos_Pixel.x+1,
        fLineY,
        vec2Pos_Pixel.x+fBarSize-1,
        fLineY,BarColor);
    }
  }


  return 0;
}

int Draw_Energy_Bar(float fEnergyRate,
                    b2Vec2 &vec2CurPos,b2Vec2 &vec2Offset,float fBarSizeWidth_M,
                    float fBarSizeHeight_Pixel,
                    float fScale_Pixel_per_Meter
                    ) {
  float fTotalWidth_Pixel =fBarSizeWidth_M*fScale_Pixel_per_Meter; // Bar Total width - pixel
  b2Vec2 vec2LineStartPos = vec2CurPos+vec2Offset;
  b2Vec2 vec2LineStartPos_Pixel;
  MeterToPixel(vec2LineStartPos.x,vec2LineStartPos.y,
               vec2LineStartPos_Pixel.x,vec2LineStartPos_Pixel.y);
  Draw_Energy_Bar(fEnergyRate,vec2LineStartPos_Pixel,fTotalWidth_Pixel,fBarSizeHeight_Pixel);

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
  SDL_Renderer* pRenderer = pApp->m_pRenderer;
  TTF_Font*     pFont     = pApp->m_pFont;



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

int Plug_Judge(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance) {

//  std::shared_ptr<CApp> pApp =Get_pApp();
//  SDL_Renderer* pRenderer = pApp->m_pRenderer;
//  TTF_Font*     pFont     = pApp->m_pFont;


  return 0;
}
int Plug_Scroll_Init(CPhysic_World* &pWorld,             
            CObjDirectory &ObjDirectory,
            SDL_Event* &pEvt,double& dbTimeDiff,
            CPlugin* pInstance) {
  std::map<std::string, float> & Float_Common = pInstance->m_Float_Common;
  //std::map<std::string, int> & Int_Common = pInstance->m_Int_Common;
  //std::map<std::string, b2Vec2> & Vec2_Common = pInstance->m_Vec2_Common;
  std::map<std::string, void*> & Ptr_Common = pInstance->m_Ptr_Common;

  // Check Every 1 second
  Float_Common["FindScrollTarget_Interval_SEC"] = 1;
  Float_Common["FindScrollTarget_time_SEC"] = 0;
  Ptr_Common["ScrollTarget"] = nullptr;
  


  return 0;
}



/**
 * @brief Set the screen scroll
 *
 * @param pWorld
 * @param ObjDirectory
 * @param pEvt
 * @param dbTimeDiff
 * @param pInstance
 *
 * @return 
 */
int Plug_Scroll(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance) {
#if 1 // :x: for test

  std::shared_ptr<CApp> pApp =Get_pApp();

  std::map<std::string, float> & Float_Common = pInstance->m_Float_Common;
  std::map<std::string, void*> & Ptr_Common = pInstance->m_Ptr_Common;
  ObjAttr_t* &pScrollTarget =(ObjAttr_t*&)Ptr_Common["ScrollTarget"];

  float &fFindScrollTarget_time_SEC =
                                  Float_Common["FindScrollTarget_time_SEC"];
  float &fFindScrollTarget_Interval_SEC = 
                                  Float_Common["FindScrollTarget_Interval_SEC"];
  fFindScrollTarget_time_SEC += dbTimeDiff;
  // Find Scroll target every "FindScrollTarget_Interval_SEC" seconds
  if (fFindScrollTarget_time_SEC > fFindScrollTarget_Interval_SEC) {
    fFindScrollTarget_time_SEC=0;
    // Find player 1 
    if (ObjDirectory.m_mapTagObj.find("Player01") != ObjDirectory.m_mapTagObj.end()){
      if (ObjDirectory.m_mapTagObj["Player01"].size() > 1) {
        printf("\033[1;31m[%s][%d] :x: Error! Player01 Tag"
            " should be unique \033[m\n",
            __FUNCTION__,__LINE__);
        return -1;
      }
      pScrollTarget =(ObjAttr_t*)ObjDirectory.m_mapTagObj["Player01"][0];
    }
    else if (ObjDirectory.m_mapTagObj.find("Player02") != ObjDirectory.m_mapTagObj.end()){
      if (ObjDirectory.m_mapTagObj["Player01"].size() > 1) {
        printf("\033[1;31m[%s][%d] :x: Error! Player02 Tag"
            " should be unique \033[m\n",
            __FUNCTION__,__LINE__);
        return -1;
      }
      pScrollTarget =(ObjAttr_t*)ObjDirectory.m_mapTagObj["Player02"][0];

    } else
      pScrollTarget = nullptr;
  }

  if (pScrollTarget != nullptr && pScrollTarget->pBody != nullptr) {
    b2Vec2 vec2CurPos_M = pScrollTarget->pBody->GetPosition();
    Do_Scroll(vec2CurPos_M,pApp);
  }
#endif // :x: for test
  return 0;
}
