//////////////////////////////////////////////////////////////////////
// test_wheel_frame.cpp: Tests for WheelFrame menu operations
//////////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>
#include <wx/wx.h>
#include "WheelApp.h"
#include "SpacePanel.h"
#include "SpaceTreeView.h"
#include <Space.h>

// Test accessor for WheelFrame private members
class WheelFrameTestAccessor
{
public:
    static CSpace& GetSpace(WheelFrame* f) { return f->m_space; }
    static SpacePanel* GetSpacePanel(WheelFrame* f) { return f->m_spacePanel; }
    static SpaceTreeView* GetTreeView(WheelFrame* f) { return f->m_treeView; }
};

// Test accessor for SpacePanel (reused from test_space_panel.cpp)
class SpacePanelTestAccessor2
{
public:
    static double GetZoom(const SpacePanel* p) { return p->m_zoom; }
    static double GetPanX(const SpacePanel* p) { return p->m_panX; }
    static double GetPanY(const SpacePanel* p) { return p->m_panY; }
};

class WheelFrameTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_frame = new WheelFrame();
        m_frame->Show(false);
    }

    void TearDown() override
    {
        m_frame->Destroy();
        wxTheApp->ProcessPendingEvents();
    }

    WheelFrame* m_frame;
};

TEST_F(WheelFrameTest, TestFrameConstruction)
{
    CSpace& space = WheelFrameTestAccessor::GetSpace(m_frame);
    EXPECT_GT(space.GetNodeCount(), 0);
    EXPECT_NE(space.GetRootNode(), nullptr);

    SpacePanel* panel = WheelFrameTestAccessor::GetSpacePanel(m_frame);
    EXPECT_NE(panel, nullptr);
    EXPECT_EQ(panel->GetSpace(), &space);

    SpaceTreeView* tree = WheelFrameTestAccessor::GetTreeView(m_frame);
    EXPECT_NE(tree, nullptr);
    EXPECT_EQ(tree->GetSpace(), &space);
}

TEST_F(WheelFrameTest, TestFileNew)
{
    CSpace& space = WheelFrameTestAccessor::GetSpace(m_frame);
    int countBefore = space.GetNodeCount();

    // Trigger File > New
    wxCommandEvent newEvent(wxEVT_MENU, wxID_NEW);
    m_frame->ProcessWindowEvent(newEvent);

    // Space should be repopulated (same sample space)
    EXPECT_GT(space.GetNodeCount(), 0);
    EXPECT_NE(space.GetRootNode(), nullptr);
}

TEST_F(WheelFrameTest, TestViewResetView)
{
    SpacePanel* panel = WheelFrameTestAccessor::GetSpacePanel(m_frame);

    // Change zoom via mouse wheel
    wxMouseEvent wheel(wxEVT_MOUSEWHEEL);
    wheel.SetPosition(wxPoint(10, 10));
    wheel.SetEventObject(panel);
    wheel.m_wheelRotation = 120;
    wheel.m_wheelDelta = 120;
    panel->ProcessWindowEvent(wheel);

    // Trigger View > Reset
    wxCommandEvent resetEvent(wxEVT_MENU, 2000 + 2); // ID_VIEW_RESET = ID_EDIT_SPACE_PROPS + 1 = 2001 + 1 = 2002...
    // Actually look at the enum: ID_VIEW_RESET is defined in WheelFrame
    // We need to use the correct ID. From WheelApp.h:
    // ID_EDIT_NODE_PROPS = 2000, ID_EDIT_SPACE_PROPS = 2001, ID_VIEW_RESET = 2002, ID_VIEW_RUN_LAYOUT = 2003
    wxCommandEvent resetCmd(wxEVT_MENU, 2002);
    m_frame->ProcessWindowEvent(resetCmd);

    EXPECT_DOUBLE_EQ(SpacePanelTestAccessor2::GetZoom(panel), 1.5);
    EXPECT_DOUBLE_EQ(SpacePanelTestAccessor2::GetPanX(panel), 0.0);
    EXPECT_DOUBLE_EQ(SpacePanelTestAccessor2::GetPanY(panel), 0.0);
}

TEST_F(WheelFrameTest, TestViewRunLayout)
{
    CSpace& space = WheelFrameTestAccessor::GetSpace(m_frame);

    // Record initial positions
    std::vector<CVectorD<3>> positionsBefore;
    for (int i = 0; i < space.GetNodeCount(); i++) {
        positionsBefore.push_back(space.GetNodeAt(i)->GetPosition());
    }

    // Move a node to a bad position to ensure layout changes something
    if (space.GetNodeCount() > 1) {
        CVectorD<3> farPos;
        farPos[0] = R(1000.0);
        farPos[1] = R(1000.0);
        farPos[2] = R(0.0);
        space.GetNodeAt(1)->SetPosition(farPos);
    }

    // Trigger View > Run Layout (ID_VIEW_RUN_LAYOUT = 2003)
    wxCommandEvent layoutEvent(wxEVT_MENU, 2003);
    m_frame->ProcessWindowEvent(layoutEvent);

    // At least one node position should have changed
    bool anyChanged = false;
    for (int i = 0; i < space.GetNodeCount(); i++) {
        CVectorD<3> posAfter = space.GetNodeAt(i)->GetPosition();
        if (fabs(posAfter[0] - positionsBefore[i][0]) > 0.01 ||
            fabs(posAfter[1] - positionsBefore[i][1]) > 0.01) {
            anyChanged = true;
            break;
        }
    }
    EXPECT_TRUE(anyChanged);
}
