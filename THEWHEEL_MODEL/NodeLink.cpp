// NodeLink.cpp: implementation of the CNodeLink class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
// #include "theWheel2001.h"
#include "NodeLink.h"

#include "Node.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNodeLink::CNodeLink(CNode *pToNode, float weight)
	: forTarget(pToNode),
		weight(weight),
		hasPropagated(FALSE)
{

}

CNodeLink::~CNodeLink()
{

}

IMPLEMENT_SERIAL(CNodeLink, CObject, 1)

void CNodeLink::Serialize(CArchive &ar)
{
	// serialize the link weight
	weight.Serialize(ar);

	// serialize the link target
	forTarget.Serialize(ar);
}
