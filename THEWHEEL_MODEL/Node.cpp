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

// TODO: get these from the layout function
// constants for the distance scale vs. activation curve
const REAL SIZE_SCALE = 100.0;
const REAL DIST_SCALE_MIN = 1.0;
const REAL DIST_SCALE_MAX = 1.35;
const REAL ACTIVATION_MIDPOINT = 0.25;

//////////////////////////////////////////////////////////////////////
// CompareLinkWeights
// 
// comparison function for sorting links by weight, descending
//////////////////////////////////////////////////////////////////////
int __cdecl CompareLinkWeights(const void *elem1, const void *elem2)
{
	CNodeLink *pLink1 = *(CNodeLink**) elem1;
	CNodeLink *pLink2 = *(CNodeLink**) elem2;

	return 1000.0 * (pLink2->GetWeight() - pLink1->GetWeight());

}	// CompareLinkWeights


//////////////////////////////////////////////////////////////////////
// Event Firing
//////////////////////////////////////////////////////////////////////
#define NODE_FIRE_CHANGE(TAG) \
	if (m_pSpace) m_pSpace->UpdateAllViews(NULL, TAG, this); 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CNode::CNode
// 
// constructs a CNode object with the given name and description
//////////////////////////////////////////////////////////////////////
CNode::CNode(CSpace *pSpace,
			 const CString& strName, 
			 const CString& strDesc)
	: m_pSpace(pSpace),
		m_pParent(NULL),
		m_strName(strName),
		m_strDescription(strDesc),
		m_pDib(NULL),
		m_pSoundBuffer(NULL),
		m_vPosition(CVectorD<3>(0.0, 0.0, 0.0)),

		m_primaryActivation((REAL) 0.005),		// initialize with a very 
		m_secondaryActivation((REAL) 0.005),	// small activation

		m_pMaxActivator(NULL),
		m_maxDeltaActivation((REAL) 0.0),
		m_bFoundMaxActivator(FALSE),
		m_bSubThreshold(TRUE),

		m_pOptSSV(NULL),

		m_pView(NULL)
{
}	// CNode::CNode


//////////////////////////////////////////////////////////////////////
// CNode::~CNode
// 
// constructs a CNode object with the given name and description
//////////////////////////////////////////////////////////////////////
CNode::~CNode()
{
	// delete the links
	for (int nAt = 0; nAt < m_arrChildren.GetSize(); nAt++)
	{
		delete m_arrChildren[nAt];
	}
	m_arrChildren.RemoveAll();

	// delete the links
	for (nAt = 0; nAt < m_arrLinks.GetSize(); nAt++)
	{
		delete m_arrLinks[nAt];
	}
	m_arrLinks.RemoveAll();

	// delete the DIB, if present
	if (m_pDib)
	{
		delete m_pDib;
	}

	// delete the optSSV, if present
	if (m_pOptSSV)
	{
		delete m_pOptSSV;
	}
}	// CNode::~CNode


//////////////////////////////////////////////////////////////////////
// implements CNode's dynamic serialization
//////////////////////////////////////////////////////////////////////
IMPLEMENT_SERIAL(CNode, CObject, VERSIONABLE_SCHEMA|8);
//		8 -- added optimal SSV
//		7 -- added activation and position
//		6 -- added class description
//		5 -- added URL

//////////////////////////////////////////////////////////////////////
// CNode::GetSpace
// 
// returns a pointer to the space containing the node
//////////////////////////////////////////////////////////////////////
CSpace *CNode::GetSpace()
{
	return m_pSpace;

}	// CNode::GetSpace



//////////////////////////////////////////////////////////////////////
// CNode::GetParent
// 
// returns a pointer to the node's parent
//////////////////////////////////////////////////////////////////////
CNode *CNode::GetParent()
{
	return m_pParent;

}	// CNode::GetParent


//////////////////////////////////////////////////////////////////////
// CNode::GetParent const
// 
// returns a pointer to the node's parent
//////////////////////////////////////////////////////////////////////
const CNode *CNode::GetParent() const
{
	return m_pParent;

}	// CNode::GetParent const


//////////////////////////////////////////////////////////////////////
// CNode::SetParent
// 
// sets a pointer to the node's parent
//////////////////////////////////////////////////////////////////////
void CNode::SetParent(CNode *pParent)
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

			// set the parent as the initial max activator
			m_pMaxActivator = m_pParent;

			// set the position to the parent's
			SetPosition(m_pParent->GetPosition());
		}
	}

}	// CNode::SetParent


//////////////////////////////////////////////////////////////////////
// CNode::GetChildCount
// 
// returns the number of child nodes for this node
//////////////////////////////////////////////////////////////////////
int CNode::GetChildCount() const
{
	return m_arrChildren.GetSize();

}	// CNode::GetChildCount


//////////////////////////////////////////////////////////////////////
// CNode::GetChildAt
// 
// returns the child node at the given index
//////////////////////////////////////////////////////////////////////
CNode *CNode::GetChildAt(int nAt)
{
	return (CNode *) m_arrChildren.GetAt(nAt);

}	// CNode::GetChildAt


//////////////////////////////////////////////////////////////////////
// CNode::GetChildAt const
// 
// returns the child node at the given index
//////////////////////////////////////////////////////////////////////
const CNode *CNode::GetChildAt(int nAt) const
{
	return (CNode *) m_arrChildren.GetAt(nAt);

}	// CNode::GetChildAt


//////////////////////////////////////////////////////////////////////
// CNode::GetName
// 
// returns the node's name
//////////////////////////////////////////////////////////////////////
const CString& CNode::GetName() const
{
	return m_strName;

}	// CNode::GetName


//////////////////////////////////////////////////////////////////////
// CNode::SetName
// 
// sets the name of the node
//////////////////////////////////////////////////////////////////////
void CNode::SetName(const CString& strName)
{
	m_strName = strName;

	// fire change
	NODE_FIRE_CHANGE(EVT_NODE_NAMED_CHANGED);

}	// CNode::SetName


//////////////////////////////////////////////////////////////////////
// CNode::GetDescription
// 
// returns the node's description text
//////////////////////////////////////////////////////////////////////
const CString& CNode::GetDescription() const
{
	return m_strDescription;

}	// CNode::GetDescription


//////////////////////////////////////////////////////////////////////
// CNode::SetDescription
// 
// sets the node's description text
//////////////////////////////////////////////////////////////////////
void CNode::SetDescription(const CString& strDesc)
{
	m_strDescription = strDesc;

	// fire change
	NODE_FIRE_CHANGE(EVT_NODE_DESC_CHANGED);

}	// CNode::SetDescription


//////////////////////////////////////////////////////////////////////
// CNode::GetClass
// 
// the node class
//////////////////////////////////////////////////////////////////////
const CString& CNode::GetClass() const
{
	return m_strClass;

}	// CNode::GetClass


//////////////////////////////////////////////////////////////////////
// CNode::SetClass
// 
// the node class
//////////////////////////////////////////////////////////////////////
void CNode::SetClass(const CString& strClass)
{
	m_strClass = strClass;

	// fire change
	NODE_FIRE_CHANGE(EVT_NODE_CLASS_CHANGED);

}	// CNode::SetClass


//////////////////////////////////////////////////////////////////////
// CNode::GetImageFilename
// 
// returns the name of the node's image file
//////////////////////////////////////////////////////////////////////
const CString& CNode::GetImageFilename() const
{
	return m_strImageFilename;

}	// CNode::GetImageFilename


//////////////////////////////////////////////////////////////////////
// CNode::SetImageFilename
// 
// sets the name of the node's image file
//////////////////////////////////////////////////////////////////////
void CNode::SetImageFilename(const CString& strImageFilename)
{
	m_strImageFilename = strImageFilename;

	// trigger re-loading of image filename
	if (m_pDib != NULL)
	{
		delete m_pDib;
		m_pDib = NULL;
	}

	// fire change
	NODE_FIRE_CHANGE(EVT_NODE_IMAGE_CHANGED);

}	// CNode::SetImageFilename



//////////////////////////////////////////////////////////////////////
// CNode::GetDib
// 
// returns a pointer to the node's image as a CDib.  loads the image
// if it is not already present
//////////////////////////////////////////////////////////////////////
CDib *CNode::GetDib()
{
	// if we have not loaded the image,
	if (m_pDib == NULL && GetImageFilename() != "")
	{
		// create a new DIB
		m_pDib = new CDib();

		// and try to load the image
		if (!m_pDib->Load("./images/" + GetImageFilename()))
		{
			// no luck -- delete the DIB and return NULL
			delete m_pDib;
			m_pDib = NULL;
		}
	}

	return m_pDib;

}	// CNode::GetDib


//////////////////////////////////////////////////////////////////////
// CNode::GetSoundFilename
// 
// returns the name of the node's sound file
//////////////////////////////////////////////////////////////////////
const CString& CNode::GetSoundFilename() const
{
	return m_strSoundFilename;

}	// CNode::GetSoundFilename


//////////////////////////////////////////////////////////////////////
// CNode::SetSoundFilename
// 
// sets the name of the node's sound file
//////////////////////////////////////////////////////////////////////
void CNode::SetSoundFilename(const CString& strSoundFilename)
{
	m_strSoundFilename = strSoundFilename;

}	// CNode::SetImageFilename


//////////////////////////////////////////////////////////////////////
// CNode::GetSoundBuffer
// 
// returns interface to sound buffer, loading if necessary
//////////////////////////////////////////////////////////////////////
LPDIRECTSOUNDBUFFER CNode::GetSoundBuffer()
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
// CNode::GetUrl
// 
// returns the url for this node
//////////////////////////////////////////////////////////////////////
const CString& CNode::GetUrl() const
{
	return m_strUrl;

} // CNode::GetUrl


//////////////////////////////////////////////////////////////////////
// CNode::SetUrl
// 
// sets the url for this node
//////////////////////////////////////////////////////////////////////
void CNode::SetUrl(const CString& strUrl)
{
	m_strUrl = strUrl;

}	// CNode::SetUrl


//////////////////////////////////////////////////////////////////////
// CNode::GetPosition
// 
// returns the position of the node
//////////////////////////////////////////////////////////////////////
const CVectorD<3>& CNode::GetPosition() const
{
	return m_vPosition;

}	// CNode::GetPosition


//////////////////////////////////////////////////////////////////////
// CNode::SetPosition
// 
// sets the position of the node
//////////////////////////////////////////////////////////////////////
void CNode::SetPosition(const CVectorD<3>& vPos, BOOL bFireChange)
{
	m_vPosition = vPos;

	if (bFireChange)
	{
		NODE_FIRE_CHANGE(EVT_NODE_POSITION_CHANGED);
	}

}	// CNode::SetPosition


//////////////////////////////////////////////////////////////////////
// CNode::GetSize
// 
// returns the size of the node (width & height)
//////////////////////////////////////////////////////////////////////
CVectorD<3> CNode::GetSize(REAL activation) const
{
	// compute the desired aspect ratio (maintain the current aspect ratio)
	REAL aspectRatio = 13.0 / 16.0 - 6.0 / 16.0 * (REAL) exp(-6.0f * activation);

	// compute the new width and height from the desired area and the desired
	//		aspect ratio
	CVectorD<3> vSize;
	vSize[0] = (REAL) sqrt(activation / aspectRatio);
	vSize[1] = (REAL) sqrt(activation * aspectRatio);

	// return the computed size
	return vSize;

}	// CNode::GetSize


//////////////////////////////////////////////////////////////////////
// CNode::GetLinkCount
// 
// returns the number of links for this node
//////////////////////////////////////////////////////////////////////
int CNode::GetLinkCount() const
{
	return m_arrLinks.GetSize();

}	// CNode::GetLinkCount


//////////////////////////////////////////////////////////////////////
// CNode::GetLinkAt
// 
// finds a link at the given index position.
//////////////////////////////////////////////////////////////////////
CNodeLink *CNode::GetLinkAt(int nAt)
{
	return (CNodeLink *) m_arrLinks.GetAt(nAt);

}	// CNode::GetLinkAt


//////////////////////////////////////////////////////////////////////
// CNode::GetLinkAt const
// 
// finds a link at the given index position.
//////////////////////////////////////////////////////////////////////
const CNodeLink *CNode::GetLinkAt(int nAt) const
{
	return (const CNodeLink *) m_arrLinks.GetAt(nAt);

}	// CNode::GetLinkAt const


//////////////////////////////////////////////////////////////////////
// CNode::GetLinkTo
// 
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


//////////////////////////////////////////////////////////////////////
// CNode::GetLinkTo const
// 
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


//////////////////////////////////////////////////////////////////////
// CNode::GetLinkWeight
// 
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
	return 0.0f;

}	// CNode::GetLinkWeight


//////////////////////////////////////////////////////////////////////
// CNode::GetLinkGainWeight
// 
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


//////////////////////////////////////////////////////////////////////
// CNode::LinkTo
// 
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
			double otherWeight = pToNode->GetLinkWeight(this);
			pToNode->LinkTo(this, (weight + otherWeight) * 0.5, FALSE);
		}

		// and update the map
		m_mapLinks.SetAt(pToNode, weight);
	}

	// sort the links
	SortLinks();

	// fire change
	NODE_FIRE_CHANGE(EVT_NODE_LINKWGT_CHANGED);

}	// CNode::LinkTo


//////////////////////////////////////////////////////////////////////
// CNode::Unlink
// 
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
	NODE_FIRE_CHANGE(EVT_NODE_LINKWGT_CHANGED);
}

//////////////////////////////////////////////////////////////////////
// CNode::RemoveAllLinks
// 
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
	NODE_FIRE_CHANGE(EVT_NODE_LINKWGT_CHANGED);

}	// CNode::RemoveAllLinks


//////////////////////////////////////////////////////////////////////
// CNode::SortLinks
// 
// sorts the links from greatest to least weight.
//////////////////////////////////////////////////////////////////////
void CNode::SortLinks()
{
	qsort(m_arrLinks.GetData(), m_arrLinks.GetSize(), 
		sizeof(CNodeLink*), CompareLinkWeights);

}	// CNode::SortLinks


//////////////////////////////////////////////////////////////////////
// CNode::GetActivation
// 
// returns the node's activation
//////////////////////////////////////////////////////////////////////
REAL CNode::GetActivation() const
{
	return m_primaryActivation + m_secondaryActivation;

}	// CNode::GetActivation


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
// CNode::GetDescendantCount
// 
// returns the number of descendants
//////////////////////////////////////////////////////////////////////
int CNode::GetDescendantCount() const
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
// CNode::GetMaxActivator
// 
// returns the current maximum activator
//////////////////////////////////////////////////////////////////////
CNode *CNode::GetMaxActivator()
{
	return m_pMaxActivator;

}	// CNode::GetMaxActivator


//////////////////////////////////////////////////////////////////////
// CNode::IsSubThreshold
// 
// return sub-threshold flag
//////////////////////////////////////////////////////////////////////
BOOL CNode::IsSubThreshold()
{
	return m_bSubThreshold;

}	// CNode::IsSubThreshold


//////////////////////////////////////////////////////////////////////
// CNode::SetSubThreshold
// 
// stes flag indicating that this is a sub-threshold node
//////////////////////////////////////////////////////////////////////
void CNode::SetSubThreshold(BOOL bIs)
{	
	m_bSubThreshold = bIs;

}	// CNode::SetSubThreshold


//////////////////////////////////////////////////////////////////////
// CNode::IsPostSuper
// 
// return post-super-threshold flag
//////////////////////////////////////////////////////////////////////
BOOL CNode::IsPostSuper()
{
	return m_bPostSuper;

}	// CNode::IsPostSuper


//////////////////////////////////////////////////////////////////////
// CNode::SetPostSuper
// 
// stes flag indicating that this is a post-super-threshold node
//////////////////////////////////////////////////////////////////////
void CNode::SetPostSuper(BOOL bIs)
{	
	m_bPostSuper = bIs;

}	// CNode::SetPostSuper


//////////////////////////////////////////////////////////////////////
// CNode::GetOptimalStateVector
// 
// optimal state vector (for interpolated layout)
//////////////////////////////////////////////////////////////////////
CSpaceStateVector *CNode::GetOptimalStateVector()
{
	return m_pOptSSV;

}	// CNode::GetMaxActivator


//////////////////////////////////////////////////////////////////////
// CNode::GetView
// 
// returns the view object for this node
//////////////////////////////////////////////////////////////////////
CObject *CNode::GetView()
{
	return m_pView;

}	// CNode::GetView


//////////////////////////////////////////////////////////////////////
// CNode::SetView
// 
// sets the view object for this node
//////////////////////////////////////////////////////////////////////
void CNode::SetView(CObject *pView)
{
	m_pView = pView;

}	// CNode::SetView


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
		ar >> m_strName;
		ar >> m_strDescription;
		ar >> m_strImageFilename;

		if (nSchema >= 5)
		{
			ar >> m_strUrl;
		}

		if (nSchema >= 6)
		{
			ar >> m_strClass;
		}

		if (nSchema >= 7)
		{
			ar >> m_primaryActivation;
			ar >> m_secondaryActivation;
			ar >> m_vPosition;
			ar >> m_pMaxActivator;
			ar >> m_maxDeltaActivation;
		}

		if (nSchema >= 8)
		{
			if (m_pOptSSV)
			{
				delete m_pOptSSV;
			}

			ar >> m_pOptSSV;
		}
	}
	else
	{
		ar << m_strName;
		ar << m_strDescription;
		ar << m_strImageFilename;

		// schema 5
		ar << m_strUrl;

		// schema 6
		ar << m_strClass;

		// schema 7
		ar << m_primaryActivation;
		ar << m_secondaryActivation;
		ar << m_vPosition;
		ar << m_pMaxActivator;
		ar << m_maxDeltaActivation;

		if (nSchema >= 8)
		{
			ar << m_pOptSSV;
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

		// sort the links
		SortLinks();

		// and set up the map
		m_mapLinks.RemoveAll();
		for (nAt = 0; nAt < GetLinkCount(); nAt++)
		{
			m_mapLinks.SetAt(GetLinkAt(nAt)->GetTarget(), 
				GetLinkAt(nAt)->GetWeight());
		}
	}

}	// CNode::Serialize


//////////////////////////////////////////////////////////////////////
// CNode::SetActivation
// 
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
		m_primaryActivation += deltaActivation;

		// update the total activation value
		if (m_pSpace)
		{
			m_pSpace->m_totalPrimaryActivation += deltaActivation;
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
		if (!m_bFoundMaxActivator 

			// or update the max if greater than current max
			|| pActivator->GetActivation() * weight > m_maxDeltaActivation)
		{
			m_pMaxActivator = pActivator;
			m_maxDeltaActivation = pActivator->GetActivation() * weight;

			// set the flag to indicate we have found a max activator 
			//		for this round
			m_bFoundMaxActivator = TRUE;
		}
	}

}	// CNode::SetActivation


//////////////////////////////////////////////////////////////////////
// CNode::PropagateActivation
// 
// propagates the activation of this node through the other nodes
//		in the space
//////////////////////////////////////////////////////////////////////
void CNode::PropagateActivation(REAL scale)
{
	// iterate through the links from highest to lowest activation
	for (int nAt = 0; nAt < GetLinkCount(); nAt++)
	{
		// get the link
		CNodeLink *pLink = GetLinkAt(nAt);

		// skip if the link is a stabilizer
		if (pLink->IsStabilizer())
		{
			continue;
		}

		// stop propagating if its below the link weight threshold
		if (pLink->GetWeight() < PROPAGATE_THRESHOLD_WEIGHT)
		{
			break;
		}

		// only propagate through the link if we have not already done so
		if (!pLink->HasPropagated())
		{
			// mark this link as having propagated
			pLink->SetHasPropagated(TRUE);

			// get the link target
			CNode *pTarget = pLink->GetTarget();

			// compute the distance function
			REAL distance = (pTarget->GetPosition() - GetPosition()).GetLength();
			REAL norm_distance = distance 
				/ (SIZE_SCALE * GetSize(GetActivation()).GetLength());

			// compute the optimal distance
			const REAL MIDWEIGHT = 0.5;
			REAL opt_dist = DIST_SCALE_MIN
				+ (DIST_SCALE_MAX - DIST_SCALE_MIN) 
					* (1.0 - MIDWEIGHT / (MIDWEIGHT + ACTIVATION_MIDPOINT));

			// compute the exponential of the distance
			REAL exp_dist = exp(1.0 * (norm_distance - opt_dist) - 2.0);
			if (_finite(exp_dist))
			{
				// compute the gain and set it
				REAL new_gain = 1.0 - exp_dist / (exp_dist + 1.0);
				pLink->SetGain(new_gain);
			}
			else
			{
				// getting too far away, need more power
				pLink->SetGain(1.0);
			}

			// compute the desired new activation = this activation * weight
			REAL targetActivation = GetActivation() 
				* pLink->GetWeight();

			// perturb the new activation
			targetActivation *= 
				(1.0005f - 0.001f * (REAL) rand() / (REAL) RAND_MAX);

			// the new activation defaults to the original activation
			REAL newActivation = pTarget->GetActivation();

			// now change it if the current target activation is less than the target
			if (pTarget->GetActivation() < targetActivation)
			{
				// compute the new actual activation
				newActivation = pTarget->GetActivation() + 
					(targetActivation - pTarget->GetActivation()) * scale;
			}

			// set the new actual activation
			pTarget->SetActivation(newActivation, this, pLink->GetGainWeight());

			// propagate to linked nodes
			pTarget->PropagateActivation(scale * (REAL) 1.0);
		}
	}

}	// CNode::PropagateActivation


//////////////////////////////////////////////////////////////////////
// CNode::ResetForPropagation
// 
// resets the "hasPropagated" flags on the link weights
//////////////////////////////////////////////////////////////////////
void CNode::ResetForPropagation()
{
	// if a max activator was not found during previous propagation,
	if (!m_bFoundMaxActivator)
	{
		// cut the activation in half
		m_primaryActivation /= 2.0;

		// cut the secondary activation in half as well
		m_secondaryActivation /= 2.0;

		// adjust the space's total count
		if (m_pSpace)
		{
			m_pSpace->m_totalPrimaryActivation -= 
				m_primaryActivation;
			m_pSpace->m_totalSecondaryActivation -= 
				m_secondaryActivation;
		}
	}

	// reset the max delta activation
	m_bFoundMaxActivator = FALSE;

	// reset each of the node's links
	for (int nAt = 0; nAt < GetLinkCount(); nAt++)
	{
		CNodeLink *pLink = GetLinkAt(nAt);
		pLink->SetHasPropagated(FALSE);
	}

	// now do the same for all children
	for (nAt = 0; nAt < GetChildCount(); nAt++)
	{
		// for each child,
		CNode *pChildNode = GetChildAt(nAt);

		// now recursively reset the children
		pChildNode->ResetForPropagation();
	}

}	// CNode::ResetForPropagation

