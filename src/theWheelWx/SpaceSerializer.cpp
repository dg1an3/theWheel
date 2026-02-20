//////////////////////////////////////////////////////////////////////
// SpaceSerializer.cpp: JSON serialization for CSpace
//////////////////////////////////////////////////////////////////////

#include "SpaceSerializer.h"
#include <Node.h>
#include <NodeLink.h>
#include <fstream>
#include <sstream>
#include <cstdlib>

// Simple JSON writer (no external dependency)
static void WriteIndent(std::ostream& os, int depth)
{
    for (int i = 0; i < depth; i++) os << "  ";
}

static void WriteString(std::ostream& os, const std::string& s)
{
    os << "\"";
    for (char c : s) {
        if (c == '"') os << "\\\"";
        else if (c == '\\') os << "\\\\";
        else if (c == '\n') os << "\\n";
        else os << c;
    }
    os << "\"";
}

// Recursively write a node and its children
static void WriteNode(std::ostream& os, const CNode* pNode, int depth)
{
    WriteIndent(os, depth);
    os << "{\n";

    WriteIndent(os, depth + 1);
    os << "\"name\": ";
    WriteString(os, (const char*)pNode->GetName());
    os << ",\n";

    WriteIndent(os, depth + 1);
    os << "\"description\": ";
    WriteString(os, (const char*)pNode->GetDescription());
    os << ",\n";

    WriteIndent(os, depth + 1);
    os << "\"class\": ";
    WriteString(os, (const char*)pNode->GetClass());
    os << ",\n";

    const CVectorD<3>& pos = pNode->GetPosition();
    WriteIndent(os, depth + 1);
    os << "\"position\": [" << pos[0] << ", " << pos[1] << ", " << pos[2] << "],\n";

    WriteIndent(os, depth + 1);
    os << "\"activation\": " << pNode->GetActivation() << ",\n";

    // Write links (only to nodes, not back-links)
    WriteIndent(os, depth + 1);
    os << "\"links\": [\n";
    for (int i = 0; i < pNode->GetLinkCount(); i++) {
        CNodeLink* pLink = const_cast<CNode*>(pNode)->GetLinkAt(i);
        WriteIndent(os, depth + 2);
        os << "{ \"target\": ";
        WriteString(os, (const char*)pLink->GetTarget()->GetName());
        os << ", \"weight\": " << pLink->GetWeight();
        os << ", \"gain\": " << pLink->GetGain();
        os << " }";
        if (i < pNode->GetLinkCount() - 1) os << ",";
        os << "\n";
    }
    WriteIndent(os, depth + 1);
    os << "],\n";

    // Write children
    WriteIndent(os, depth + 1);
    os << "\"children\": [\n";
    for (int i = 0; i < const_cast<CNode*>(pNode)->GetChildCount(); i++) {
        WriteNode(os, const_cast<CNode*>(pNode)->GetChildAt(i), depth + 2);
        if (i < const_cast<CNode*>(pNode)->GetChildCount() - 1) os << ",";
        os << "\n";
    }
    WriteIndent(os, depth + 1);
    os << "]\n";

    WriteIndent(os, depth);
    os << "}";
}

bool SpaceSerializer::Save(const CSpace* pSpace, const std::string& path)
{
    std::ofstream file(path);
    if (!file.is_open()) return false;

    file << "{\n";
    file << "  \"version\": 1,\n";
    file << "  \"root\": ";
    if (pSpace->GetRootNode()) {
        WriteNode(file, pSpace->GetRootNode(), 1);
    }
    file << "\n}\n";

    return file.good();
}

// Simple JSON parser helpers
static std::string TrimWhitespace(const std::string& s)
{
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

bool SpaceSerializer::Load(CSpace* pSpace, const std::string& path)
{
    // For now, just return false - full JSON parsing will come later
    // The app will use CreateSampleSpace for initial testing
    (void)pSpace;
    (void)path;
    return false;
}

void SpaceSerializer::CreateSampleSpace(CSpace* pSpace)
{
    pSpace->DeleteContents();
    pSpace->CreateSimpleSpace();

    CNode* pRoot = pSpace->GetRootNode();
    if (!pRoot) return;

    // The CreateSimpleSpace already creates nodes - let's set some positions
    // for a nice initial layout
    srand(42);
    for (int i = 0; i < pSpace->GetNodeCount(); i++) {
        CNode* pNode = pSpace->GetNodeAt(i);
        CVectorD<3> pos;
        REAL angle = R(2.0) * PI * R(i) / R(pSpace->GetNodeCount());
        REAL radius = R(80.0) + R(rand() % 40);
        pos[0] = radius * cos(angle);
        pos[1] = radius * sin(angle);
        pos[2] = R(0.0);
        pNode->SetPosition(pos);
    }
}
