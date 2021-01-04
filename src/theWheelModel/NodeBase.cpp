#include "StdAfx.h"
#include "NodeBase.h"

namespace theWheel {

NodeBase::NodeBase(void)
{
}

NodeBase::~NodeBase(void)
{
}

//////////////////////////////////////////////////////////////////////
CSpace *NodeBase::GetSpace()
	// returns a pointer to the space containing the node
{
	return m_pSpace;

}	// CNode::GetSpace


// TODO: this logic should be in pParent->AddChild???
//////////////////////////////////////////////////////////////////////
void 
	NodeBase::SetParent(CNode *pParent)
	// sets a pointer to the node's parent
{
	// if this currently has a parent
	if (m_pParent)
	{
		// find this in the children array
		vector<CNode*>::iterator iter;
		iter = std::remove(m_pParent->m_arrChildren.begin(), m_pParent->m_arrChildren.end(), this);
		m_pParent->m_arrChildren.erase(iter, m_pParent->m_arrChildren.end());
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

		// add to the children array
		m_pParent->m_arrChildren.push_back(this); 

	}

}	// CNode::SetParent


//////////////////////////////////////////////////////////////////////
int 
	NodeBase::GetChildCount() const
	// returns the number of child nodes for this node
{
	return (int) m_arrChildren.size();

}	// CNode::GetChildCount


//////////////////////////////////////////////////////////////////////
CNode *
	NodeBase::GetChildAt(int nAt)
	// returns the child node at the given index
{
	return m_arrChildren.at(nAt);

}	// CNode::GetChildAt


//////////////////////////////////////////////////////////////////////
int 
	NodeBase::GetDescendantCount()
	// returns the number of descendants
{
	int nCount = 0;
	for (int nAt = 0; nAt < GetChildCount(); nAt++)
	{
		CNode *pChild = GetChildAt(nAt);
		nCount += pChild->GetDescendantCount() + 1;		// 1 for the child itself
	}

	return nCount;

}	// CNode::GetDescendantCount


//////////////////////////////////////////////////////////////////////
void 
	NodeBase::SetName(const CString& strName)
	// sets the name of the node
{
	m_Name = strName;

	// fire change
	NODE_FIRE_CHANGE();

}	// CNode::SetName


//////////////////////////////////////////////////////////////////////
void 
	NodeBase::SetDescription(const CString& strDesc)
	// sets the node's description text
{
	m_Description = strDesc;

	// fire change
	NODE_FIRE_CHANGE();

}	// CNode::SetDescription


//////////////////////////////////////////////////////////////////////
void 
	NodeBase::SetClass(const CString& strClass)
	// sets the node class	
{
	m_Class = strClass;

	// fire change
	NODE_FIRE_CHANGE();

}	// CNode::SetClass



//////////////////////////////////////////////////////////////////////
void 
	NodeBase::SetImageFilename(const CString& strImageFilename)
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
CDib *
	NodeBase::GetDib()
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

}	// namespace theWheel