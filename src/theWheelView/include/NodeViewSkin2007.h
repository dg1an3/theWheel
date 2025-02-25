#pragma once

#include "Plaque.h"

class CNodeView;

namespace theWheel2007 {

/// <summary>
/// </summary>
class NodeViewSkin
{
public:
	/// <summary>
	/// constructs a new skin for the device
	/// </summary>
	NodeViewSkin(LPDIRECT3DDEVICE9 pDevice)
		: m_pDevice(pDevice)
	{
	}

	/// <summary>
	// renders the node view skin at the given height
	/// </summary>
	void Render(CNodeView *pNV);

	/// <summary>
	// calculates the size for a given activation
	/// </summary>
	void CalcRectForActivation(REAL activation, CExtent<2, REAL>& rect);

	virtual ~NodeViewSkin(void)
	{
		for (auto plaque : m_arrPlaques)
		{
			if (plaque)
				delete plaque;
		}
	}

protected:
	/// <summary>
	// retrieves (and possibly creates) the index of nearest plaque, given the activation
	/// </summary>
	int GetPlaqueIndex(REAL activation);

private:
	// stores the device
	LPDIRECT3DDEVICE9 m_pDevice;

	// array of moldings for the meshes
	std::vector<Plaque*> m_arrPlaques;

};	// class NodeViewSkin

}	// namespace 
