#include"Plugins.h"
#include "CPlugin.h"

static float GetDistance(float fX1,float fY1, float fX2, float fY2) {
  // Calculating distance
  return std::sqrt(pow(fX2 - fX1, 2) +
      pow(fY2 - fY1, 2) * 1.0);
};



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

int Plug_Player01(CPhysic_World* &pWorld,SDL_Event* &pEvt,double& dbTimeDiff,
    CPlugin* pInstance) {

  // Find Player 1 Body, it should be one
  b2Body* pBody = pInstance->m_pBody;
  b2ContactEdge* pContacts = pBody->GetContactList();

  float fMass = pBody->GetMass();
  float fJumpImpulse = fMass * 7.0;
  float fMoveSpeed = 6.0;



  static  bool bPrevIsGround = false;
  bool bIsGround=false; 
  bool bIsFly=false; 
  b2WorldManifold worldManifold;
  auto vecVelocity = pBody->GetLinearVelocity();

  if (pContacts) {
    do {
      // Draw A & B position points
      b2Vec2 vecPosA; // ground tile
      vecPosA = pContacts->contact->GetFixtureA()->GetBody()->GetPosition();
      //Dbg_DrawPoint_Scale(vecPosA.x,vecPosA.y);
      b2Vec2 vecPosB; // player
      vecPosB = pContacts->contact->GetFixtureB()->GetBody()->GetPosition();
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
  
  if (pEvt) {
    // Input Event
    if( pEvt->type == SDL_KEYDOWN )
    {
      if (
          Prev_Key_Input != pEvt->key.keysym.sym ||
          dbInputTimeDiff > 0.3
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
            pBody->ApplyForceToCenter(b2Vec2( 0,-2000), true);
            break;

          case SDLK_r:
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
 * @param vecOutputBodies
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

/**
 * @brief 
 *
 * @param pWorld
 * @param pEvt
 * @param dbTimeDiff
 * @param pInstance
 *
 * @return 
 */
int Plug_Enemy_Ground_Tracker(CPhysic_World* &pWorld, SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance) {
  std::map<std::string, float> & Float_Common = pInstance->m_Float_Common;
  std::map<std::string, std::string> &Str_Common =pInstance->m_Str_Common;
  std::map<std::string, int> &Int_Common=pInstance->m_Int_Common;
  // character control rate --> 0.5 sec
  float fCtrlRate_SEC = 0.5;
  // character reaction rate --> 0.8 sec
  float fReactionRate_SEC = 0.8;
  // Target Search Range (Meter)
  float fRange_M = 70.f;


  b2Body* pBody = pInstance->m_pBody;
  // Initiate direction
  if (Str_Common.find("Direction_X") == Str_Common.end())
    Str_Common["Direction_X"] = "Stop";
  if (Str_Common.find("Direction_Y") == Str_Common.end())
    Str_Common["Direction_Y"] = "Stop";

  float fMass = pBody->GetMass();
  float fJumpImpulse = fMass * 5.0;
  float fMoveSpeed = 2.0;

  

  auto vecVelocity = pBody->GetLinearVelocity();
  // 기울어지지 않도록 0도로 고정
  pBody->SetTransform(pBody->GetPosition(),0);
  bool bIsGround = IsTouchingGround(pBody);

  Float_Common["InputTimeDiff"] +=dbTimeDiff;
  if (Float_Common["InputTimeDiff"] > fCtrlRate_SEC ) 
  {
    Float_Common["InputTimeDiff"] = 0;

    if (Str_Common["Direction_X"] == "Minus") {
      pBody->SetLinearVelocity(b2Vec2( -fMoveSpeed,vecVelocity.y));
    }
    else if (Str_Common["Direction_X"] == "Plus") {
      pBody->SetLinearVelocity(b2Vec2(  fMoveSpeed,vecVelocity.y));
    }
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

      if ( pTargetBody->GetPosition().x < pBody->GetPosition().x ) {
        Str_Common["Direction_X"] = "Minus";
      } else 
      {
        Str_Common["Direction_X"] = "Plus";
      }

      if ( pTargetBody->GetPosition().y > pBody->GetPosition().y  
          ) {
        Str_Common["Direction_Y"] = "Plus";
      } 

    } 

  }

  return 0;
}

