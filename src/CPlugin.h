#ifndef _CPLUGIN_H_
#define _CPLUGIN_H_

#include <functional>
#include "CPhysic_World.h"
#include <SDL2/SDL.h>

class CPlugin {
public:
  CPlugin():m_strObjName("") {} 
  CPlugin(std::string strObjName) : m_strObjName(strObjName){} 
  b2Body* m_pBody;
  // plugin Callback 
  std::function<int(CPhysic_World* &,CObjDirectory &,SDL_Event* &,double &, CPlugin* )> OnExecute;
  // Used when it created
  std::function<int(CPhysic_World* &,CObjDirectory &,SDL_Event* &,double &, CPlugin* )> OnInit;
  // Used when it destroyed
  std::function<int(CPhysic_World* &,CObjDirectory &,SDL_Event* &,double &, CPlugin* )> OnDeInit;

  // the Obj. name in ObjDirectory
  std::string m_strObjName;

  // Common values for each instance
  std::map<std::string, float> m_Float_Common;
  std::map<std::string, std::string> m_Str_Common;
  std::map<std::string, int> m_Int_Common;
  std::map<std::string, void*> m_Ptr_Common;
  std::map<std::string, b2Vec2> m_Vec2_Common;
  std::map<std::string, std::vector<std::string>> m_Str_Vec_Common;
};

int CreatePlugins_byObjDir(CObjDirectory &ObjDirectory);

int CreateGeneralPlugins(CObjDirectory &ObjDirectory); 

#endif // end #ifndef _CPLUGIN_H_
