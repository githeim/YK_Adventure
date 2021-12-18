#ifndef _PLUGINS_H_
#define _PLUGINS_H_ 
#include "CPhysic_World.h"
#include <SDL2/SDL.h>
#include "CPlugin.h"

int Dbg_DrawPoint_Scale(float fX_M,float fY_M); 
int Dbg_DrawLine_Scale(float fAX_M,float fAY_M,
                       float fBX_M,float fBY_M
    );

int Plug_Player01(CPhysic_World* &pWorld,SDL_Event* &pEvt,double& dbTimeDiff,
                  CPlugin* pInstance);
int Plug_Enemy_Ground_Tracker(CPhysic_World* &pWorld,SDL_Event* &pEvt,double& dbTimeDiff,
    CPlugin* pInstance);
int Plug_Enemy_Flyer(CPhysic_World* &pWorld, SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance);
#endif /* ifndef _PLUGINS_H_ */
