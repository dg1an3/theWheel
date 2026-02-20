//////////////////////////////////////////////////////////////////////
// SpacePanel.cpp: 2D rendering panel for CSpace
//
// Implements wxWidgets 2D rendering equivalent to the DrawSkinGDI()
// path from theWheelView/NodeViewSkin.cpp, with spring-based
// animation from theWheelView/Spring.h.
//////////////////////////////////////////////////////////////////////

#include "SpacePanel.h"
#include "SpaceTreeView.h"
#include <wx/dcbuffer.h>
#include <NodeLink.h>
#include <cmath>

// Colors matching the original GDI rendering
static const wxColour BG_COLOR(232, 232, 232);
static const wxColour LINK_COLOR(100, 100, 160);
static const wxColour NODE_BORDER_LIGHT(255, 255, 255);
static const wxColour NODE_BORDER_DARK(128, 128, 128);
static const wxColour TEXT_COLOR(40, 40, 40);
static const wxColour SELECTED_COLOR(255, 200, 60);

// Node sizing constants
static const REAL MIN_NODE_RADIUS = 8.0f;
static const REAL MAX_NODE_RADIUS = 80.0f;
static const REAL ACTIVATION_SCALE = 120.0f;

wxBEGIN_EVENT_TABLE(SpacePanel, wxPanel)
    EVT_PAINT(SpacePanel::OnPaint)
    EVT_TIMER(SpacePanel::TIMER_ID, SpacePanel::OnTimer)
    EVT_LEFT_DOWN(SpacePanel::OnLeftDown)
    EVT_LEFT_DCLICK(SpacePanel::OnLeftDClick)
    EVT_LEFT_UP(SpacePanel::OnLeftUp)
    EVT_MOTION(SpacePanel::OnMotion)
    EVT_MOUSEWHEEL(SpacePanel::OnMouseWheel)
    EVT_SIZE(SpacePanel::OnSize)
wxEND_EVENT_TABLE()

SpacePanel::SpacePanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
              wxFULL_REPAINT_ON_RESIZE)
    , m_pSpace(nullptr)
    , m_pTreeView(nullptr)
    , m_timer(this, TIMER_ID)
    , m_zoom(1.5)
    , m_panX(0), m_panY(0)
    , m_dragging(false)
    , m_panning(false)
    , m_pDragNode(nullptr)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetBackgroundColour(*wxWHITE);
}

void SpacePanel::SetSpace(CSpace* pSpace)
{
    m_pSpace = pSpace;
    m_nodeViews.clear();
    if (m_pSpace) {
        SyncNodeViewData();
        m_timer.Start(TIMER_MS);
    } else {
        m_timer.Stop();
    }
    ResetView();
    Refresh();
}

void SpacePanel::ResetView()
{
    m_zoom = 1.5;
    m_panX = 0;
    m_panY = 0;
}

void SpacePanel::SyncNodeViewData()
{
    if (!m_pSpace) return;

    for (int i = 0; i < m_pSpace->GetNodeCount(); i++) {
        CNode* pNode = m_pSpace->GetNodeAt(i);
        if (m_nodeViews.find(pNode) == m_nodeViews.end()) {
            NodeViewData& nv = m_nodeViews[pNode];
            nv.positionSpring.SetPosition(pNode->GetPosition());
            nv.positionSpring.SetOrigin(pNode->GetPosition());
            nv.activationSpring.SetPosition(pNode->GetActivation());
            nv.activationSpring.SetOrigin(pNode->GetActivation());
            nv.springActivation = pNode->GetActivation();
        }
    }
}

void SpacePanel::UpdateSprings()
{
    if (!m_pSpace) return;

    SyncNodeViewData();

    const REAL dt = R(0.030);

    for (int i = 0; i < m_pSpace->GetNodeCount(); i++) {
        CNode* pNode = m_pSpace->GetNodeAt(i);
        auto it = m_nodeViews.find(pNode);
        if (it == m_nodeViews.end()) continue;
        NodeViewData& nv = it->second;

        // Update position spring target
        nv.positionSpring.SetOrigin(pNode->GetPosition());
        nv.positionSpring.Update(dt, 2);

        // Update activation spring target
        nv.activationSpring.SetOrigin(pNode->GetActivation());
        nv.activationSpring.Update(dt, 2);
        nv.springActivation = nv.activationSpring.GetPosition();
        if (nv.springActivation < 0.0f) nv.springActivation = 0.0f;
    }
}

wxPoint SpacePanel::WorldToScreen(const CVectorD<3>& worldPos) const
{
    wxSize sz = GetClientSize();
    int cx = sz.GetWidth() / 2;
    int cy = sz.GetHeight() / 2;
    return wxPoint(
        cx + (int)(worldPos[0] * m_zoom + m_panX),
        cy + (int)(worldPos[1] * m_zoom + m_panY)
    );
}

CVectorD<3> SpacePanel::ScreenToWorld(const wxPoint& screenPos) const
{
    wxSize sz = GetClientSize();
    int cx = sz.GetWidth() / 2;
    int cy = sz.GetHeight() / 2;
    CVectorD<3> world;
    world[0] = R((screenPos.x - cx - m_panX) / m_zoom);
    world[1] = R((screenPos.y - cy - m_panY) / m_zoom);
    world[2] = R(0.0);
    return world;
}

CNode* SpacePanel::HitTestNode(const wxPoint& screenPos)
{
    if (!m_pSpace) return nullptr;

    // Check nodes in reverse order (top-most first = highest activation)
    for (int i = 0; i < m_pSpace->GetNodeCount(); i++) {
        CNode* pNode = m_pSpace->GetNodeAt(i);
        if (pNode->GetIsSubThreshold()) continue;

        auto it = m_nodeViews.find(pNode);
        if (it == m_nodeViews.end()) continue;

        wxPoint nodeScreen = WorldToScreen(it->second.positionSpring.GetPosition());
        REAL radius = MIN_NODE_RADIUS + sqrt(it->second.springActivation) * ACTIVATION_SCALE;
        radius *= m_zoom;

        double dx = screenPos.x - nodeScreen.x;
        double dy = screenPos.y - nodeScreen.y;
        if (dx * dx + dy * dy <= radius * radius) {
            return pNode;
        }
    }
    return nullptr;
}

wxColour SpacePanel::ActivationColor(REAL activation) const
{
    // Blend from light gray to a warm activation color
    REAL t = std::min(activation * 3.0f, 1.0f);
    int r = (int)(232 + t * (255 - 232));
    int g = (int)(232 - t * (232 - 200));
    int b = (int)(232 - t * (232 - 120));
    return wxColour(r, g, b);
}

wxColour SpacePanel::ClassColor(CNode* pNode) const
{
    if (!m_pSpace || pNode->GetClass().GetLength() == 0) {
        return ActivationColor(pNode->GetActivation());
    }

    auto& colorMap = const_cast<CSpace*>(m_pSpace)->GetClassColorMap();
    auto it = colorMap.find(pNode->GetClass());
    if (it != colorMap.end()) {
        COLORREF cr = it->second;
        return wxColour(cr & 0xFF, (cr >> 8) & 0xFF, (cr >> 16) & 0xFF);
    }

    return ActivationColor(pNode->GetActivation());
}

// Event handlers

void SpacePanel::OnPaint(wxPaintEvent& event)
{
    wxBufferedPaintDC dc(this);
    dc.SetBackground(wxBrush(*wxWHITE));
    dc.Clear();

    if (!m_pSpace) {
        dc.SetTextForeground(wxColour(128, 128, 128));
        dc.DrawText("No space loaded. Use File > New to create a sample space.",
                     20, 20);
        return;
    }

    DrawLinks(dc);
    DrawNodes(dc);
}

void SpacePanel::OnTimer(wxTimerEvent& event)
{
    UpdateSprings();
    Refresh();
}

void SpacePanel::OnLeftDown(wxMouseEvent& event)
{
    SetFocus();
    wxPoint pos = event.GetPosition();

    CNode* pHit = HitTestNode(pos);
    if (pHit) {
        // Activate the clicked node
        m_pSpace->ActivateNode(pHit, R(0.5));
        m_pSpace->NormalizeNodes();

        // Set as current node
        m_pSpace->SetCurrentNode(pHit);
        if (m_pTreeView) {
            m_pTreeView->SelectNode(pHit);
        }

        // Start drag
        m_dragging = true;
        m_pDragNode = pHit;
        m_lastMousePos = pos;
        CaptureMouse();
    } else {
        // Start panning
        m_panning = true;
        m_lastMousePos = pos;
        CaptureMouse();
    }

    Refresh();
}

void SpacePanel::OnLeftDClick(wxMouseEvent& event)
{
    wxPoint pos = event.GetPosition();
    CNode* pHit = HitTestNode(pos);
    if (pHit) {
        // Double-click to maximize activation
        m_pSpace->ActivateNode(pHit, R(1.0));
        m_pSpace->NormalizeNodes();
        Refresh();
    }
}

void SpacePanel::OnLeftUp(wxMouseEvent& event)
{
    if (m_dragging || m_panning) {
        ReleaseMouse();
        m_dragging = false;
        m_panning = false;
        m_pDragNode = nullptr;
    }
}

void SpacePanel::OnMotion(wxMouseEvent& event)
{
    wxPoint pos = event.GetPosition();

    if (m_dragging && m_pDragNode) {
        // Move the node
        CVectorD<3> worldPos = ScreenToWorld(pos);
        m_pDragNode->SetPosition(worldPos);

        // Also update the spring so it snaps to the new position
        auto it = m_nodeViews.find(m_pDragNode);
        if (it != m_nodeViews.end()) {
            it->second.positionSpring.SetPosition(worldPos);
            it->second.positionSpring.SetOrigin(worldPos);
        }
        Refresh();
    } else if (m_panning) {
        m_panX += pos.x - m_lastMousePos.x;
        m_panY += pos.y - m_lastMousePos.y;
        m_lastMousePos = pos;
        Refresh();
    }
}

void SpacePanel::OnMouseWheel(wxMouseEvent& event)
{
    double factor = event.GetWheelRotation() > 0 ? 1.1 : 0.9;
    m_zoom *= factor;
    m_zoom = std::max(0.1, std::min(m_zoom, 20.0));
    Refresh();
}

void SpacePanel::OnSize(wxSizeEvent& event)
{
    Refresh();
    event.Skip();
}

// Drawing

void SpacePanel::DrawLinks(wxDC& dc)
{
    if (!m_pSpace) return;

    for (int i = 0; i < m_pSpace->GetNodeCount(); i++) {
        CNode* pNode = m_pSpace->GetNodeAt(i);
        if (pNode->GetIsSubThreshold()) continue;

        for (int j = 0; j < pNode->GetLinkCount(); j++) {
            CNodeLink* pLink = pNode->GetLinkAt(j);
            CNode* pTarget = pLink->GetTarget();

            if (pLink->GetIsStabilizer()) continue;
            if (pTarget->GetIsSubThreshold()) continue;

            // Only draw each link once (from higher to lower activation)
            if (pNode->GetActivation() < pTarget->GetActivation()) continue;

            DrawLink(dc, pNode, pTarget, pLink->GetWeight());
        }
    }
}

void SpacePanel::DrawLink(wxDC& dc, CNode* pFrom, CNode* pTo, REAL weight)
{
    auto itFrom = m_nodeViews.find(pFrom);
    auto itTo = m_nodeViews.find(pTo);
    if (itFrom == m_nodeViews.end() || itTo == m_nodeViews.end()) return;

    wxPoint ptFrom = WorldToScreen(itFrom->second.positionSpring.GetPosition());
    wxPoint ptTo = WorldToScreen(itTo->second.positionSpring.GetPosition());

    // Line width based on weight
    int penWidth = std::max(1, (int)(weight * 3.0f));

    // Alpha based on average activation
    REAL avgAct = (itFrom->second.springActivation + itTo->second.springActivation) * 0.5f;
    int alpha = std::min(255, (int)(avgAct * 500.0f + 30.0f));

    wxPen pen(wxColour(100, 100, 180, alpha), penWidth);
    dc.SetPen(pen);
    dc.DrawLine(ptFrom, ptTo);
}

void SpacePanel::DrawNodes(wxDC& dc)
{
    if (!m_pSpace) return;

    // Draw in reverse order so highest activation is on top
    for (int i = m_pSpace->GetNodeCount() - 1; i >= 0; i--) {
        CNode* pNode = m_pSpace->GetNodeAt(i);
        if (pNode->GetIsSubThreshold()) continue;

        auto it = m_nodeViews.find(pNode);
        if (it == m_nodeViews.end()) continue;

        DrawNode(dc, pNode, it->second);
    }
}

void SpacePanel::DrawNode(wxDC& dc, CNode* pNode, const NodeViewData& viewData)
{
    wxPoint center = WorldToScreen(viewData.positionSpring.GetPosition());
    REAL act = viewData.springActivation;
    REAL radius = MIN_NODE_RADIUS + sqrt(act) * ACTIVATION_SCALE;
    int r = (int)(radius * m_zoom);

    if (r < 2) return;

    bool isSelected = (m_pSpace && m_pSpace->GetCurrentNode() == pNode);

    // Compute elliptangle-like shape: blend between ellipse and rounded rect
    // For simplicity in 2D, use rounded rectangle for high activation, ellipse for low
    wxRect nodeRect(center.x - r, center.y - (int)(r * 0.7),
                    r * 2, (int)(r * 1.4));

    // Fill color based on activation
    wxColour fillColor = ActivationColor(act);
    dc.SetBrush(wxBrush(fillColor));

    // Border
    if (isSelected) {
        dc.SetPen(wxPen(SELECTED_COLOR, 3));
    } else {
        // Beveled look: light top-left, dark bottom-right
        dc.SetPen(wxPen(NODE_BORDER_LIGHT, 2));
    }

    // Draw the shape
    if (r > 20) {
        // Rounded rectangle for larger nodes (like elliptangle)
        int cornerRadius = std::min(r / 3, 15);
        dc.DrawRoundedRectangle(nodeRect, cornerRadius);

        // Draw a second border for bevel effect
        if (!isSelected) {
            dc.SetPen(wxPen(NODE_BORDER_DARK, 1));
            wxRect innerRect = nodeRect;
            innerRect.Deflate(1);
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.DrawRoundedRectangle(innerRect, cornerRadius - 1);
        }
    } else {
        // Ellipse for smaller nodes
        dc.DrawEllipse(nodeRect);
    }

    // Draw node name
    if (r > 15 && pNode->GetName().GetLength() > 0) {
        dc.SetTextForeground(TEXT_COLOR);

        // Scale font size with node size
        int fontSize = std::max(8, std::min(r / 3, 14));
        wxFont font(fontSize, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        dc.SetFont(font);

        wxString name((const char*)pNode->GetName());
        wxSize textSize = dc.GetTextExtent(name);

        // Center the text in the node
        int tx = center.x - textSize.GetWidth() / 2;
        int ty = center.y - textSize.GetHeight() / 2;

        // Clip to node bounds
        if (textSize.GetWidth() < nodeRect.GetWidth() - 4) {
            dc.DrawText(name, tx, ty);
        } else {
            // Truncate with ellipsis
            wxString truncated = name;
            while (truncated.Length() > 1) {
                truncated = truncated.Left(truncated.Length() - 1);
                wxSize ts = dc.GetTextExtent(truncated + "...");
                if (ts.GetWidth() < nodeRect.GetWidth() - 4) {
                    dc.DrawText(truncated + "...",
                                center.x - ts.GetWidth() / 2, ty);
                    break;
                }
            }
        }
    }
}
