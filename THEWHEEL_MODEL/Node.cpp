//////////////////////////////////////////////////////////////////////
// Node.cpp: implementation of the CNode class.
//
// Copyright (C) 1999-2003 Derek Graham Lane
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// the class definition
#include "Node.h"

// include the document class
#include "Space.h"

#ifdef INTEL_MATH
// intel math lib
#include <mathf.h>
#include ".\include\node.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// PROPAGATE_THRESHOLD_WEIGHT
// 
// constant defining threshold for propagation
//////////////////////////////////////////////////////////////////////
const REAL PROPAGATE_THRESHOLD_WEIGHT = 0.01;

//////////////////////////////////////////////////////////////////////
// CompareLinkWeights
// 
// comparison function for sorting links by weight, descending
//////////////////////////////////////////////////////////////////////
int __cdecl CompareLinkWeights(const void *elem1, const void *elem2)
{
	CNodeLink *pLink1 = *(CNodeLink**) elem1;
	CNodeLink *pLink2 = *(CNodeLink**) elem2;

	return Round<int>(1000.0 * (pLink2->GetWeight() - pLink1->GetWeight()));

}	// CompareLinkWeights


//////////////////////////////////////////////////////////////////////
// Event Firing
//////////////////////////////////////////////////////////////////////
#define NODE_FIRE_CHANGE() \
	if (m_pSpace) m_pSpace->NodeAttributeChangedEvent.Fire(this);

const REAL PRIM_FRAC = 0.5;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////	
CNode::CNode(CSpace *pSpace,
			 const CString& strName, 
			 const CString& strDesc)
	// constructs a CNode object with the given name and description
	: m_pSpace(pSpace)
		, m_pParent(NULL)

		, m_pDib(NULL)
		, m_hIcon(NULL) 
		, m_pSoundBuffer(NULL)

		, m_primaryActivation((REAL) 0.005)		// initialize with a very 
		, m_secondaryActivation((REAL) 0.005)		// small activation

		, m_pMaxActivator(NULL)
		, m_maxDeltaActivation((REAL) 0.0)
		, m_bFoundMaxActivator(FALSE)

		, m_IsSubThreshold(TRUE)
		, m_IsPostSuper(FALSE)
		, m_PostSuperCount(0)
		, m_bPositionReset(false)

		, m_rmse(0.0)

		, m_pView(NULL)

		, m_nDegSep(UNKNOWN_DEGSEP)
{
	SetName(strName);
	SetDescription(strDesc);

	// TODO: fix this (why is this being initialized?)
	m_vPosition[0] = 0.0;
	m_vPosition[1] = 0.0;
	m_vPosition[2] = 0.0;

}	// CNode::CNode


//////////////////////////////////////////////////////////////////////
CNode::~CNode()		
	// constructs a CNode object with the given name and description
{
	// delete the links
	for (int nAt = 0; nAt < m_arrChildren.GetSize(); nAt++)
	{
		delete m_arrChildren[nAt];
	}
	m_arrChildren.RemoveAll();

	// delete the links
	for (int nAt = 0; nAt < m_arrLinks.GetSize(); nAt++)
	{
		delete m_arrLinks[nAt];
	}
	m_arrLinks.RemoveAll();

	// delete the DIB, if present
	if (m_pDib)
	{
		delete m_pDib;
	}

}	// CNode::~CNode


//////////////////////////////////////////////////////////////////////
IMPLEMENT_SERIAL(CNode, CObject, VERSIONABLE_SCHEMA|8);
// implements CNode's dynamic serialization
//		8 -- added optimal SSV
//		7 -- added activation and position
//		6 -- added class description
//		5 -- added URL


//////////////////////////////////////////////////////////////////////
CSpace *CNode::GetSpace()
	// returns a pointer to the space containing the node
{
	return m_pSpace;

}	// CNode::GetSpace


//////////////////////////////////////////////////////////////////////
void CNode::SetParent(CNode *pParent)
	// sets a pointer to the node's parent
{
	// if this currently has a parent
	if (m_pParent)
	{
		// find this in the children array
		for (int nAt = 0; nAt < m_pParent->m_arrChildren.GetSize(); nAt++)
		{
			if (m_pParent->m_arrChildren[nAt] == this)
			{
				// remove from the current parent's children array
				m_pParent->m_arrChildren.RemoveAt(nAt);

				// don't skip the next one
				nAt--;
			}
		}
	}

	// set the parent pointer
	m_pParent = pParent;

	// if a parent has been set,
	if (m_pParent)
	{
		// set the space, if it hasn't been set yet
		if (NULL == m_pSpace)
		{
			m_pSpace = pParent->m_pSpace;
		}

		// check to ensure the element is not already present
		BOOL bFound = FALSE;
		for (int nAt = 0; nAt < m_arrChildren.GetSize(); nAt++)
		{
			if (m_arrChildren[nAt] == this)
			{
				bFound = TRUE;
			}
		}

		// if not found,
		if (!bFound)
		{
			// add to the children array
			m_pParent->m_arrChildren.Add(this);
		}
	}

}	// CNode::SetParent


//////////////////////////////////////////////////////////////////////
int CNode::GetChildCount() const
	// returns the number of child nodes for this node
{
	return m_arrChildren.GetSize();

}	// CNode::GetChildCount


//////////////////////////////////////////////////////////////////////
CNode *CNode::GetChildAt(int nAt)
	// returns the child node at the given index
{
	return (CNode *) m_arrChildren.GetAt(nAt);

}	// CNode::GetChildAt


//////////////////////////////////////////////////////////////////////
const CNode *CNode::GetChildAt(int nAt) const
	// returns the child node at the given index	
{
	return (CNode *) m_arrChildren.GetAt(nAt);

}	// CNode::GetChildAt


//////////////////////////////////////////////////////////////////////
int CNode::GetDescendantCount() const
	// returns the number of descendants
{
	int nCount = 0;
	for (int nAt = 0; nAt < GetChildCount(); nAt++)
	{
		const CNode *pChild = GetChildAt(nAt);
		nCount += pChild->GetDescendantCount() + 1;		// 1 for the child itself
	}

	return nCount;

}	// CNode::GetDescendantCount


//////////////////////////////////////////////////////////////////////
void CNode::SetName(const CString& strName)
	// sets the name of the node
{
	m_Name = strName;

	// fire change
	NODE_FIRE_CHANGE();

}	// CNode::SetName


//////////////////////////////////////////////////////////////////////
void CNode::SetDescription(const CString& strDesc)
	// sets the node's description text
{
	m_Description = strDesc;

	// fire change
	NODE_FIRE_CHANGE();

}	// CNode::SetDescription


//////////////////////////////////////////////////////////////////////
void CNode::SetClass(const CString& strClass)
	// sets the node class	
{
	m_Class = strClass;

	// fire change
	NODE_FIRE_CHANGE();

}	// CNode::SetClass



//////////////////////////////////////////////////////////////////////
void CNode::SetImageFilename(const CString& strImageFilename)
	// sets the name of the node's image file
{
	m_ImageFilename = strImageFilename;

	// trigger re-loading of image filename
	if (m_pDib != NULL)
	{
		delete m_pDib;
		m_pDib = NULL;
	}

	// fire change
	NODE_FIRE_CHANGE();

}	// CNode::SetImageFilename


//////////////////////////////////////////////////////////////////////
CDib *CNode::GetDib()
	// returns a pointer to the node's image as a CDib.  loads the image
	// if it is not already present
{
	// if we have not loaded the image,
	if (m_pDib == NULL && GetImageFilename() != "")
	{
		// create a new DIB
		m_pDib = new CDib();

		// and try to load the image
		const CString& strPath = GetSpace()->GetPathName();
		int nLastSlash = strPath.ReverseFind('\\');
		CString strImagePath = strPath.Left(nLastSlash);
		if (!m_pDib->Load(strImagePath + "/images/" + GetImageFilename()))
		{
			// no luck -- delete the DIB and return NULL
			delete m_pDib;
			m_pDib = NULL;
		}
	}

	return m_pDib;

}	// CNode::GetDib


//////////////////////////////////////////////////////////////////////
HICON CNode::GetIcon()
	// gets a handle to the node's icon, instead of a DIB
{
	// retrieve if no icon exists and there is a URL
	if (m_hIcon == NULL 
		&& GetUrl() != "")
	{
		// get info on the URL file from the shell
		SHFILEINFO info;
		ZeroMemory(&info, sizeof(info));
		::SHGetFileInfo(GetUrl(), 0, &info, sizeof(info), 
			SHGFI_ICON | SHGFI_LARGEICON);

		// get the icon
		m_hIcon = info.hIcon;
	}

	return m_hIcon;

}	// CNode::GetIcon


//////////////////////////////////////////////////////////////////////
LPDIRECTSOUNDBUFFER CNode::GetSoundBuffer()
	// returns interface to sound buffer, loading if necessary
{
	if (NULL != m_pSoundBuffer)
	{
		return m_pSoundBuffer;
	}

	// structures that hold the file data
	LPWAVEFORMATEX lpWaveFormat = NULL;
	UINT nSize;
	DWORD dwSamples;
	LPBYTE lpWaveData = NULL;

	// the direct sound buffer
	LPVOID lpBuffer = NULL;
	DWORD dwBufSize;

	// load the file
	if (0 != WaveLoadFile(GetSoundFilename(), 
		&nSize, &dwSamples, &lpWaveFormat, &lpWaveData))
	{
		goto CLEANUP;
	}

	// create the sound buffer
	DSBUFFERDESC bufdesc;
	memset(&bufdesc, 0, sizeof(bufdesc));
	bufdesc.dwSize = sizeof(bufdesc);
	bufdesc.dwFlags = DSBCAPS_STATIC;
	bufdesc.dwBufferBytes = __min(nSize, DSBSIZE_MAX);
	bufdesc.lpwfxFormat = lpWaveFormat;
	if (DS_OK != m_pSpace->GetDirectSound()
			->CreateSoundBuffer(&bufdesc, &m_pSoundBuffer, NULL))
	{
		goto CLEANUP;
	}

	// lock the buffer
	if (DS_OK != m_pSoundBuffer->Lock(0, 0, &lpBuffer, &dwBufSize, 
			NULL, NULL, DSBLOCK_ENTIREBUFFER))
	{
		// error, so free the buffer first
		m_pSoundBuffer->Release();
		m_pSoundBuffer = NULL;

		goto CLEANUP;
	}
	ASSERT(dwBufSize == nSize);

	// copy the wave data
	memcpy(lpBuffer, lpWaveData, nSize);

CLEANUP:
	// unlock the sound buffer
	if (NULL != m_pSoundBuffer)
	{
		m_pSoundBuffer->Unlock(lpBuffer, dwBufSize, NULL, 0);
	}

	// free the wave format structure
	if (NULL != lpWaveFormat)
	{
		GlobalFree(lpWaveFormat);
	}

	// free the wave data
	if (NULL != lpWaveData)
	{
		GlobalFree(lpWaveData);
	}

	// return the buffer (or NULL if error)
	return m_pSoundBuffer;

}	// CNode::GetSoundBuffer


//////////////////////////////////////////////////////////////////////
const CVectorD<3>& CNode::GetPosition() const
	// returns the position of the node
{
	return m_vPosition;

}	// CNode::GetPosition


//////////////////////////////////////////////////////////////////////
void CNode::SetPosition(const CVectorD<3>& vPos, bool bResetFlag)
	// sets the position of the node
{
	// TODO: remove RMSE? stuff
	// update RMSE
	m_rmse = (REAL) 0.01 * m_rmse
		+ (REAL) 0.99 * (m_vPosition - vPos).GetLength();

	m_vPosition = vPos;

	if (bResetFlag)
	{
		m_bPositionReset = true;
	}

}	// CNode::SetPosition


//////////////////////////////////////////////////////////////////////
bool CNode::IsPositionReset(bool bClearFlag)
	// returns flag to indicate if position has been reset; optionally
	//		clears flag
{
	bool bFlag = m_bPositionReset;
	if (bClearFlag)
	{
		m_bPositionReset = false;
	}

	return bFlag;

}	// CNode::IsPositionReset


//////////////////////////////////////////////////////////////////////
REAL CNode::GetRMSE()
	// returns positional RMSE
{
	return m_rmse;

}	// CNode::GetRMSE


//////////////////////////////////////////////////////////////////////
CVectorD<3> CNode::GetSize(REAL activation) const
	// returns the size of the node (width & height)
{
	const REAL max = 13.0 / 16.0;
	const REAL scale = 6.0 / 16.0;

	// compute the desired aspect ratio (maintain the current aspect ratio)
	REAL aspectRatio = max - scale 
		* (REAL) exp((REAL) -6.0 * activation);

	// compute the new width and height from the desired area and the desired
	//		aspect ratio
	CVectorD<3> vSize;
	vSize[0] = (REAL) sqrt(activation / aspectRatio);
	vSize[1] = (REAL) sqrt(activation * aspectRatio);

	// return the computed size
	return vSize;

}	// CNode::GetSize


//////////////////////////////////////////////////////////////////////
int CNode::GetLinkCount() const
	// returns the number of links for this node
{
	return m_arrLinks.GetSize();

}	// CNode::GetLinkCount


// finds a link at the given index position.
//////////////////////////////////////////////////////////////////////
CNodeLink *CNode::GetLinkAt(int nAt)
{
	return (CNodeLink *) m_arrLinks.GetAt(nAt);

}	// CNode::GetLinkAt


// finds a link at the given index position.
//////////////////////////////////////////////////////////////////////
const CNodeLink *CNode::GetLinkAt(int nAt) const
{
	return (const CNodeLink *) m_arrLinks.GetAt(nAt);

}	// CNode::GetLinkAt const


// finds a link to the given target, if it exists.
// otherwise returns NULL.
//////////////////////////////////////////////////////////////////////
CNodeLink * CNode::GetLinkTo(CNode * pToNode)
{
	// search through the links,
	for (int nAt = 0; nAt < GetLinkCount(); nAt++)
	{
		// looking for the one with the desired target
		CNodeLink *pLink = GetLinkAt(nAt);
		if (pLink->GetTarget() == pToNode)
		{
			// and return it
			return pLink; 
		}
	}

	// not found?  return NULL
	return NULL;

}	// CNode::GetLinkTo


// finds a link to the given target, if it exists.
// otherwise returns NULL.
//////////////////////////////////////////////////////////////////////
const CNodeLink * CNode::GetLinkTo(CNode * pToNode) const
{
	// search through the links,
	for (int nAt = 0; nAt < GetLinkCount(); nAt++)
	{
		// looking for the one with the desired target
		const CNodeLink *pLink = GetLinkAt(nAt);
		if (pLink->GetTarget() == pToNode)
		{
			// and return it
			return pLink; 
		}
	}

	// not found?  return NULL
	return NULL;

}	// CNode::GetLinkTo const


// finds a link to the given target, if it exists.
// otherwise returns NULL.
//////////////////////////////////////////////////////////////////////
REAL CNode::GetLinkWeight(CNode * pToNode) const
{
	// find the weight in the map
	REAL weight;
	if (m_mapLinks.Lookup(pToNode, weight))
	{	
		// return the weight
		return weight;
	}

	// not found? return 0.0
	return (REAL) 0.0;

}	// CNode::GetLinkWeight


// returns the gained weight for the given link
//////////////////////////////////////////////////////////////////////
REAL CNode::GetLinkGainWeight(CNode *pToNode)
{
	CNodeLink *pLink = GetLinkTo(pToNode);
	if (pLink)
	{
		return pLink->GetGainWeight();
	}

	// not found? return 0.0
	return (REAL) 0.0;

}	// CNode::GetLinkGainWeight


// links the node to the target node (creating a CNodeLink in the
// proces, if necessary).
//////////////////////////////////////////////////////////////////////
void CNode::LinkTo(CNode *pToNode, REAL weight, BOOL bReciprocalLink)
{
	// don't link to NULL
	ASSERT(pToNode != NULL);

	// don't self-link
	if (pToNode == this)
	{
		return;
	}

	// find any existing links
	CNodeLink *pLink = GetLinkTo(pToNode);

	// was a link found
	if (pLink == NULL)
	{
		// create a new node link with the target and weight
		m_arrLinks.Add(new CNodeLink(pToNode, weight));

		// add to the map
		m_mapLinks.SetAt(pToNode, weight);

		// cross-link at the same weight
		if (bReciprocalLink)
		{
			pToNode->LinkTo(this, weight, FALSE);
		}
	}
	else
	{
		// otherwise, just set the weight in one direction
		pLink->SetWeight(weight);

		// cross-link at the same weight
		if (bReciprocalLink)
		{
			REAL otherWeight = pToNode->GetLinkWeight(this);
			pToNode->LinkTo(this, (weight + otherWeight) * (REAL) 0.5, FALSE);
		}

		// and update the map
		m_mapLinks.SetAt(pToNode, weight);
	}

	// sort the links
	SortLinks();

	// fire change
	NODE_FIRE_CHANGE();

}	// CNode::LinkTo


// removes a link
//////////////////////////////////////////////////////////////////////
void CNode::Unlink(CNode *pNode, BOOL bReciprocalLink)
{
	// search through the links,
	for (int nAt = 0; nAt < GetLinkCount(); nAt++)
	{
		// looking for the one with the desired target
		CNodeLink *pLink = (CNodeLink *) m_arrLinks.GetAt(nAt);
		if (pLink->GetTarget() == pNode)
		{
			// and remove from the array
			m_arrLinks.RemoveAt(nAt);

			// delete the link
			delete pLink;

			// break from the loop
			break;
		}
	}

	// remove from the weight map
	m_mapLinks.RemoveKey(pNode);

	// if the reciprocal link should be removed,
	if (bReciprocalLink)
	{
		// then unlink in the other direction
		pNode->Unlink(this, FALSE);
	}

	// fire change
	NODE_FIRE_CHANGE();
}

// removes all the links for the node
//////////////////////////////////////////////////////////////////////
void CNode::RemoveAllLinks()
{
	for (int nAt = 0; nAt < m_arrLinks.GetSize(); nAt++)
	{
		delete m_arrLinks[nAt];
	}
	m_arrLinks.RemoveAll();
	m_mapLinks.RemoveAll();

	// fire change
	NODE_FIRE_CHANGE();

}	// CNode::RemoveAllLinks


// sorts the links from greatest to least weight.
//////////////////////////////////////////////////////////////////////
void CNode::SortLinks()
{
	qsort(m_arrLinks.GetData(), m_arrLinks.GetSize(), 
		sizeof(CNodeLink*), CompareLinkWeights);

}	// CNode::SortLinks


//////////////////////////////////////////////////////////////////////
REAL CNode::GetMaxLinkWeight(void)
	// returns max link weight for this and all child nodes
{
	REAL maxWeight = 0.0;

	// find the max of my link weights
	for (int nAt = 0; nAt < GetLinkCount(); nAt++)
	{
		CNodeLink *pLink = GetLinkAt(nAt);
		maxWeight = __max(maxWeight,
			pLink->GetWeight());
	}

	// now do the same for all children
	for (int nAt = 0; nAt < GetChildCount(); nAt++)
	{
		// for each child,
		CNode *pChildNode = GetChildAt(nAt);

		// recursively compare max to current
		maxWeight = __max(maxWeight,
			pChildNode->GetMaxLinkWeight());
	}

	return maxWeight;
}


//////////////////////////////////////////////////////////////////////
void CNode::ScaleLinkWeights(REAL scale)
	// scales all link weights by scale factor
{
	// scale each of the node's links
	for (int nAt = 0; nAt < GetLinkCount(); nAt++)
	{
		CNodeLink *pLink = GetLinkAt(nAt);
		pLink->SetWeight(pLink->GetWeight()
			* scale);
	}

	// rebuild link map
	m_mapLinks.RemoveAll();
	for (int nAt = 0; nAt < GetLinkCount(); nAt++)
	{
		m_mapLinks.SetAt(GetLinkAt(nAt)->GetTarget(), 
			GetLinkAt(nAt)->GetWeight());
	}

	// now do the same for all children
	for (int nAt = 0; nAt < GetChildCount(); nAt++)
	{
		// for each child,
		CNode *pChildNode = GetChildAt(nAt);

		// now recursively scale the child's link sweight
		pChildNode->ScaleLinkWeights(scale);
	}
}


// returns the node's activation
//////////////////////////////////////////////////////////////////////
REAL CNode::GetActivation() const
{
	return m_primaryActivation + m_secondaryActivation;

}	// CNode::GetActivation


// sets the node's activation.  if an activator is passed, it is
//		compared to the current Max Activator
//////////////////////////////////////////////////////////////////////
void CNode::SetActivation(REAL newActivation, CNode *pActivator, REAL weight)
{
	// compute the delta
	REAL deltaActivation = newActivation - GetActivation();

	// set the activation, based on whether primary or secondary
	if (pActivator == NULL)
	{
		m_primaryActivation += PRIM_FRAC * deltaActivation;
		m_secondaryActivation += (1.0 - PRIM_FRAC) * deltaActivation;

		// update the total activation value
		if (m_pSpace)
		{
			m_pSpace->m_totalPrimaryActivation += PRIM_FRAC * deltaActivation;
			m_pSpace->m_totalSecondaryActivation += (1.0 - PRIM_FRAC) * deltaActivation;
		}
	}
	else
	{
		m_secondaryActivation += deltaActivation;

		// update the total activation value
		if (m_pSpace)
		{
			m_pSpace->m_totalSecondaryActivation += deltaActivation;
		}
	}

	// compare the delta for a new max activator
	if (pActivator != NULL)
	{
		// set the new max if this is the first found
		if ( // !m_bFoundMaxActivator 

			// or update the max if greater than current max
			deltaActivation /* pActivator->GetActivation() * weight */ > m_maxDeltaActivation)
		{
			m_pMaxActivator = pActivator;
			m_maxDeltaActivation = deltaActivation; /* pActivator->GetActivation() * weight */
				// pActivator->GetActivation() * weight;

			// set the flag to indicate we have found a max activator 
			//		for this round
			m_bFoundMaxActivator = TRUE;
		}
	} 

}	// CNode::SetActivation




//////////////////////////////////////////////////////////////////////
// CNode::GetPrimaryActivation
// 
// returns the node's primary activation
//////////////////////////////////////////////////////////////////////
REAL CNode::GetPrimaryActivation() const
{
	return m_primaryActivation;

}	// CNode::GetPrimaryActivation


//////////////////////////////////////////////////////////////////////
// CNode::GetSecondaryActivation
// 
// returns the node's secondary activation
//////////////////////////////////////////////////////////////////////
REAL CNode::GetSecondaryActivation() const
{
	return m_secondaryActivation;

}	// CNode::GetSecondaryActivation


//////////////////////////////////////////////////////////////////////
// CNode::GetMaxActivator
// 
// returns the current maximum activator
//////////////////////////////////////////////////////////////////////
CNode *CNode::GetMaxActivator()
{
	return m_pMaxActivator;

}	// CNode::GetMaxActivator


//////////////////////////////////////////////////////////////////////
// CNode::Serialize
// 
// serialize the node
//////////////////////////////////////////////////////////////////////
void CNode::Serialize(CArchive &ar)
{
	UINT nSchema = ar.GetObjectSchema();

	// serialize the node body
	if (ar.IsLoading())
	{
		ar >> m_Name;
		ar >> m_Description;
		ar >> m_ImageFilename;

		if (nSchema >= 5)
		{
			CString strUrl;
			ar >> strUrl;
			SetUrl(strUrl);
		}

		if (nSchema >= 6)
		{
			ar >> m_Class;
		}

		if (nSchema >= 7)
		{
			double primaryActivation;
			ar >> primaryActivation;
			m_primaryActivation = primaryActivation;

			double secondaryActivation;
			ar >> secondaryActivation;
			m_secondaryActivation = secondaryActivation;

			CVectorD<3, double> vPosition;
			ar >> vPosition;
			m_vPosition[0] = vPosition[0];
			m_vPosition[1] = vPosition[1];
			m_vPosition[2] = vPosition[2];

			ar >> m_pMaxActivator;
			double maxDeltaActivation;
			ar >> maxDeltaActivation;
			m_maxDeltaActivation = maxDeltaActivation;
		}

		if (nSchema >= 8)
		{
//			if (m_pOptSSV)
//			{
//				delete m_pOptSSV;
//			}

			CObject *pOptSSV;
			ar >> pOptSSV;
			delete pOptSSV;
		}
	}
	else
	{
		ar << GetName();
		ar << m_Description;
		ar << m_ImageFilename;

		// schema 5
		ar << GetUrl();

		// schema 6
		ar << m_Class;

		// schema 7
		ar << (double) m_primaryActivation;
		ar << (double) m_secondaryActivation;

		CVectorD<3, double> vPosition;
		vPosition[0] = m_vPosition[0];
		vPosition[1] = m_vPosition[1];
		vPosition[2] = m_vPosition[2];
		ar << vPosition;

		ar << m_pMaxActivator;
		ar << (double) m_maxDeltaActivation;

		if (nSchema >= 8)
		{
			CObject *pOptSSV = NULL;
			ar << pOptSSV;
		}
	}

	// serialize children
	m_arrChildren.Serialize(ar);

	// serialize links
	m_arrLinks.Serialize(ar);

	// if we are loading,
	if (ar.IsLoading())
	{
		// set the children's parent
		for (int nAt = 0; nAt < GetChildCount(); nAt++)
		{
			GetChildAt(nAt)->m_pParent = this;
		}

		// remove zero links
		for (int nAt = GetLinkCount()-1; nAt >= 0; nAt--)
		{
			CNodeLink *pLink = GetLinkAt(nAt);
			if (pLink->GetWeight() < 1e-6)
			{
				m_arrLinks.RemoveAt(nAt);
				delete pLink;
			}
		}

		// sort the links
		SortLinks();

		// and set up the map
		m_mapLinks.RemoveAll();
		for (int nAt = 0; nAt < GetLinkCount(); nAt++)
		{
			m_mapLinks.SetAt(GetLinkAt(nAt)->GetTarget(), 
				GetLinkAt(nAt)->GetWeight());
		}
	}

}	// CNode::Serialize


//////////////////////////////////////////////////////////////////////
// CNode::PropagateActivation
// 
// propagates the activation of this node through the other nodes
//		in the space
//////////////////////////////////////////////////////////////////////
void CNode::PropagateActivation(REAL initScale, REAL alpha)
{
#ifndef PROP_PULL_MODEL

	// TODO: figure how to set m_newActivation for the initial node

	// iterate through the links from highest to lowest activation
	for (int nAt = 0; nAt < GetLinkCount(); nAt++)
	{
		// get the link
		CNodeLink *pLink = GetLinkAt(nAt);

		// stop propagating if its below the link weight threshold
		if (pLink->GetWeight() < PROPAGATE_THRESHOLD_WEIGHT)
		{
			break;
		}

		// only propagate through the link if we have not already done so
		if (!pLink->GetHasPropagated())
		{
			// get the link target
			CNode *pTarget = pLink->GetTarget();

			// compute the desired new activation = this activation * weight
			REAL targetActivation = GetActivation() 
				* pLink->GetWeight();

			// attenuate if the link is a stabilizer
			if (pLink->GetIsStabilizer())
			{
				targetActivation *= (REAL) 0.25;
			}

			// now change it if the current target activation is less than the target
			if (pTarget->GetActivation() < targetActivation)
			{
				// compute the new actual activation
				REAL deltaActivation = 
					(targetActivation - pTarget->GetActivation()) 
						* initScale
						* pLink->GetWeight()		// TODO: check this
						; // * pLink->GetWeight()
						; // * pLink->GetWeight();

				pTarget->m_newActivation += deltaActivation;

				if (!pLink->GetIsStabilizer()
					&& deltaActivation > pTarget->m_maxDeltaActivation)
				{
					pTarget->m_pMaxActivator = this;
					pTarget->m_maxDeltaActivation = deltaActivation;

					// set the flag to indicate we have found a max activator 
					//		for this round
					pTarget->m_bFoundMaxActivator = TRUE;
				}
			}
		}
	}

	// iterate through the links from highest to lowest activation
	for (int nAt = 0; nAt < GetLinkCount(); nAt++)
	{
		// get the link
		CNodeLink *pLink = GetLinkAt(nAt);

		// stop propagating if its below the link weight threshold
		if (pLink->GetWeight() < PROPAGATE_THRESHOLD_WEIGHT)
		{
			break;
		}

		// only propagate through the link if we have not already done so
		if (!pLink->GetHasPropagated())
		{
			// get the link target
			CNode *pTarget = pLink->GetTarget();

			// mark this link as having propagated
			pLink->SetHasPropagated(TRUE);

			// propagate to linked nodes
			pTarget->PropagateActivation(initScale * alpha, alpha);
		}
	}

#else
#error PROP_PULL_MODEL
	// updates activation of node at given degree of separation

	// list of nodes at current nDoS
	CArray<CNode *, CNode *> arrCurrDegSep;
	arrCurrDegSep.RemoveAll();
	arrCurrDegSep.Add(this);

	// list of nodes at > current nDoS
	CArray<CNode *, CNode *> arrNextDegSep;

	// iterate over current DoS nodes, as long as there are some
	REAL scale = initScale;
	for (int nDegSep = 0; arrCurrDegSep.GetSize() > 0; nDegSep++)
	{
		// empty next list
		arrNextDegSep.RemoveAll();

		// iterate over current DegSep nodes
		for (int nAt = 0; nAt < arrCurrDegSep.GetSize(); nAt++)
		{
			// update the activation for the current node
			arrCurrDegSep[nAt]->UpdateActivation(scale, nDegSep, arrNextDegSep);
		}

		// update scale for alpha decay
		scale *= alpha;

		// copy the next DoS list to current
		arrCurrDegSep.Copy(arrNextDegSep);
	} 
#endif
}	// CNode::PropagateActivation


//////////////////////////////////////////////////////////////////////
// CNode::ResetForPropagation
// 
// resets the "hasPropagated" flags on the link weights
//////////////////////////////////////////////////////////////////////
void CNode::ResetForPropagation()
{
/*	// if a max activator was not found during previous propagation,
	if (!m_bFoundMaxActivator)
	{
		// adjust the space's total count
		if (m_pSpace)
		{
			m_pSpace->m_totalPrimaryActivation -= 
				m_primaryActivation * 0.99;
			m_pSpace->m_totalSecondaryActivation -= 
				m_secondaryActivation * 0.99;
		} 

		// cut the activation in half
		m_primaryActivation *= (REAL) 0.99;

		// cut the secondary activation in half as well
		m_secondaryActivation *= (REAL) 0.99;
	}
*/
	// reset each of the node's links
	for (int nAt = 0; nAt < GetLinkCount(); nAt++)
	{
		CNodeLink *pLink = GetLinkAt(nAt);
		pLink->SetHasPropagated(FALSE);
	}

	// reset degrees of separation
	m_nDegSep = UNKNOWN_DEGSEP;

	// reset new activation
	m_newActivation = m_secondaryActivation;

	// reset the max delta activation
	// m_bFoundMaxActivator = FALSE;

	// reset max delta & max activator
	// m_maxDeltaActivation = 0.0;

	// TODO: figure this out (has to do with no max activator in PositionSubNodes
	// m_pMaxActivator = NULL;

	// now do the same for all children
	for (int nAt = 0; nAt < GetChildCount(); nAt++)
	{
		// for each child,
		CNode *pChildNode = GetChildAt(nAt);

		// now recursively reset the children
		pChildNode->ResetForPropagation();
	}

}	// CNode::ResetForPropagation


// transfers new_activation (from Propagate) to current activation for all child nodes
void CNode::UpdateFromNewActivation(void)
{
	// update the total activation value
	if (m_pSpace)
	{
		m_pSpace->m_totalSecondaryActivation += 
			(m_newActivation - m_secondaryActivation);
	}

	// check this
	m_secondaryActivation = m_newActivation;

	// now do the same for all children
	for (int nAt = 0; nAt < GetChildCount(); nAt++)
	{
		// for each child,
		CNode *pChildNode = GetChildAt(nAt);

		// now recursively reset the children
		pChildNode->UpdateFromNewActivation();
	}
}


#ifdef PROP_PULL_MODEL

int CompareTargetActivation(const void *elemL, const void *elemR)
{
	CNodeLink * pLinkL = (CNodeLink *) elemL;
	REAL targetActL = pLinkL->m_targetActivation;

	CNodeLink * pLinkR = (CNodeLink *) elemR;
	REAL targetActR = pLinkR->m_targetActivation;

	return Round<int>(1e+5 * (targetActR - targetActL));
}

// updates activation of node at given degree of separation
void CNode::UpdateActivation(REAL scale, int nDegSep, 
							 CArray<CNode *, CNode *>& arrNextDegSep)
{
	// set my nDoS
	m_nDegSep = nDegSep;

	// list of nodes < my nDoS
	static CNodeLink * arrLessDegSep_Links[255];
	int nLength = 0;
	// arrLessDegSep_Links.RemoveAll();

	// iterate over linked nodes
	for (int nAtLinked = 0; nAtLinked < GetLinkCount(); nAtLinked++)
	{
		CNode *pLinkedTo = GetLinkAt(nAtLinked)->GetTarget();

		// see if the link has not yet set the deg sep
		if (UNKNOWN_DEGSEP == pLinkedTo->m_nDegSep)	
		{
			// add to list of next nDoS
			arrNextDegSep.Add(pLinkedTo);
		}
		// are we < my DoS?
		else if (pLinkedTo->m_nDegSep < m_nDegSep)
		{
			// get the link
			CNodeLink *pLink = pLinkedTo->GetLinkTo(this);
			if (pLink)
			{
				pLink->m_targetActivation =  pLinkedTo->GetActivation() 
					* pLink->GetWeight();
				pLink->m_pFrom = pLinkedTo;

				// add to list
				arrLessDegSep_Links[nLength] = pLink;
				nLength++;
			}
		}
	}

	if (// arrLessDegSep_Links.GetSize() 
			nLength > 0)
	{
		// sort the list by activation
		qsort((void *) arrLessDegSep_Links, 
			(size_t) nLength, // arrLessDegSep_Links.GetSize(), 
				sizeof(CNodeLink *),
			CompareTargetActivation);
	}

	// iterate over sorted list
	for (int nAt = 0; nAt < __min(nLength, 1); // arrLessDegSep_Links.GetSize(), 1); 
		nAt++)
	{
		CNodeLink *pLink = arrLessDegSep_Links[nAt];

		// get target activation for this link
		// REAL targetActivation = pLink->GetTargetActivation();

		// compute the desired new activation = this activation * weight
		REAL targetActivation = pLink->m_pFrom->GetActivation() 
			* pLink->GetWeight();

		// attenuate if the link is a stabilizer
		if (pLink->IsStabilizer())
		{
			targetActivation *= (REAL) 0.25;
		}

		// the new activation defaults to the original activation
		REAL newActivation = GetActivation();

		// now change it if the current target activation is less than the target
		if (GetActivation() < targetActivation)
		{
			// compute the new actual activation
			newActivation = GetActivation() + 
				(targetActivation - GetActivation()) * scale;
		}

		// set the new actual activation
		SetActivation(newActivation, pLink->m_pFrom, pLink->GetGainWeight());
	}

	if (// arrLessDegSep_Links.GetSize() 
		nLength > 0)
	{
		// store max activator
		m_pMaxActivator = arrLessDegSep_Links[nLength-1]->m_pFrom;

		// set the flag to indicate we have found a max activator 
		//		for this round
		m_bFoundMaxActivator = TRUE;
	}
}
#endif // PROP_PULL_MODEL

