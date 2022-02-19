#include "ObjAttr.h"
bool CObjDirectory::Find_by_Name(std::string &strObjName, ObjAttr_t* &pObj) {
  if (m_mapObjs.find(strObjName) == m_mapObjs.end()) {
    return false;
  }
  pObj = m_mapObjs[strObjName];
  return true;
}

bool CObjDirectory::Find_by_Tag(std::string &strTag, 
                                std::vector<ObjAttr_t *>& pvecObj) {
  if (m_mapTagObj.find(strTag) == m_mapTagObj.end())
  {
    return false;
  }
  pvecObj = m_mapTagObj[strTag];  
  return true;
}

bool CObjDirectory::Find_by_Body(b2Body* pBody, ObjAttr_t* &pObj) {
  if (m_mapBody_Obj.find(pBody) == m_mapBody_Obj.end()) {
    return false;
  }
  pObj = m_mapBody_Obj[pBody];
  return true;
}

bool CObjDirectory::UpdateDirectory(std::map<std::string,ObjAttr_t*> &mapObjs) {
  m_mapTagObj.clear();
  m_mapLayerObj.clear();
  m_mapBody_Obj.clear();

  for( auto Item : mapObjs) {
    auto pObj = Item.second;
    
    // record tag - obj table
    for (auto strTag : pObj->vecTag) {
        m_mapTagObj[strTag].push_back(pObj);
    }

    // record layer idx table
    m_mapLayerObj[pObj->iLayerIdx].push_back(pObj);

    // record body - obj table
    m_mapBody_Obj[pObj->pBody] = pObj;
  }

  return true;
}

void CObjDirectory::Clear() {
  for (auto item : m_mapObjs) {
    auto pObj = item.second;
    if (pObj) {
      delete pObj;
      pObj = nullptr;
    }
  }
  m_mapObjs.clear();
}

CObjDirectory::~CObjDirectory() {

}
