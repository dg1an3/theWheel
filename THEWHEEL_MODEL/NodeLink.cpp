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
: m_bPropagated(FALSE),
	forTarget(pToNode),
	weight(weight)
{

}

CNodeLink::~CNodeLink()
{

}

IMPLEMENT_SERIAL(CNodeLink, CObject, 1)

void CNodeLink::Serialize(CArchive &ar)
{
	weight.Serialize(ar);
	forTarget.Serialize(ar);

//	if (ar.IsStoring())
//	{
//		ar << m_weight;
//		ar << m_pToNode; // ar.WriteObject((CObject *)m_pToNode);
//	}
//	else
//	{
//		ar >> m_weight;
//		ar >> m_pToNode; // = (CNode *)ar.ReadObject(RUNTIME_CLASS(CNode));
//	}
}

//float CNodeLink::GetWeight()
//{
//	return myWeight.Get();
//}
