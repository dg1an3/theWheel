// THEWHEEL_MODEL_Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "THEWHEEL_MODEL_Test.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <Space.h>

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

void PrintInfo(CNode *pNode)
{
	const char *strName = pNode->name.Get();
	cout << strName << ": " << pNode->activation.Get() << endl;
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		nRetCode = 1;
	}
	else
	{
		CSpace *pSpace = (CSpace *)RUNTIME_CLASS(CSpace)->CreateObject();
		pSpace->OnNewDocument();

		// write the activation for all nodes (before)
		cout << "Total activation of all nodes = " 
			<< pSpace->rootNode.GetDescendantActivation()
			<< endl;

		// now normalize
		pSpace->NormalizeNodes();

		// write the activation for all nodes (after normalization)
		cout << "Total activation of all nodes = " 
			<< pSpace->rootNode.GetDescendantActivation()
			<< endl;

		// now try a propagation

		// boost the root node up to about 10 times the other nodes
		double rootActivation = pSpace->rootNode.activation.Get();
		pSpace->rootNode.activation.Set(rootActivation * 100.0);

		// and propagate the activation at a scale of 0.8
		pSpace->rootNode.PropagateActivation(0.8);
		pSpace->rootNode.ResetForPropagation();

		// now normalize
		pSpace->NormalizeNodes();

		// and print some activation values
		PrintInfo(&pSpace->rootNode);
		for (int nAt = 0; nAt < pSpace->rootNode.children.GetSize(); nAt++)
		{
			CNode *pNode = (CNode *)pSpace->rootNode.children.Get(nAt);
			PrintInfo(pNode);
			for (int nAt2 = 0; nAt2 < pNode->children.GetSize(); nAt2++)
			{
				PrintInfo((CNode *)pNode->children.Get(nAt2));
			}
		}
	}

	return nRetCode;
}


