//////////////////////////////////////////////////////////////////////
// Node.cpp: implementation of the CNode class.
//
// Copyright (C) 1999-2001
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
		m_vPosition(CVector<3>(0.0, 0.0, 0.0)),

		m_primaryActivation((REAL) 0.005),		// initialize with a very 
		m_secondaryActivation((REAL) 0.005),	// small activation
		m_springActivation(m_primaryActivation + m_secondaryActivation),
		m_maxDeltaActivation((REAL) 0.0),

		m_pMaxActivator(NULL),
		m_pView(NULL)
{
}

//////////////////////////////////////////////////////////////////////
// CNode::~CNode
// 
// constructs a CNode object with the given name and description
//////////////////////////////////////////////////////////////////////
CNode::~CNode()
{
	// delete the links
	for (int nAt = 0; nAt < m_arrChildren.GetSize(); nAt++)
		delete m_arrChildren[nAt];
	m_arrChildren.RemoveAll();

	// children.RemoveAll();

	// delete the links
	for (nAt = 0; nAt < m_arrLinks.GetSize(); nAt++)
		delete m_arrLinks[nAt];
	m_arrLinks.RemoveAll();

	// delete the DIB, if present
	delete m_pDib;
}

//////////////////////////////////////////////////////////////////////
// implements CNode's dynamic serialization
//////////////////////////////////////////////////////////////////////
IMPLEMENT_SERIAL(CNode, CObject, VERSIONABLE_SCHEMA|6);
//		6 -- added class description
//		5 -- added URL

//////////////////////////////////////////////////////////////////////
// CNode::GetParent
// 
// returns a pointer to the node's parent
//////////////////////////////////////////////////////////////////////
CNode *CNode::GetParent()
{
	return m_pParent;
}


//////////////////////////////////////////////////////////////////////
// CNode::GetParent const
// 
// returns a pointer to the node's parent
//////////////////////////////////////////////////////////////////////
const CNode *CNode::GetParent() const
{
	return m_pParent;
}


//////////////////////////////////////////////////////////////////////
// CNode::SetParent
// 
// sets a pointer to the node's parent
//////////////////////////////////////////////////////////////////////
void CNode::SetParent(CNode *pParent)
{
	// check to ensure parent is in same space
	// ASSERT(pParent->m_pSpace == m_pSpace);
	if (m_pParent)
	{
		if (m_pSpace == NULL)
			m_pSpace = pParent->m_pSpace;

		for (int nAt = 0; nAt < m_pParent->m_arrChildren.GetSize(); nAt++)
		{
			if (m_pParent->m_arrChildren[nAt] == this)
			{
				m_pParent->m_arrChildren.RemoveAt(nAt);
				nAt--;
			}
		}
	}

	m_pParent = pParent;

	if (m_pParent)
	{
		// check to ensure the element is not already present
		BOOL bFound = FALSE;
		for (int nAt = 0; nAt < m_arrChildren.GetSize(); nAt++)
		{
			if (m_arrChildren[nAt] == this)
			{
				bFound = TRUE;
			}
		}

		if (!bFound)
		{
			m_pParent->m_arrChildren.Add(this);
		}
	}

	// set the parent as the initial max activator
	m_pMaxActivator = m_pParent;

	// set the position to the parent's
	SetPosition(m_pParent->GetPosition());

	// notify views of the change
	if (m_pSpace != NULL)
	{
		m_pSpace->UpdateAllViews(NULL, 0L, this);
	}
}


int CNode::GetChildCount() const
{
	return m_arrChildren.GetSize();
}

CNode *CNode::GetChildAt(int nAt)
{
	return (CNode *) m_arrChildren.GetAt(nAt);
}

const CNode *CNode::GetChildAt(int nAt) const
{
	return (CNode *) m_arrChildren.GetAt(nAt);
}


//////////////////////////////////////////////////////////////////////
// CNode::GetName
// 
// returns the node's name
//////////////////////////////////////////////////////////////////////
const CString& CNode::GetName() const
{
	return m_strName;
}


//////////////////////////////////////////////////////////////////////
// CNode::SetName
// 
// sets the name of the node
//////////////////////////////////////////////////////////////////////
void CNode::SetName(const CString& strName)
{
	m_strName = strName;

	// notify views of the change
	if (m_pSpace != NULL)
		m_pSpace->UpdateAllViews(NULL, 0L, this);
}


//////////////////////////////////////////////////////////////////////
// CNode::GetDescription
// 
// returns the node's description text
//////////////////////////////////////////////////////////////////////
const CString& CNode::GetDescription() const
{
	return m_strDescription;
}


//////////////////////////////////////////////////////////////////////
// CNode::SetDescription
// 
// sets the node's description text
//////////////////////////////////////////////////////////////////////
void CNode::SetDescription(const CString& strDesc)
{
	m_strDescription = strDesc;

	// notify views of the change
	if (m_pSpace != NULL)
		m_pSpace->UpdateAllViews(NULL, 0L, this);
}

// the node class
const CString& CNode::GetClass() const
{
	return m_strClass;
}

void CNode::SetClass(const CString& strClass)
{
	m_strClass = strClass;
}

//////////////////////////////////////////////////////////////////////
// CNode::GetImageFilename
// 
// returns the name of the node's image file
//////////////////////////////////////////////////////////////////////
const CString& CNode::GetImageFilename() const
{
	return m_strImageFilename;
}


//////////////////////////////////////////////////////////////////////
// CNode::SetImageFilename
// 
// sets the name of the node's image file
//////////////////////////////////////////////////////////////////////
void CNode::SetImageFilename(const CString& strImageFilename)
{
	m_strImageFilename = strImageFilename;

	// notify views of the change
	if (m_pSpace != NULL)
		m_pSpace->UpdateAllViews(NULL, 0L, this);
}


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
}


//////////////////////////////////////////////////////////////////////
// CNode::GetSoundFilename
// 
// returns the name of the node's sound file
//////////////////////////////////////////////////////////////////////
const CString& CNode::GetSoundFilename() const
{
	return m_strSoundFilename;
}


//////////////////////////////////////////////////////////////////////
// CNode::SetImageFilename
// 
// sets the name of the node's image file
//////////////////////////////////////////////////////////////////////
void CNode::SetSoundFilename(const CString& strSoundFilename)
{
	m_strSoundFilename = strSoundFilename;
}


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
}


//////////////////////////////////////////////////////////////////////
// CNode::GetUrl
// 
// returns the url for this node
//////////////////////////////////////////////////////////////////////
const CString& CNode::GetUrl() const
{
	return m_strUrl;
}


//////////////////////////////////////////////////////////////////////
// CNode::SetUrl
// 
// sets the url for this node
//////////////////////////////////////////////////////////////////////
void CNode::SetUrl(const CString& strUrl)
{
	m_strUrl = strUrl;
}


//////////////////////////////////////////////////////////////////////
// CNode::GetLinkCount
// 
// returns the number of links for this node
//////////////////////////////////////////////////////////////////////
int CNode::GetLinkCount() const
{
	return m_arrLinks.GetSize();
}


//////////////////////////////////////////////////////////////////////
// CNode::GetLinkAt
// 
// finds a link at the given index position.
//////////////////////////////////////////////////////////////////////
CNodeLink *CNode::GetLinkAt(int nAt)
{
	return (CNodeLink *) m_arrLinks.GetAt(nAt);
}


//////////////////////////////////////////////////////////////////////
// CNode::GetLinkAt const
// 
// finds a link at the given index position.
//////////////////////////////////////////////////////////////////////
const CNodeLink *CNode::GetLinkAt(int nAt) const
{
	return (const CNodeLink *) m_arrLinks.GetAt(nAt);
}


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
}


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
}


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
}


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
			pToNode->LinkTo(this, weight);
	}
	else
	{
		// otherwise, just set the weight in one direction
		pLink->SetWeight(weight);

		// and update the map
		m_mapLinks.SetAt(pToNode, weight);
	}

	// sort the links
	SortLinks();
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
}


//////////////////////////////////////////////////////////////////////
// CNode::SortLinks
// 
// sorts the links from greatest to least weight.
//////////////////////////////////////////////////////////////////////
void CNode::SortLinks()
{
	// flag to indicate a rearrangement has occurred
	BOOL bRearrange;
	do 
	{
		// initially, no rearrangement has occurred
		bRearrange = FALSE;

		// for each link,
		for (int nAt = 0; nAt < GetLinkCount()-1; nAt++)
		{
			// get the current and next links
			CNodeLink *pThisLink = GetLinkAt(nAt);
			CNodeLink *pNextLink = GetLinkAt(nAt+1);

			// compare their weights
			if (pThisLink->GetWeight() < pNextLink->GetWeight())
			{
				// if first is less than second, swap
				m_arrLinks.SetAt(nAt, pNextLink);
				m_arrLinks.SetAt(nAt+1, pThisLink);

				// a rearrangement has occurred
				bRearrange = TRUE;
			}
		}

	// continue as long as rearrangements occur
	} while (bRearrange);

}


//////////////////////////////////////////////////////////////////////
// CNode::LearnFromNode
// 
// applies a simple learning rule to the link weights
//////////////////////////////////////////////////////////////////////
void CNode::LearnFromNode(CNode *pOtherNode, REAL k)
{
	REAL otherAct = (REAL) pOtherNode->GetPrimaryActivation();
	REAL weightTo = (REAL) pOtherNode->GetLinkWeight(this);

	if (otherAct > GetPrimaryActivation() 
		&& otherAct * weightTo < GetPrimaryActivation())
	{
		REAL targetWeight = (REAL) GetPrimaryActivation() / otherAct;
		ASSERT(targetWeight > weightTo);

		REAL newWeight = weightTo + (targetWeight - weightTo) * k;
		if (newWeight < 2.0)
			pOtherNode->LinkTo(this, newWeight);
	}

	// TODO: now soft-normalize the link weights
}


//////////////////////////////////////////////////////////////////////
// CNode::GetActivation
// 
// returns the node's activation
//////////////////////////////////////////////////////////////////////
REAL CNode::GetActivation() const
{
	return m_primaryActivation + m_secondaryActivation;
}


//////////////////////////////////////////////////////////////////////
// CNode::GetPrimaryActivation
// 
// returns the node's primary activation
//////////////////////////////////////////////////////////////////////
REAL CNode::GetPrimaryActivation() const
{
	return m_primaryActivation;
}


//////////////////////////////////////////////////////////////////////
// CNode::GetSecondaryActivation
// 
// returns the node's secondary activation
//////////////////////////////////////////////////////////////////////
REAL CNode::GetSecondaryActivation() const
{
	return m_secondaryActivation;
}


//////////////////////////////////////////////////////////////////////
// CNode::GetSpringActivation
// 
// returns the node's spring activation value
//////////////////////////////////////////////////////////////////////
REAL CNode::GetSpringActivation() const
{
	return m_springActivation;
}


//////////////////////////////////////////////////////////////////////
// CNode::UpdateSprings
// 
// updates the spring activation
//////////////////////////////////////////////////////////////////////
void CNode::UpdateSpring(REAL springConst)
{
	// update spring activation
	m_springActivation = 
		GetActivation(); //  * ((REAL) 1.0 - springConst)
			// + m_springActivation * springConst;
}


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
}


//////////////////////////////////////////////////////////////////////
// CNode::GetDescendantActivation
// 
// sums the activation value of this node with all children nodes
//////////////////////////////////////////////////////////////////////
REAL CNode::GetDescendantActivation() const
{
	// initialize with the activation of this node
	REAL sum = GetActivation();

	// iterate over child nodes
	for (int nAt = 0; nAt < GetChildCount(); nAt++)
	{
		// for each child node
		const CNode *pNode = GetChildAt(nAt);

		// sum its descendent activation
		sum += pNode->GetDescendantActivation();
	}

	// return the sum
	return sum;
}


//////////////////////////////////////////////////////////////////////
// CNode::GetDescendantPrimaryActivation
// 
// sums the primary activation value of this node with all children 
// nodes
//////////////////////////////////////////////////////////////////////
REAL CNode::GetDescendantPrimaryActivation() const
{
	// initialize with the activation of this node
	REAL sum = GetPrimaryActivation();

	// iterate over child nodes
	for (int nAt = 0; nAt < GetChildCount(); nAt++)
	{
		// for each child node
		const CNode *pNode = GetChildAt(nAt);

		// sum its descendent activation
		sum += pNode->GetDescendantPrimaryActivation();
	}

	// return the sum
	return sum;
}


//////////////////////////////////////////////////////////////////////
// CNode::GetDescendantSecondaryActivation
// 
// sums the primary activation value of this node with all children 
// nodes
//////////////////////////////////////////////////////////////////////
REAL CNode::GetDescendantSecondaryActivation() const
{
	// initialize with the activation of this node
	REAL sum = GetSecondaryActivation();

	// iterate over child nodes
	for (int nAt = 0; nAt < GetChildCount(); nAt++)
	{
		// for each child node
		const CNode *pNode = GetChildAt(nAt);

		// sum its descendent activation
		sum += pNode->GetDescendantSecondaryActivation();
	}

	// return the sum
	return sum;
}


//////////////////////////////////////////////////////////////////////
// CNode::GetMaxActivator
// 
// returns the current maximum activator
//////////////////////////////////////////////////////////////////////
CNode *CNode::GetMaxActivator()
{
	return m_pMaxActivator;
}


//////////////////////////////////////////////////////////////////////
// CNode::GetView
// 
// returns the view object for this node
//////////////////////////////////////////////////////////////////////
CObject *CNode::GetView()
{
	return m_pView;
}


//////////////////////////////////////////////////////////////////////
// CNode::SetView
// 
// sets the view object for this node
//////////////////////////////////////////////////////////////////////
void CNode::SetView(CObject *pView)
{
	m_pView = pView;
}


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
	}
	else
	{
		ar << m_strName;
		ar << m_strDescription;
		ar << m_strImageFilename;
		ar << m_strUrl;
		ar << m_strClass;
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
}


//////////////////////////////////////////////////////////////////////
// CNode::SetActivation
// 
// sets the node's activation.  if an activator is passed, it is
//		compared to the current Max Activator
//////////////////////////////////////////////////////////////////////
void CNode::SetActivation(REAL newActivation, CNode *pActivator)
{
	// compute the delta
	REAL deltaActivation = newActivation - GetActivation();

	// set the activation, based on whether primary or secondary
	if (pActivator == NULL)
	{
		m_primaryActivation += deltaActivation;
	}
	else
	{
		m_secondaryActivation += deltaActivation;
	}

	// compare the delta for a new max activator
	if (deltaActivation > m_maxDeltaActivation 
		&& pActivator != NULL)
	{
		m_pMaxActivator = pActivator;
	}
}


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

		// only propagate through the link if we have not already done so
		if (!pLink->HasPropagated())
		{
			// mark this link as having propagated
			pLink->SetHasPropagated(TRUE);

			// get the link target
			CNode *pTarget = pLink->GetTarget();

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
			pTarget->SetActivation(newActivation, this);

			// and propagate to linked nodes
			pTarget->PropagateActivation(scale * (REAL) 1.0);
		}
	}
}


//////////////////////////////////////////////////////////////////////
// CNode::ResetForPropagation
// 
// resets the "hasPropagated" flags on the link weights
//////////////////////////////////////////////////////////////////////
void CNode::ResetForPropagation()
{
	// reset the max delta activation
	m_maxDeltaActivation = 0.0;
	m_pMaxActivator = NULL;

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
}


//////////////////////////////////////////////////////////////////////
// CNode::ScaleDescendantActivation
// 
// sums the activation value of this node with all children nodes
//////////////////////////////////////////////////////////////////////
void CNode::ScaleDescendantActivation(REAL primScale, REAL secScale)
{
	// scale this node's activation
	m_primaryActivation *= primScale;
	m_secondaryActivation *= secScale;

	// iterate over child nodes
	for (int nAt = 0; nAt < GetChildCount(); nAt++)
	{
		// for each child node
		CNode *pNode = GetChildAt(nAt);

		// scale the child's activation
		pNode->ScaleDescendantActivation(primScale, secScale);
	}
}


//////////////////////////////////////////////////////////////////////
// CNode::GetRandomDescendant
// 
// returns a random descendant, for testing purposes
//////////////////////////////////////////////////////////////////////
CNode * CNode::GetRandomDescendant()
{
	int nDescendant = rand() * GetDescendantCount() / RAND_MAX;

	for (int nAt = 0; nAt < GetChildCount(); nAt++)
	{
		CNode *pChild = GetChildAt(nAt);
		if (pChild->GetDescendantCount() > nDescendant)
			return pChild->GetRandomDescendant();
		nDescendant -= pChild->GetDescendantCount();
	}

	return GetChildAt(nDescendant);
}

const CVector<3>& CNode::GetPosition() const
{
	return m_vPosition;
}

void CNode::SetPosition(const CVector<3>& vPos)
{
	m_vPosition = vPos;
}

CVector<3> CNode::GetSize(REAL activation) const
{
	// compute the desired aspect ratio (maintain the current aspect ratio)
	REAL aspectRatio = 13.0 / 16.0 - 6.0 / 16.0 * (REAL) exp(-6.0f * activation);

	// compute the new width and height from the desired area and the desired
	//		aspect ratio
	CVector<3> vSize;
	vSize[0] = (REAL) sqrt(activation / aspectRatio);
	vSize[1] = (REAL) sqrt(activation * aspectRatio);

	// return the computed size
	return vSize;
}
