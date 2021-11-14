#include"Plugins.h"

int Plug_Player01(CPhysic_World* &pWorld,SDL_Event* &pEvt,double& dbTimeDiff) {
//  static double dbLocalTimeDiff =0;
//  dbLocalTimeDiff += dbTimeDiff;
//  if (dbLocalTimeDiff< 0.1 || pEvt ==nullptr) {
//    return 0;
//  }
//  printf("\033[1;33m[%s][%d] :x: diff %f %f \033[m\n",__FUNCTION__,__LINE__,dbLocalTimeDiff,dbTimeDiff);
//  dbLocalTimeDiff = 0;


  // Find Player 1 Body, it should be one
  b2Body* pBody = pWorld->m_mapTags["Player01"][0];
  b2ContactEdge* pContacts = pBody->GetContactList();
  static  bool bPrevContact = false;
  bool bContact = false;
  if (pContacts ) 
  {
    if (pContacts->contact->IsTouching())
    {
      //printf("\033[1;33m[%s][%d] :x: Touching \033[m\n",__FUNCTION__,__LINE__);
      bContact = true;
      b2Vec2 vecPosA;
      vecPosA = pContacts->contact->GetFixtureA()->GetBody()->GetPosition();
      Dbg_DrawPoint_Scale(vecPosA.x,vecPosA.y);
      b2Vec2 vecPosB;
      vecPosB = pContacts->contact->GetFixtureB()->GetBody()->GetPosition();
      Dbg_DrawPoint_Scale(vecPosB.x,vecPosB.y);
      b2Vec2 normal = pContacts->contact->GetManifold()->localNormal;
      b2Vec2 point =pContacts->contact->GetManifold()->localPoint;
      Dbg_DrawPoint_Scale(vecPosA.x+point.x,vecPosA.y+point.y);
      Dbg_DrawLine_Scale(vecPosA.x+point.x,vecPosA.y+point.y,
          vecPosA.x+point.x-normal.x,vecPosA.y+point.y-normal.y
          );


    }
  }
  else
    bContact = false;
  
  if (bContact != bPrevContact) {
    printf("\033[1;32m[%s][%d] :x: %d --> %d \033[m\n",
        __FUNCTION__,__LINE__,bPrevContact,bContact);


  }
  if (bContact != bPrevContact && bContact == true) {
    printf("\033[1;33m[%s][%d] :x: chk %p\033[m\n",__FUNCTION__,__LINE__,pContacts->contact);
   

    b2WorldManifold *pWorldManifold;
    //pContacts->contact->GetWorldManifold(pWorldManifold);



    pBody->SetLinearDamping(100);
  }
  else 
    pBody->SetLinearDamping(0);
  bPrevContact = bContact;
//  if (pContacts->next) {
//    printf("\033[1;33m[%s][%d] :x: Contact! \033[m\n",__FUNCTION__,__LINE__);
//    bContact = true;
//  }
  int iNumkeys;
  const unsigned char *pState=SDL_GetKeyboardState(&iNumkeys);
  auto vecVelocity = pBody->GetLinearVelocity();
  auto fVelocity = std::sqrt(pow(vecVelocity.x,2) + pow(vecVelocity.y,2));

  if (pEvt) {
    // Input Event
    if( pEvt->type == SDL_KEYDOWN )
    {
      switch( pEvt->key.keysym.sym ) {
        case SDLK_a:
            pBody->SetLinearVelocity(b2Vec2( -6,vecVelocity.y));
          break;
        case SDLK_d:
            pBody->SetLinearVelocity(b2Vec2(  6,vecVelocity.y));
          break;
        case SDLK_w:
//          if (bContact)
//            pBody->ApplyLinearImpulseToCenter(b2Vec2( 0,400),true);
          if (bContact) {
            if (pState[SDL_SCANCODE_A]){
              printf("\033[1;33m[%s][%d] :x: chk %d \033[m\n",__FUNCTION__,__LINE__,pState[SDL_SCANCODE_A]);
            //  pBody->ApplyLinearImpulseToCenter(b2Vec2( -400,0), true);
            pBody->ApplyForceToCenter(b2Vec2(-4000,0), true);
            }
            if (pState[SDL_SCANCODE_D]){
              printf("\033[1;33m[%s][%d] :x: chk %d \033[m\n",__FUNCTION__,__LINE__,pState[SDL_SCANCODE_D]);
            //  pBody->ApplyLinearImpulseToCenter(b2Vec2(  400,0), true);
            pBody->ApplyForceToCenter(b2Vec2( 4000,0), true);
            }
            pBody->ApplyLinearImpulseToCenter(b2Vec2(  0,800), true);
          }
          break;
        case SDLK_s:
          pBody->ApplyForceToCenter(b2Vec2( 0,-2000), true);
          break;
        case SDLK_e:
          pBody->ApplyForceToCenter(b2Vec2( 0,-2000), true);
          pBody->ApplyAngularImpulse(30, true);
          break;
        case SDLK_f:
          pBody->ApplyForceToCenter(b2Vec2( 0,-2000), true);
          break;

        case SDLK_r:
          break;


      }
    }
    if( pEvt->type == SDL_KEYUP ) {
      switch( pEvt->key.keysym.sym ) {
        case SDLK_w:
          break;
//        case SDLK_a:
//          if (bContact)
//            pBody->SetLinearDamping(1000.f);
//          break;
//        case SDLK_d:
//          if (bContact)
//            pBody->SetLinearDamping(1000.f);
//          break;
//          if (bContact)
//            pBody->ApplyLinearImpulseToCenter(b2Vec2( 0,400),true);
//          break;

      }

    }
  }

  return 0;

}
