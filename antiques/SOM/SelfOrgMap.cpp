#include "StdAfx.h"
#include "SelfOrgMap.h"

SelfOrgMap::SelfOrgMap(int nWidth, int nHeight)
{
    m_nWidth = nWidth;
    m_nHeight = nHeight;
	m_arrReferenceNodes.resize(nWidth * nHeight);

	// initialize units and positions
	for (int nX = 0; nX < nWidth; nX++)
	{
		for (int nY = 0; nY < nHeight; nY++)
		{
			m_arrReferenceNodes[nX + nY * nWidth] = new CUnitNode();
			m_arrReferenceNodes[nX + nY * nWidth]->SetPosition(CVectorD<2>(nX, nY));
		}
	}

    // initialize the neighbor pointers
	for (int nX = 0; nX < nWidth; nX++)
	{
		for (int nY = 0; nY < nHeight; nY++)
		{
			CUnitNode *pUnit = m_arrReferenceNodes[nX + nY * nWidth];

			pUnit->m_arrNeighbors.push_back(
				m_arrReferenceNodes[(nX + nWidth - 1) % nWidth + nWidth* nY]);
			pUnit->m_arrNeighbors.push_back(
				m_arrReferenceNodes[nX + nWidth * ((nY + nHeight - 1) % nHeight)]);
			pUnit->m_arrNeighbors.push_back(
				m_arrReferenceNodes[(nX + 1) % nWidth + nWidth * nY]);
			pUnit->m_arrNeighbors.push_back(
				m_arrReferenceNodes[nX + nWidth * ((nY + 1) % nHeight)]);
		}
	}
}

SelfOrgMap::~SelfOrgMap(void)
{
}
