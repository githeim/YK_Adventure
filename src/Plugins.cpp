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

  if (pEvt) {
    // Input Event
    if( pEvt->type == SDL_KEYDOWN )
    {
      switch( pEvt->key.keysym.sym ) {
        case SDLK_LEFT:
          break;

        case SDLK_a:
          pBody->ApplyForceToCenter(b2Vec2(-1000,0), true);
          break;
        case SDLK_d:
          pBody->ApplyForceToCenter(b2Vec2( 1000,0), true);
          break;
        case SDLK_w:
          //pBody->ApplyForceToCenter(b2Vec2( 0,2000), true);
          pBody->ApplyLinearImpulseToCenter(b2Vec2( 0,100),true);
          break;
        case SDLK_s:
          pBody->ApplyForceToCenter(b2Vec2( 0,-2000), true);
          break;
        case SDLK_e:
          pBody->ApplyForceToCenter(b2Vec2( 0,-2000), true);
          pBody->ApplyAngularImpulse(30, true);
          break;



      }
    }
  }

  return 0;

}
