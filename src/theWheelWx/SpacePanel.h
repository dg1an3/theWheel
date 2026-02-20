//////////////////////////////////////////////////////////////////////
// SpacePanel.h: 2D rendering panel for CSpace
//
// Replaces CSpaceView (DirectX 9) with wxWidgets 2D rendering.
// Draws nodes as filled ellipses with activation-based sizing,
// and links as lines between node centers.
//////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/wx.h>
#include <Space.h>
#include <Node.h>
#include "Spring.h"
#include <vector>
#include <map>

// Forward declaration
class SpaceTreeView;

// Per-node view data for animation
struct NodeViewData
{
    Spring positionSpring;
    Spring1D activationSpring;
    REAL springActivation;

    NodeViewData()
        : springActivation(0.0f)
    {
        positionSpring.SetK(3.0f);
        positionSpring.SetB(10.0f);
        activationSpring.SetOrigin(0.0f);
        activationSpring.SetPosition(0.0f);
    }
};

class SpacePanel : public wxPanel
{
public:
    SpacePanel(wxWindow* parent);

    void SetSpace(CSpace* pSpace);
    CSpace* GetSpace() const { return m_pSpace; }

    void SetTreeView(SpaceTreeView* pTreeView) { m_pTreeView = pTreeView; }

    // Camera control
    void ResetView();

private:
    // Event handlers
    void OnPaint(wxPaintEvent& event);
    void OnTimer(wxTimerEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftDClick(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnMotion(wxMouseEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnSize(wxSizeEvent& event);

    // Rendering helpers
    void DrawNodes(wxDC& dc);
    void DrawLinks(wxDC& dc);
    void DrawNode(wxDC& dc, CNode* pNode, const NodeViewData& viewData);
    void DrawLink(wxDC& dc, CNode* pFrom, CNode* pTo, REAL weight);

    // Coordinate transforms
    wxPoint WorldToScreen(const CVectorD<3>& worldPos) const;
    CVectorD<3> ScreenToWorld(const wxPoint& screenPos) const;

    // Find the node under a screen position
    CNode* HitTestNode(const wxPoint& screenPos);

    // Update spring animations
    void UpdateSprings();

    // Ensure all nodes have view data
    void SyncNodeViewData();

    // Color helpers
    wxColour ActivationColor(REAL activation) const;
    wxColour ClassColor(CNode* pNode) const;

    CSpace* m_pSpace;
    SpaceTreeView* m_pTreeView;
    wxTimer m_timer;

    // Camera
    double m_zoom;
    double m_panX, m_panY;

    // Interaction state
    bool m_dragging;
    bool m_panning;
    CNode* m_pDragNode;
    wxPoint m_lastMousePos;

    // Per-node animation data
    std::map<CNode*, NodeViewData> m_nodeViews;

    static const int TIMER_ID = 100;
    static const int TIMER_MS = 30;

    wxDECLARE_EVENT_TABLE();
};
