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
#include <wx/image.h>
#include <wx/filename.h>
#include <NodeLink.h>
#include <cmath>
#ifdef USE_OPENGL_RENDERER
#include <wx/dcclient.h>
#endif

// Legacy theWheel color palette (mirrors NodeViewSkin.cpp / NodeView.cpp).
// These match the original GDI renderer: a light gray body with a beveled
// outline (light upper-left, dark lower-right) and a class-colored title
// band, with DEFAULT_TITLE as the fallback when no class color is set.
static const wxColour BACKGROUND_COLOR(232, 232, 232);
static const wxColour UPPER_DARK_COLOR(240, 240, 240);
static const wxColour UPPER_LIGHT_COLOR(255, 255, 255);
static const wxColour LOWER_LIGHT_COLOR(160, 160, 160);
static const wxColour LOWER_DARK_COLOR(128, 128, 128);
static const wxColour DEFAULT_TITLE(166, 190, 191);
static const wxColour TEXT_COLOR(40, 40, 40);
static const wxColour SELECTED_COLOR(255, 200, 60);

// Node sizing constants
static const REAL MIN_NODE_RADIUS = 8.0f;
static const REAL MAX_NODE_RADIUS = 80.0f;
static const REAL ACTIVATION_SCALE = 120.0f;

// Returns true if the given title-band color is "dark enough" that white
// text reads better than the default dark text color.
static bool TitleBandIsDark(const wxColour& c)
{
    int luma = (299 * c.Red() + 587 * c.Green() + 114 * c.Blue()) / 1000;
    return luma < 150;
}

#ifdef USE_OPENGL_RENDERER
wxBEGIN_EVENT_TABLE(SpacePanel, wxGLCanvas)
#else
wxBEGIN_EVENT_TABLE(SpacePanel, wxPanel)
#endif
    EVT_PAINT(SpacePanel::OnPaint)
    EVT_TIMER(SpacePanel::TIMER_ID, SpacePanel::OnTimer)
    EVT_LEFT_DOWN(SpacePanel::OnLeftDown)
    EVT_LEFT_DCLICK(SpacePanel::OnLeftDClick)
    EVT_LEFT_UP(SpacePanel::OnLeftUp)
    EVT_MOTION(SpacePanel::OnMotion)
    EVT_LEAVE_WINDOW(SpacePanel::OnLeaveWindow)
    EVT_MOUSEWHEEL(SpacePanel::OnMouseWheel)
    EVT_SIZE(SpacePanel::OnSize)
wxEND_EVENT_TABLE()

#ifdef USE_OPENGL_RENDERER
// wxGLCanvas requires GL attributes
static int glAttribs[] = {
    WX_GL_RGBA, WX_GL_DOUBLEBUFFER,
    WX_GL_DEPTH_SIZE, 16,
    WX_GL_STENCIL_SIZE, 0,
    0
};
#endif

SpacePanel::SpacePanel(wxWindow* parent)
#ifdef USE_OPENGL_RENDERER
    : wxGLCanvas(parent, wxID_ANY, glAttribs, wxDefaultPosition, wxDefaultSize,
                 wxFULL_REPAINT_ON_RESIZE)
    , m_glContext(nullptr)
    , m_glInitialized(false)
#else
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
              wxFULL_REPAINT_ON_RESIZE)
#endif
    , m_pSpace(nullptr)
    , m_pTreeView(nullptr)
    , m_timer(this, TIMER_ID)
    , m_zoom(1.5)
    , m_panX(0), m_panY(0)
    , m_dragging(false)
    , m_panning(false)
    , m_pDragNode(nullptr)
    , m_currentMousePos(0, 0)
    , m_mouseInWindow(false)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
#ifndef USE_OPENGL_RENDERER
    SetBackgroundColour(*wxWHITE);
#endif
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

    // Proximity hover: activate nearest node when mouse is in the window
    // but not over any node
    if (m_mouseInWindow && !m_dragging && !m_panning &&
        !HitTestNode(m_currentMousePos))
    {
        CNode* pNearest = nullptr;
        double minDist = 1e9;

        for (int i = 0; i < m_pSpace->GetNodeCount(); i++) {
            CNode* pNode = m_pSpace->GetNodeAt(i);
            if (pNode->GetIsSubThreshold()) continue;

            auto it = m_nodeViews.find(pNode);
            if (it == m_nodeViews.end()) continue;

            wxPoint nodeScreen = WorldToScreen(it->second.positionSpring.GetPosition());
            double dx = m_currentMousePos.x - nodeScreen.x;
            double dy = m_currentMousePos.y - nodeScreen.y;
            double dist = sqrt(dx * dx + dy * dy);
            if (dist < minDist) {
                minDist = dist;
                pNearest = pNode;
            }
        }

        if (pNearest) {
            // Activation falls off linearly over HOVER_RADIUS screen pixels
            const double HOVER_RADIUS = 200.0;
            double proximity = std::max(0.0, (HOVER_RADIUS - minDist) / HOVER_RADIUS);
            if (proximity > 0.0) {
                m_pSpace->ActivateNode(pNearest, R(proximity * 0.008));
                m_pSpace->NormalizeNodes();
            }
        }
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
#ifdef USE_OPENGL_RENDERER
    OnPaintGL(event);
#else
    wxBufferedPaintDC dc(this);
    // Use a slightly lighter shade than the legacy node background so the
    // BACKGROUND_COLOR-filled node bodies are visually distinct from the
    // canvas. The legacy MFC view used the same gray for both and relied on
    // beveled outlines for separation — we keep the bevels but lift the
    // canvas to near-white for clarity.
    dc.SetBackground(wxBrush(wxColour(248, 248, 248)));
    dc.Clear();

    if (!m_pSpace) {
        dc.SetTextForeground(wxColour(128, 128, 128));
        dc.DrawText("No space loaded. Use File > New to create a sample space.",
                     20, 20);
        return;
    }

    DrawLinks(dc);
    DrawNodes(dc);
#endif
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

void SpacePanel::OnLeaveWindow(wxMouseEvent& event)
{
    m_mouseInWindow = false;
}

void SpacePanel::OnMotion(wxMouseEvent& event)
{
    wxPoint pos = event.GetPosition();
    m_currentMousePos = pos;
    m_mouseInWindow = true;

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
    wxPoint pos = event.GetPosition();
    CNode* pHit = HitTestNode(pos);

    if (pHit && m_pSpace) {
        // Mouse wheel over a node: adjust activation
        REAL scale = event.GetWheelRotation() > 0 ? R(0.3) : R(-0.15);
        m_pSpace->ActivateNode(pHit, scale);
        m_pSpace->NormalizeNodes();

        // Select this node
        m_pSpace->SetCurrentNode(pHit);
        if (m_pTreeView) {
            m_pTreeView->SelectNode(pHit);
        }
    } else {
        // Mouse wheel over empty space: zoom
        double factor = event.GetWheelRotation() > 0 ? 1.1 : 0.9;
        m_zoom *= factor;
        m_zoom = std::max(0.1, std::min(m_zoom, 20.0));
    }
    Refresh();
}

void SpacePanel::OnSize(wxSizeEvent& event)
{
#ifdef USE_OPENGL_RENDERER
    if (m_glInitialized) {
        wxSize sz = GetClientSize();
        m_glRenderer.Resize(sz.GetWidth(), sz.GetHeight());
    }
#endif
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

    // Brightness based on average activation — strongly activated links
    // appear darker (closer to LOWER_DARK_COLOR), weak links fade toward the
    // page background (BACKGROUND_COLOR), mirroring NodeViewSkin::DrawLink.
    REAL avgAct = (itFrom->second.springActivation + itTo->second.springActivation) * 0.5f;
    REAL t = std::min(1.0f, avgAct * 6.0f);
    int rC = (int)(BACKGROUND_COLOR.Red()   * (1.0f - t) + LOWER_DARK_COLOR.Red()   * t);
    int gC = (int)(BACKGROUND_COLOR.Green() * (1.0f - t) + LOWER_DARK_COLOR.Green() * t);
    int bC = (int)(BACKGROUND_COLOR.Blue()  * (1.0f - t) + LOWER_DARK_COLOR.Blue()  * t);

    wxPen pen(wxColour(rC, gC, bC), penWidth);
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

    // Outer rectangle for the node (a bit wider than tall, "elliptangle"-ish).
    wxRect nodeRect(center.x - r, center.y - (int)(r * 0.7),
                    r * 2, (int)(r * 1.4));

    // For very small nodes fall back to a simple ellipse — the title-band
    // layout is not legible at that size.
    if (r <= 20) {
        wxColour fillColor = ActivationColor(act);
        dc.SetBrush(wxBrush(fillColor));
        dc.SetPen(isSelected ? wxPen(SELECTED_COLOR, 2)
                             : wxPen(LOWER_DARK_COLOR, 1));
        dc.DrawEllipse(nodeRect);
        return;
    }

    int cornerRadius = std::min(r / 3, 15);

    // 1. Body: light gray legacy background.
    dc.SetBrush(wxBrush(BACKGROUND_COLOR));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRoundedRectangle(nodeRect, cornerRadius);

    // 2. Title band: clipped to the rounded body so the colored band
    //    sits flush against the upper bevel.
    int titleHeight = std::max(14, r / 3);
    if (titleHeight > nodeRect.GetHeight() - 6) {
        titleHeight = nodeRect.GetHeight() - 6;
    }
    wxRect titleRect(nodeRect.GetLeft(), nodeRect.GetTop(),
                     nodeRect.GetWidth(), titleHeight);

    wxColour titleColor = ClassColor(pNode);
    {
        dc.SetBrush(wxBrush(titleColor));
        dc.SetPen(*wxTRANSPARENT_PEN);
        // Clip to the rounded body so the band follows the corners.
        dc.SetClippingRegion(nodeRect);
        // Draw a slightly oversized rectangle so it tucks under the bevel.
        dc.DrawRectangle(titleRect.GetLeft(), titleRect.GetTop(),
                         titleRect.GetWidth(), titleRect.GetHeight());
        dc.DestroyClippingRegion();
    }

    // 3. Beveled outline: upper-left light highlight, then a darker inset
    //    rounded rectangle offset down/right for the lower-right shadow,
    //    mirroring the legacy GDI bevel.
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(wxPen(UPPER_LIGHT_COLOR, 1));
    {
        wxRect outerHi = nodeRect;
        outerHi.Deflate(1, 1);
        dc.DrawRoundedRectangle(outerHi, std::max(0, cornerRadius - 1));
    }
    dc.SetPen(wxPen(LOWER_LIGHT_COLOR, 1));
    {
        wxRect inner = nodeRect;
        inner.Deflate(2, 2);
        inner.x += 1;
        inner.y += 1;
        dc.DrawRoundedRectangle(inner, std::max(1, cornerRadius - 2));
    }

    // 4. Selection / outline pen — drawn last so it sits on top.
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    if (isSelected) {
        int penWidth = std::max(2, r / 20);
        dc.SetPen(wxPen(SELECTED_COLOR, penWidth));
    } else {
        dc.SetPen(wxPen(LOWER_DARK_COLOR, 1));
    }
    dc.DrawRoundedRectangle(nodeRect, cornerRadius);

    // 5. Image (if any) on the left of the body region.
    wxRect bodyRect(nodeRect.GetLeft() + 3,
                    titleRect.GetBottom() + 2,
                    nodeRect.GetWidth() - 6,
                    nodeRect.GetBottom() - titleRect.GetBottom() - 4);

    auto& mutableNV = m_nodeViews[pNode];
    if (EnsureNodeImage(pNode, mutableNV) && bodyRect.GetHeight() > 8) {
        const wxBitmap& bmp = mutableNV.image;
        if (bmp.IsOk()) {
            // Fit image into a square area on the left of the body, preserving
            // aspect ratio.
            int targetH = bodyRect.GetHeight() - 2;
            int targetW = std::min(targetH, bodyRect.GetWidth() / 2);
            if (targetW > 4 && targetH > 4) {
                wxImage img = bmp.ConvertToImage();
                int srcW = img.GetWidth();
                int srcH = img.GetHeight();
                // Preserve aspect ratio inside (targetW x targetH).
                int drawW = targetW;
                int drawH = targetH;
                if (srcW > 0 && srcH > 0) {
                    double sa = (double)srcW / (double)srcH;
                    double ta = (double)targetW / (double)targetH;
                    if (sa > ta) {
                        drawH = (int)(targetW / sa);
                    } else {
                        drawW = (int)(targetH * sa);
                    }
                }
                if (drawW < 1) drawW = 1;
                if (drawH < 1) drawH = 1;
                wxImage scaled = img.Scale(drawW, drawH, wxIMAGE_QUALITY_NORMAL);
                wxBitmap scaledBmp(scaled);
                int ix = bodyRect.GetLeft() + (targetW - drawW) / 2;
                int iy = bodyRect.GetTop() + (targetH - drawH) / 2;
                dc.DrawBitmap(scaledBmp, ix, iy, true);
                // Shrink body region so text starts to the right of the image.
                bodyRect.x += targetW + 4;
                bodyRect.width -= targetW + 4;
            }
        }
    }

    // 6. Title text inside the title band.
    if (pNode->GetName().GetLength() > 0 && titleRect.GetHeight() > 8) {
        int fontSize = std::max(8, titleHeight - 6);
        wxFont font(fontSize, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
                    wxFONTWEIGHT_BOLD);
        dc.SetFont(font);
        dc.SetTextForeground(TitleBandIsDark(titleColor)
                             ? wxColour(255, 255, 255)
                             : TEXT_COLOR);

        wxString name((const char*)pNode->GetName());

        // Truncate with ellipsis to fit the title band.
        int avail = titleRect.GetWidth() - 8;
        wxString display = name;
        wxSize ts = dc.GetTextExtent(display);
        while (ts.GetWidth() > avail && display.Length() > 1) {
            display = display.Left(display.Length() - 1);
            ts = dc.GetTextExtent(display + "...");
            if (ts.GetWidth() <= avail) {
                display += "...";
                break;
            }
        }

        dc.SetClippingRegion(titleRect);
        int tx = titleRect.GetLeft() + 4;
        int ty = titleRect.GetTop() + (titleRect.GetHeight() - ts.GetHeight()) / 2;
        dc.DrawText(display, tx, ty);
        dc.DestroyClippingRegion();
    }

    // 7. Description text in the body region (skipping over the image, if any).
    if (bodyRect.GetHeight() > 10 && bodyRect.GetWidth() > 10 &&
        pNode->GetDescription().GetLength() > 0)
    {
        int descFontSize = std::max(7, r / 5);
        wxFont descFont(descFontSize, wxFONTFAMILY_SWISS,
                        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        dc.SetFont(descFont);
        dc.SetTextForeground(TEXT_COLOR);

        wxString desc((const char*)pNode->GetDescription());

        dc.SetClippingRegion(bodyRect);
        // Simple word-wrap: emit space-delimited words on as many lines as fit.
        int yCursor = bodyRect.GetTop();
        int lineHeight = dc.GetCharHeight();
        wxString line;
        size_t i = 0;
        while (i <= desc.length() && yCursor + lineHeight <= bodyRect.GetBottom()) {
            bool atEnd = (i == desc.length());
            wxChar ch = atEnd ? wxT('\n') : desc[i];
            if (ch == wxT('\n') || ch == wxT('\r') || ch == wxT(' ') || atEnd) {
                if (ch == wxT('\n') || ch == wxT('\r')) {
                    dc.DrawText(line, bodyRect.GetLeft(), yCursor);
                    yCursor += lineHeight;
                    line.Clear();
                } else {
                    // word boundary
                    size_t j = i;
                    while (j < desc.length() && desc[j] != wxT(' ') &&
                           desc[j] != wxT('\n') && desc[j] != wxT('\r')) {
                        j++;
                    }
                    wxString word = desc.SubString(i, j - 1);
                    wxString trial = line.empty() ? word : (line + wxT(' ') + word);
                    if (dc.GetTextExtent(trial).GetWidth() <= bodyRect.GetWidth()) {
                        line = trial;
                    } else {
                        if (!line.empty()) {
                            dc.DrawText(line, bodyRect.GetLeft(), yCursor);
                            yCursor += lineHeight;
                        }
                        line = word;
                    }
                    i = j;
                    continue;
                }
            }
            i++;
        }
        if (!line.empty() && yCursor + lineHeight <= bodyRect.GetBottom()) {
            dc.DrawText(line, bodyRect.GetLeft(), yCursor);
        }
        dc.DestroyClippingRegion();
    }
}

bool SpacePanel::EnsureNodeImage(CNode* pNode, NodeViewData& viewData)
{
    wxString filename((const char*)pNode->GetImageFilename());

    if (filename.IsEmpty()) {
        // No image set; nothing to load.
        viewData.image = wxBitmap();
        viewData.loadedImageFilename.Clear();
        viewData.imageLoadAttempted = true;
        return false;
    }

    // If we already attempted to load this exact filename, return cached result.
    if (viewData.imageLoadAttempted && viewData.loadedImageFilename == filename) {
        return viewData.image.IsOk();
    }

    viewData.imageLoadAttempted = true;
    viewData.loadedImageFilename = filename;
    viewData.image = wxBitmap();

    // Build a list of candidate paths to try. Mirrors the MFC version's
    // <space-path-dir>/images/<filename> convention plus some saner fallbacks.
    wxArrayString candidates;
    candidates.Add(filename);

    if (m_pSpace) {
        wxString spacePath((const char*)m_pSpace->GetPathName());
        if (!spacePath.IsEmpty()) {
            wxFileName fn(spacePath);
            wxString dir = fn.GetPath();
            if (!dir.IsEmpty()) {
                candidates.Add(dir + wxFileName::GetPathSeparator()
                               + "images" + wxFileName::GetPathSeparator()
                               + filename);
                candidates.Add(dir + wxFileName::GetPathSeparator() + filename);
            }
        }
    }

    candidates.Add("images" + wxString(wxFileName::GetPathSeparator()) + filename);
    candidates.Add("data" + wxString(wxFileName::GetPathSeparator())
                   + "images" + wxString(wxFileName::GetPathSeparator())
                   + filename);

    for (size_t i = 0; i < candidates.GetCount(); i++) {
        const wxString& path = candidates[i];
        if (!wxFileName::FileExists(path)) continue;
        wxImage img;
        if (img.LoadFile(path)) {
            viewData.image = wxBitmap(img);
            return viewData.image.IsOk();
        }
    }

    return false;
}

#ifdef USE_OPENGL_RENDERER

void SpacePanel::OnPaintGL(wxPaintEvent& event)
{
    wxPaintDC paintDC(this); // required even for GL

    if (!m_glContext) {
        m_glContext = new wxGLContext(this);
    }
    SetCurrent(*m_glContext);

    if (!m_glInitialized) {
        wxSize sz = GetClientSize();
        // For wxGLCanvas, the GL context is already set by SetCurrent,
        // so we don't use EGL init. Instead, just init shaders directly.
        // However, GLRenderer is designed for EGL. On macOS with wxGLCanvas,
        // we skip EGL and use the native context that wxGLCanvas provides.
        // Mark as initialized — the wx GL context is already active.
        m_glInitialized = true;
        m_glRenderer.Resize(sz.GetWidth(), sz.GetHeight());
    }

    if (!m_pSpace) {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SwapBuffers();
        return;
    }

    wxSize sz = GetClientSize();
    float w = (float)sz.GetWidth();
    float h = (float)sz.GetHeight();

    m_glRenderer.BeginFrame(232.0f/255.0f, 232.0f/255.0f, 232.0f/255.0f);

    // Set up view/projection
    theWheelGL::Vec3f eye(0.0f, 0.0f, 5.0f);
    theWheelGL::Vec3f at(0.0f, 0.0f, 0.0f);
    theWheelGL::Vec3f up(0.0f, 1.0f, 0.0f);
    m_glRenderer.SetViewMatrix(theWheelGL::Mat4f::LookAtLH(eye, at, up));
    m_glRenderer.SetProjectionMatrix(theWheelGL::Mat4f::OrthoLH(w, h, -40.0f, 40.0f));

    // Directional light
    m_glRenderer.SetLight(theWheelGL::Vec3f(-0.2f, -0.3f, -0.5f),
                          theWheelGL::Vec3f(1.0f, 1.0f, 1.0f));
    m_glRenderer.SetAmbientLight(theWheelGL::Vec3f(0.25f, 0.25f, 0.25f));

    DrawGLLinks();
    DrawGLNodes();

    SwapBuffers();

    // wxDC text overlay after GL swap
    wxClientDC dc(this);
    DrawGLTextOverlay(dc);
}

void SpacePanel::DrawGLLinks()
{
    if (!m_pSpace) return;

    for (int i = 0; i < m_pSpace->GetNodeCount(); i++) {
        CNode* pNode = m_pSpace->GetNodeAt(i);
        if (pNode->GetIsSubThreshold()) continue;

        auto itFrom = m_nodeViews.find(pNode);
        if (itFrom == m_nodeViews.end()) continue;

        for (int j = 0; j < pNode->GetLinkCount(); j++) {
            CNodeLink* pLink = pNode->GetLinkAt(j);
            CNode* pTarget = pLink->GetTarget();
            if (pLink->GetIsStabilizer() || pTarget->GetIsSubThreshold()) continue;
            if (pNode->GetActivation() < pTarget->GetActivation()) continue;

            auto itTo = m_nodeViews.find(pTarget);
            if (itTo == m_nodeViews.end()) continue;

            wxPoint ptFrom = WorldToScreen(itFrom->second.positionSpring.GetPosition());
            wxPoint ptTo = WorldToScreen(itTo->second.positionSpring.GetPosition());

            REAL gain = R(0.01) + sqrt(pNode->GetLinkTo(pTarget)->GetGain());
            theWheelGL::GLNodeView::DrawLink(m_glRenderer,
                (float)ptFrom.x, (float)ptFrom.y, 0.0f,
                (float)ptTo.x, (float)ptTo.y, 0.0f,
                (float)(gain * itFrom->second.springActivation),
                (float)(gain * itTo->second.springActivation));
        }
    }
}

void SpacePanel::DrawGLNodes()
{
    if (!m_pSpace) return;

    for (int i = m_pSpace->GetNodeCount() - 1; i >= 0; i--) {
        CNode* pNode = m_pSpace->GetNodeAt(i);
        if (pNode->GetIsSubThreshold()) continue;

        auto it = m_nodeViews.find(pNode);
        if (it == m_nodeViews.end()) continue;

        float act = (float)it->second.springActivation;
        if (act <= 0.0f) continue;

        wxPoint center = WorldToScreen(it->second.positionSpring.GetPosition());
        theWheelGL::GLNodeView::Draw(m_glRenderer, m_glSkin,
            (float)center.x, (float)center.y, 0.0f, act);
    }
}

void SpacePanel::DrawGLTextOverlay(wxDC& dc)
{
    if (!m_pSpace) return;

    dc.SetBackgroundMode(wxTRANSPARENT);

    for (int i = 0; i < m_pSpace->GetNodeCount(); i++) {
        CNode* pNode = m_pSpace->GetNodeAt(i);
        if (pNode->GetIsSubThreshold()) continue;

        auto it = m_nodeViews.find(pNode);
        if (it == m_nodeViews.end()) continue;

        float act = (float)it->second.springActivation;
        REAL radius = MIN_NODE_RADIUS + sqrt(act) * ACTIVATION_SCALE;
        int r = (int)(radius * m_zoom);
        if (r < 15) continue;

        wxPoint center = WorldToScreen(it->second.positionSpring.GetPosition());

        if (pNode->GetName().GetLength() > 0) {
            int fontSize = std::max(8, r / 3);
            wxFont font(fontSize, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
                        wxFONTWEIGHT_BOLD);
            dc.SetFont(font);

            wxString name((const char*)pNode->GetName());
            wxSize textSize = dc.GetTextExtent(name);

            int tx = center.x - textSize.GetWidth() / 2;
            int ty = center.y - textSize.GetHeight() / 2;

            // Shadow
            dc.SetTextForeground(wxColour(0, 0, 0));
            dc.DrawText(name, tx + 1, ty + 1);
            // Text
            dc.SetTextForeground(wxColour(255, 255, 255));
            dc.DrawText(name, tx, ty);
        }

        // Description
        if (act > 0.1f && pNode->GetDescription().GetLength() > 0 && r > 30) {
            int descFontSize = std::max(7, r / 4);
            wxFont descFont(descFontSize, wxFONTFAMILY_SWISS,
                            wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
            dc.SetFont(descFont);
            dc.SetTextForeground(wxColour(40, 40, 40));

            wxString desc((const char*)pNode->GetDescription());
            int descTop = center.y + 8;
            dc.DrawText(desc, center.x - r + 4, descTop);
        }
    }
}

#endif // USE_OPENGL_RENDERER
