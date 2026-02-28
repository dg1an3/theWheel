//////////////////////////////////////////////////////////////////////
// test_wx_main.cpp: wxWidgets + Google Test integration
//
// Custom wxApp that initializes the wx event system, runs all
// Google Test tests, then exits.
//////////////////////////////////////////////////////////////////////

#include <wx/wx.h>
#include <gtest/gtest.h>

class TestApp : public wxApp
{
public:
    bool OnInit() override
    {
        wxInitAllImageHandlers();
        return true;
    }
};

wxIMPLEMENT_APP_NO_MAIN(TestApp);

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    // Initialize wxWidgets
    wxApp::SetInstance(new TestApp());
    wxEntryStart(argc, argv);
    wxTheApp->CallOnInit();

    int result = RUN_ALL_TESTS();

    wxTheApp->OnExit();
    wxEntryCleanup();

    return result;
}
