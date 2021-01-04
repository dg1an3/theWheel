#pragma once
#include "afx.h"

#include "UnitNode.h"

class SelfOrgMap : public CObject
{
public:
    SelfOrgMap(int nWidth, int nHeight);
    ~SelfOrgMap(void);

    // height and width of reference nodes
    int m_nWidth;
    int m_nHeight;

	// reference nodes of the map
	vector< CUnitNode* > m_arrReferenceNodes;
};
