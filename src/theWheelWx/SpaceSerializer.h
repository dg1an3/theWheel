//////////////////////////////////////////////////////////////////////
// SpaceSerializer.h: JSON serialization for CSpace
//
// Replaces MFC CArchive serialization with a simple JSON format
// readable on all platforms.
//////////////////////////////////////////////////////////////////////

#pragma once

#include <Space.h>
#include <string>

class SpaceSerializer
{
public:
    // Save a CSpace to a JSON file
    static bool Save(const CSpace* pSpace, const std::string& path);

    // Load a CSpace from a JSON file
    static bool Load(CSpace* pSpace, const std::string& path);

    // Create a sample space for testing
    static void CreateSampleSpace(CSpace* pSpace);
};
