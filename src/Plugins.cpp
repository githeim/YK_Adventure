#include"Plugins.h"


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
  int iCnt=0;
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

int Plug_Player01(CPhysic_World* &pWorld,SDL_Event* &pEvt,double& dbTimeDiff) {
  static double dbLocalTimeDiff =0;
  dbLocalTimeDiff += dbTimeDiff;
  //if (dbLocalTimeDiff< 0.1 || pEvt == nullptr) {
  //  return 0;
  //}
  //printf("\033[1;33m[%s][%d] :x: diff %f %f \033[m\n",__FUNCTION__,__LINE__,dbLocalTimeDiff,dbTimeDiff);
  dbLocalTimeDiff = 0;

   

  // Find Player 1 Body, it should be one
  b2Body* pBody = pWorld->m_mapTags["Player01"][0];
  b2ContactEdge* pContacts = pBody->GetContactList();

  static  bool bPrevIsGround = false;
  bool bIsGround=false; 
  bool bIsFly=false; 
  b2WorldManifold worldManifold;
  int iContactCnt =0;
  auto vecVelocity = pBody->GetLinearVelocity();
  auto fVelocity = std::sqrt(pow(vecVelocity.x,2) + pow(vecVelocity.y,2));

  if (pContacts) {
    do {
      iContactCnt++;
      // Draw A & B position points
      b2Vec2 vecPosA; // ground tile
      vecPosA = pContacts->contact->GetFixtureA()->GetBody()->GetPosition();
      //Dbg_DrawPoint_Scale(vecPosA.x,vecPosA.y);
      b2Vec2 vecPosB; // player
      vecPosB = pContacts->contact->GetFixtureB()->GetBody()->GetPosition();
      Dbg_DrawPoint_Scale(vecPosB.x,vecPosB.y);
      b2Vec2 normal = pContacts->contact->GetManifold()->localNormal;

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
#if 0 // :x: for test

  if (bIsGround)
    printf("\033[1;36m[%s][%d] :x: ground \033[m\n",__FUNCTION__,__LINE__);
  else
    printf("\033[1;32m[%s][%d] :x: Not ground \033[m\n",__FUNCTION__,__LINE__);
#endif // :x: for test

  if (bIsGround == true && bIsGround != bPrevIsGround ) {
    printf("\033[1;31m[%s][%d] :x: Friction! \033[m\n",__FUNCTION__,__LINE__);

    pBody->SetLinearVelocity(b2Vec2( 0,vecVelocity.y));
    //pBody->GetFixtureList()->SetFriction(1000.0f);
  }
//  else
//    pBody->GetFixtureList()->SetFriction(2.0f);

  bPrevIsGround = bIsGround;
  
//  // :x: input timing margin is 0.1sec 
//  static double dbInputTimeDiff =0;
//  dbInputTimeDiff += dbTimeDiff;
//  if (dbInputTimeDiff < 0.1 )
//  {
//    printf("\033[1;33m[%s][%d] :x: chk %lf  %p\033[m\n",__FUNCTION__,__LINE__,dbInputTimeDiff,pEvt);
//
//    return 0;
//  }
//  else {
//    printf("\033[1;32m[%s][%d] :x: chk %lf  %p\033[m\n",__FUNCTION__,__LINE__,dbInputTimeDiff,pEvt);
//  }
//
//  dbInputTimeDiff = 0;
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
            pBody->SetLinearVelocity(b2Vec2( -6,vecVelocity.y));
            break;
          case SDLK_d:
            pBody->SetLinearVelocity(b2Vec2(  6,vecVelocity.y));
            break;
          case SDLK_w:
            if (bIsGround || bIsFly ) 
            {
              if (pState[SDL_SCANCODE_A]){
                printf("\033[1;33m[%s][%d] :x: chk %d \033[m\n",__FUNCTION__,__LINE__,pState[SDL_SCANCODE_A]);
                pBody->ApplyForceToCenter(b2Vec2(-4000,0), true);
              }
              if (pState[SDL_SCANCODE_D]){
                printf("\033[1;33m[%s][%d] :x: chk %d \033[m\n",__FUNCTION__,__LINE__,pState[SDL_SCANCODE_D]);
                pBody->ApplyForceToCenter(b2Vec2( 4000,0), true);
              }
              pBody->ApplyLinearImpulseToCenter(b2Vec2(  0,400), true);
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
