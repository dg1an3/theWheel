#include "StdAfx.h"
#include "ObjectRenderer.h"

namespace theWeel
{

  //////////////////////////////////////////////////////////////////////////////
  ObjectRenderer::ObjectRenderer(void)
  {
  }

  //////////////////////////////////////////////////////////////////////////////
  ObjectRenderer::~ObjectRenderer(void)
  {
  }

  //////////////////////////////////////////////////////////////////////////////
  void 
    ObjectRenderer::LoadAllFromDB(MapType& map)
  {
    ObjectAccess::CObjectAccess access;
    ObjectAccess::Object objectResult;
    access.GetObjectById(0, &objectResult);
    //m_ObjectId = objectResult.ID;
    //m_ObjectName = objectResult.Name;
    //m_Description = objectResult.Description;
  }

  //////////////////////////////////////////////////////////////////////////////
  void 
    ObjectRenderer::LoadLinksFromDB(const MapType& map)
  {
 /*   theWeelLinkDB linkDB;
    linkDB.m_FromObjectId = this->m_ObjectId;
    linkDB.OpenAll();

    HRESULT hr = linkDB.MoveFirst();
    while (hr != E_FAIL)
    {
      ObjectRenderer* renderer = NULL;
      if (map.Lookup(linkDB.m_ToObjectId, renderer))
      {
        m_mapLinks.SetAt(renderer, linkDB.m_Weight);
      }
      hr = linkDB.MoveNext();
    }*/
  }

} // namespace theWeel
