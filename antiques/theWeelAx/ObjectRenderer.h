#pragma once

#include <vnl/vnl_vector_fixed.h>

namespace theWeel 
{

  /**
   *
   */
  typedef vnl_vector_fixed<double, 2> Point2D;

  /**
   * ObjectRenderer is responsible for rendering the object to the scene.
   * Also will load / save objects to the DB
   */
  class ObjectRenderer
  {
  public:
    /**
     *
     */
    ObjectRenderer(void);
    virtual ~ObjectRenderer(void);

    /**
     *
     */
    const Point2D& GetPosition();
    void SetPosition(const Point2D& point);

    /**
     *
     */
    float GetActivation();
    void SetActivation(float activation);

    /**
     *
     */
    float GetLinkWeight(ObjectRenderer* toRenderer);

    /**
     *
     */
    typedef CAtlMap<int, ObjectRenderer*> MapType;
    typedef CAtlArray<ObjectRenderer*> ArrayType;

    /**
     *
     */
    static void LoadAllFromDB(MapType& map);
    //static void SaveAllToDB(const MapType& map);

    /**
     *
     */
    void LoadLinksFromDB(const MapType& map);
    //void SaveLinksToDB();

  private:
    // object attributes (from db)
    int m_ObjectId;
    CString m_ObjectName;
    CString m_Description;

    // object rendering attributes
    float m_activation;
    Point2D m_position;

    // map of links
    CAtlMap<ObjectRenderer*, float> m_mapLinks;
  };

} // namespace theWeel