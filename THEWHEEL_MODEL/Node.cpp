// Node.cpp: implementation of the CNode class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
// #include "theWheel2001.h"
#include "Node.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNode::CNode(const CString& strName, const CString& strDesc)
	: parent(NULL),
		description(strDesc)
{
	name.Set(strName);
}

CNode::~CNode()
{
}

IMPLEMENT_SERIAL(CNode, CModelObject, 4);

void CNode::LinkTo(CNode *pToNode, float weight)
{
	links.Add(new CNodeLink(pToNode, weight));
	pToNode->links.Add(new CNodeLink(this, weight));
}

CNodeLink * CNode::GetLink(CNode * pToNode)
{
	for (int nAt = 0; nAt < links.GetSize(); nAt++)
		if (links.Get(nAt)->forTarget.Get() == pToNode)
			return links.Get(nAt); 

	return NULL;
}

float CNode::GetLinkWeight(CNode * pToNode)
{
	CNodeLink *pLink = GetLink(pToNode);
	if (pLink)
		return pLink->weight.Get();

	return 0.0f;
}

void CNode::Serialize(CArchive &ar)
{
	name.Serialize(ar);
	description.Serialize(ar);

	children.Serialize(ar);
	links.Serialize(ar);

	if (!ar.IsStoring())
		for (int nAt = 0; nAt < children.GetSize(); nAt++)
			((CNode *)children.Get(nAt))->parent.Set(this);

//	CObArray arrChildren;
//	CObArray arrLinks;
//	if (ar.IsStoring())
//	{
//		for (int nAt = 0; nAt < myChildren.GetSize(); nAt++)
//			arrChildren.Add(myChildren.Get(nAt));
//		arrChildren.Serialize(ar);
//
//		for (nAt = 0; nAt < myLinks.GetSize(); nAt++)
//			arrLinks.Add(myLinks.Get(nAt));
//		arrLinks.Serialize(ar);
//	}
//	else
//	{
//		arrChildren.Serialize(ar);
//		for (int nAt = 0; nAt < arrChildren.GetSize(); nAt++)
//			myChildren.Add((CNode *)arrChildren.GetAt(nAt));
//
//		arrLinks.Serialize(ar);
//		for (nAt = 0; nAt < arrLinks.GetSize(); nAt++)
//			myLinks.Add((CNodeLink *)arrLinks.GetAt(nAt));
//	}
}

#ifdef NONE
void CNode::NormalizeLinks()
{
	float sum = 0.0;
	for (int nAt = 0; nAt < links.GetSize(); nAt++)
	{
		CNodeLink *pLink = links.Get(nAt);
		sum += pLink->weight.Get();
	}

	for (nAt = 0; nAt < links.GetSize(); nAt++)
	{
		CNodeLink *pLink = links.Get(nAt);
		pLink->weight.Set(pLink->weight.Get() / sum);
	}

	float entropy = 0.0;
	for (nAt = 0; nAt < links.GetSize(); nAt++)
	{
		float weight = links.Get(nAt)->weight.Get();
		entropy += - weight * log( weight);
	}
	TRACE2("Node %s entropy = %lf\n", name.Get(), entropy);

	for (nAt = 0; nAt < children.GetSize(); nAt++)
	{
		CNode *pChild = (CNode *)children.Get(nAt);
		pChild->NormalizeLinks();
	}
}
#endif

void CNode::NormalizeLinks(float temp)
{
	float sum = 0.0;
	for (int nAt = 0; nAt < links.GetSize(); nAt++)
	{
		CNodeLink *pLink = links.Get(nAt);
		sum += (float) exp(pLink->weight.Get() / temp);
	}

	for (nAt = 0; nAt < links.GetSize(); nAt++)
	{
		CNodeLink *pLink = links.Get(nAt);
		pLink->weight.Set((float) exp(pLink->weight.Get() / temp) / sum);
	}

	float entropy = 0.0;
	for (nAt = 0; nAt < links.GetSize(); nAt++)
	{
		float weight = links.Get(nAt)->weight.Get();
		entropy += (float)(-weight * log( weight));
	}
	TRACE2("Node %s entropy = %lf\n", name.Get(), entropy);

	for (nAt = 0; nAt < children.GetSize(); nAt++)
	{
		CNode *pChild = (CNode *)children.Get(nAt);
		pChild->NormalizeLinks(temp);
	}
}
