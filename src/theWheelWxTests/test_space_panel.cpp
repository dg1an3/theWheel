//////////////////////////////////////////////////////////////////////
// test_space_panel.cpp: Tests for SpacePanel event handling and state
//////////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>
#include <wx/wx.h>
#include "SpacePanel.h"
#include "SpaceSerializer.h"
#include <Space.h>
#include <Node.h>

// Test accessor for SpacePanel private members
class SpacePanelTestAccessor
{
public:
    static double GetZoom(const SpacePanel* p) { return p->m_zoom; }
    static double GetPanX(const SpacePanel* p) { return p->m_panX; }
    static double GetPanY(const SpacePanel* p) { return p->m_panY; }
    static bool IsDragging(const SpacePanel* p) { return p->m_dragging; }
    static bool IsPanning(const SpacePanel* p) { return p->m_panning; }
    static const std::map<CNode*, NodeViewData>& GetNodeViews(const SpacePanel* p) { return p->m_nodeViews; }
    static CNode* HitTestNode(SpacePanel* p, const wxPoint& pos) { return p->HitTestNode(pos); }
    static wxPoint WorldToScreen(const SpacePanel* p, const CVectorD<3>& pos) { return p->WorldToScreen(pos); }
    static CVectorD<3> ScreenToWorld(const SpacePanel* p, const wxPoint& pos) { return p->ScreenToWorld(pos); }
};

class SpacePanelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_frame = new wxFrame(nullptr, wxID_ANY, "Test", wxDefaultPosition, wxSize(800, 600));
        m_panel = new SpacePanel(m_frame);
        // Give the panel an explicit size so WorldToScreen works correctly
        m_panel->SetSize(800, 600);
        m_space = new CSpace();
        SpaceSerializer::CreateSampleSpace(m_space);
        m_panel->SetSpace(m_space);
        m_frame->Show(false);
    }

    void TearDown() override
    {
        m_frame->Destroy();
        wxTheApp->ProcessPendingEvents();
        delete m_space;
    }

    // Helper: place a node at a known position, ensure it's visible (not sub-threshold)
    void PlaceNode(CNode* pNode, REAL x, REAL y, REAL activation)
    {
        CVectorD<3> pos;
        pos[0] = x;
        pos[1] = y;
        pos[2] = R(0.0);
        pNode->SetPosition(pos);
        pNode->SetActivation(activation);
        pNode->SetIsSubThreshold(FALSE);
        // Re-sync so springs snap to the new position
        m_panel->SetSpace(m_space);
    }

    wxFrame* m_frame;
    SpacePanel* m_panel;
    CSpace* m_space;
};

TEST_F(SpacePanelTest, TestSpacePanelConstruction)
{
    EXPECT_DOUBLE_EQ(SpacePanelTestAccessor::GetZoom(m_panel), 1.5);
    EXPECT_DOUBLE_EQ(SpacePanelTestAccessor::GetPanX(m_panel), 0.0);
    EXPECT_DOUBLE_EQ(SpacePanelTestAccessor::GetPanY(m_panel), 0.0);
    EXPECT_FALSE(SpacePanelTestAccessor::IsDragging(m_panel));
    EXPECT_FALSE(SpacePanelTestAccessor::IsPanning(m_panel));
    EXPECT_EQ(m_panel->GetSpace(), m_space);
}

TEST_F(SpacePanelTest, TestHitTestNode)
{
    CNode* pNode = m_space->GetNodeAt(1);
    PlaceNode(pNode, R(50.0), R(50.0), R(0.5));

    wxPoint screenPos = SpacePanelTestAccessor::WorldToScreen(m_panel, pNode->GetPosition());
    CNode* pHit = SpacePanelTestAccessor::HitTestNode(m_panel, screenPos);
    EXPECT_EQ(pHit, pNode);
}

TEST_F(SpacePanelTest, TestHitTestMiss)
{
    wxPoint farAway(9999, 9999);
    CNode* pHit = SpacePanelTestAccessor::HitTestNode(m_panel, farAway);
    EXPECT_EQ(pHit, nullptr);
}

TEST_F(SpacePanelTest, TestMouseClickActivatesNode)
{
    CNode* pNode = m_space->GetNodeAt(1);
    PlaceNode(pNode, R(100.0), R(0.0), R(0.3));

    wxPoint screenPos = SpacePanelTestAccessor::WorldToScreen(m_panel, pNode->GetPosition());

    // Inject left-down
    wxMouseEvent clickDown(wxEVT_LEFT_DOWN);
    clickDown.SetPosition(screenPos);
    clickDown.SetEventObject(m_panel);
    m_panel->ProcessWindowEvent(clickDown);

    // Inject left-up to release capture
    wxMouseEvent clickUp(wxEVT_LEFT_UP);
    clickUp.SetPosition(screenPos);
    clickUp.SetEventObject(m_panel);
    m_panel->ProcessWindowEvent(clickUp);

    // Node should be set as current
    EXPECT_EQ(m_space->GetCurrentNode(), pNode);
    // Activation should be positive
    EXPECT_GT(pNode->GetActivation(), 0.0);
}

TEST_F(SpacePanelTest, TestMouseWheelZoom)
{
    double zoomBefore = SpacePanelTestAccessor::GetZoom(m_panel);

    // Inject wheel event on empty space (far from any node)
    wxMouseEvent wheel(wxEVT_MOUSEWHEEL);
    wheel.SetPosition(wxPoint(10, 10));
    wheel.SetEventObject(m_panel);
    wheel.m_wheelRotation = 120;
    wheel.m_wheelDelta = 120;
    m_panel->ProcessWindowEvent(wheel);

    double zoomAfter = SpacePanelTestAccessor::GetZoom(m_panel);
    EXPECT_GT(zoomAfter, zoomBefore);
}

TEST_F(SpacePanelTest, TestMouseWheelActivation)
{
    CNode* pNode = m_space->GetNodeAt(1);
    PlaceNode(pNode, R(0.0), R(0.0), R(0.3));

    double zoomBefore = SpacePanelTestAccessor::GetZoom(m_panel);
    wxPoint screenPos = SpacePanelTestAccessor::WorldToScreen(m_panel, pNode->GetPosition());

    // Inject wheel event over the node
    wxMouseEvent wheel(wxEVT_MOUSEWHEEL);
    wheel.SetPosition(screenPos);
    wheel.SetEventObject(m_panel);
    wheel.m_wheelRotation = 120;
    wheel.m_wheelDelta = 120;
    m_panel->ProcessWindowEvent(wheel);

    // Zoom should NOT have changed (wheel over node = activation, not zoom)
    double zoomAfter = SpacePanelTestAccessor::GetZoom(m_panel);
    EXPECT_DOUBLE_EQ(zoomAfter, zoomBefore);

    // Node should be selected
    EXPECT_EQ(m_space->GetCurrentNode(), pNode);
}

TEST_F(SpacePanelTest, TestPanWithDrag)
{
    double panXBefore = SpacePanelTestAccessor::GetPanX(m_panel);
    double panYBefore = SpacePanelTestAccessor::GetPanY(m_panel);

    // Click on empty space (far corner) to start panning
    wxPoint startPos(10, 10);
    wxMouseEvent down(wxEVT_LEFT_DOWN);
    down.SetPosition(startPos);
    down.SetEventObject(m_panel);
    m_panel->ProcessWindowEvent(down);

    // Move mouse
    wxPoint movePos(50, 50);
    wxMouseEvent motion(wxEVT_MOTION);
    motion.SetPosition(movePos);
    motion.SetEventObject(m_panel);
    motion.m_leftDown = true;
    m_panel->ProcessWindowEvent(motion);

    // Release
    wxMouseEvent up(wxEVT_LEFT_UP);
    up.SetPosition(movePos);
    up.SetEventObject(m_panel);
    m_panel->ProcessWindowEvent(up);

    double panXAfter = SpacePanelTestAccessor::GetPanX(m_panel);
    double panYAfter = SpacePanelTestAccessor::GetPanY(m_panel);

    EXPECT_NE(panXAfter, panXBefore);
    EXPECT_NE(panYAfter, panYBefore);
    EXPECT_DOUBLE_EQ(panXAfter - panXBefore, 40.0);
    EXPECT_DOUBLE_EQ(panYAfter - panYBefore, 40.0);
}

TEST_F(SpacePanelTest, TestResetView)
{
    // Change zoom
    wxMouseEvent wheel(wxEVT_MOUSEWHEEL);
    wheel.SetPosition(wxPoint(10, 10));
    wheel.SetEventObject(m_panel);
    wheel.m_wheelRotation = 120;
    wheel.m_wheelDelta = 120;
    m_panel->ProcessWindowEvent(wheel);

    // Reset
    m_panel->ResetView();

    EXPECT_DOUBLE_EQ(SpacePanelTestAccessor::GetZoom(m_panel), 1.5);
    EXPECT_DOUBLE_EQ(SpacePanelTestAccessor::GetPanX(m_panel), 0.0);
    EXPECT_DOUBLE_EQ(SpacePanelTestAccessor::GetPanY(m_panel), 0.0);
}

TEST_F(SpacePanelTest, TestCoordinateTransforms)
{
    CVectorD<3> original;
    original[0] = R(42.0);
    original[1] = R(-17.0);
    original[2] = R(0.0);

    wxPoint screen = SpacePanelTestAccessor::WorldToScreen(m_panel, original);
    CVectorD<3> roundtrip = SpacePanelTestAccessor::ScreenToWorld(m_panel, screen);

    // Allow integer rounding error (WorldToScreen truncates to int)
    EXPECT_NEAR(roundtrip[0], original[0], 1.0);
    EXPECT_NEAR(roundtrip[1], original[1], 1.0);
    EXPECT_DOUBLE_EQ(roundtrip[2], 0.0);
}
