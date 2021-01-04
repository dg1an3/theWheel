#pragma once

#include "afx.h"

// forward declaration of the space class
class CSpace;

namespace theWheel {

class NodeBase : public CObject
{
public:
	NodeBase(void);
	virtual ~NodeBase(void);

	//////////////////////////////////////////////////////////////////
	// hierarchy

	// the space that contains the node
	CSpace *GetSpace();

	// the node's parent
	DECLARE_ATTRIBUTE_PTR_GI(Parent, NodeBase);

	// the node's children
	int GetChildCount() const;
	NodeBase *GetChildAt(int nAt);

	// returns the number of descendants of this node
	int GetDescendantCount();

	//////////////////////////////////////////////////////////////////
	// attribute accessors

	// the node's primary attributes
	DECLARE_ATTRIBUTE_GI(Name, CString);
	DECLARE_ATTRIBUTE_GI(Description, CString);
	DECLARE_ATTRIBUTE_GI(Class, CString);
	DECLARE_ATTRIBUTE_GI(ImageFilename, CString);

	// loads the image file, if necessary
	CDib *GetDib();

private:
	// pointer to the space that contains this node
	CSpace *m_pSpace;

	// the collection of children
	vector<NodeBase*> m_arrChildren;

	// pointer to the DIB, if it is loaded
	CDib *m_pDib;

};	// class NodeBase

}	// namespace theWheel