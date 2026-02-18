// test_main.cpp - Custom main() for Google Test with MFC initialization
//
// CSpace constructor calls AfxGetApp()->GetProfileInt() on MSVC,
// so we need a minimal CWinApp-derived object initialized before tests run.

#include "stdafx.h"
#include <gtest/gtest.h>

#ifdef _MSC_VER
// Minimal MFC app state for test harness (same pattern as pythewheel bindings)
class CTestApp : public CWinApp
{
public:
    CTestApp() {}
    virtual BOOL InitInstance() { return TRUE; }
};

// Static instance to initialize MFC
static CTestApp theApp;
#endif

int main(int argc, char** argv)
{
#ifdef _MSC_VER
    // Initialize MFC
    if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
    {
        return 1;
    }
#endif

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
