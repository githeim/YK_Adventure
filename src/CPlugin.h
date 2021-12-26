#ifndef _CPLUGIN_H_
#define _CPLUGIN_H_

#include <functional>
#include "CPhysic_World.h"
#include <SDL2/SDL.h>
class CPlugin;
class CPlugin {
public:
  b2Body* m_pBody;
  // plugin Callback 
  std::function<int(CPhysic_World* &,SDL_Event* &,double &, CPlugin* )> OnExecute;
  // Used when it created
  std::function<int(CPhysic_World* &,SDL_Event* &,double &, CPlugin* )> OnInit;
  // Used when it destroyed
  std::function<int(CPhysic_World* &,SDL_Event* &,double &, CPlugin* )> OnDeInit;


  // Common values for each instance
  std::map<std::string, float> m_Float_Common;
  std::map<std::string, std::string> m_Str_Common;
  std::map<std::string, int> m_Int_Common;
};


int CreatePlugins_byTagMap(std::map<std::string,std::vector<b2Body*>> &mapTags,
                  std::vector<CPlugin*> &vecPluginInstance);

int CreateGeneralPlugins(std::vector<CPlugin*> &vecPluginInstance);

#endif // end #ifndef _CPLUGIN_H_
