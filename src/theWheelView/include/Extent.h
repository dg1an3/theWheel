#pragma once

// constants for CExtent::SetSize
const int FIX_MIN = -1;
const int FIX_CENTER = 0;
const int FIX_MAX = 1;

template<int DIM = 3, class TYPE = int>
class CExtent
{
public:
	CExtent(void);
	CExtent(const RECT& rect);
	~CExtent(void);

	operator RECT() const;

#ifdef USE_IPP
	operator IppiSize() const;
	operator IppiRect() const;
#endif

	const CVectorD<DIM, TYPE>& GetMin() const 
	{
		return m_vMin; 
	}
	TYPE GetMin(int nDim) const 
	{ 
		return m_vMin[nDim]; 
	}
	void SetMin(const CVectorD<DIM, TYPE>& vMin) 
	{ 
		m_vMin = vMin; 
	}
	void SetMin(int nDim, TYPE value) 
	{ 
		m_vMin[nDim] = value; 
	}

	const CVectorD<DIM, TYPE>& GetMax() const 
	{ 
		return m_vMax; 
	}
	TYPE GetMax(int nDim) const 
	{
		return m_vMax[nDim]; 
	}
	void SetMax(const CVectorD<DIM, TYPE>& vMax) 
	{ 
		m_vMax = vMax; 
	}
	void SetMax(int nDim, TYPE value) 
	{ 
		m_vMax[nDim] = value; 
	}

	CVectorD<DIM, TYPE> GetCenter() const;
	TYPE GetCenter(int nDim) const 
	{ 
		return GetCenter()[nDim]; 
	}
	void SetCenter(const CVectorD<DIM, TYPE>& vCenter);

	CVectorD<DIM, TYPE> GetSize() const 
	{ 
		return m_vMax - m_vMin; 
	}
	TYPE GetSize(int nDim) const 
	{ 
		return m_vMax[nDim] - m_vMin[nDim]; 
	}
	void SetSize(const CVectorD<DIM, TYPE>& vSize, int nFix = FIX_CENTER);
	void SetSize(int nDim, TYPE size, int nFix = FIX_CENTER);

	// deflates extent
	void Deflate(TYPE left, TYPE top, TYPE right, TYPE bottom);

	// interpolates between two extents
	void Interpolate(const CExtent<DIM,TYPE>& with, TYPE frac);

private:
	// stores min / max of the extent
	CVectorD<DIM, TYPE> m_vMin;
	CVectorD<DIM, TYPE> m_vMax;
};

template<int DIM, class TYPE>
	CExtent<DIM, TYPE>::CExtent(void)
{
}

template<int DIM, class TYPE>
CExtent<DIM, TYPE>::CExtent(const RECT& rect)
{
	m_vMin[0] = rect.left;
	m_vMin[1] = rect.top;
	m_vMax[0] = rect.right;
	m_vMax[1] = rect.bottom;
}

template<int DIM, class TYPE>
	CExtent<DIM, TYPE>::~CExtent(void)
{
}

template<int DIM, class TYPE>
	CExtent<DIM, TYPE>::operator RECT() const
{
	RECT rectOut;
	rectOut.left = Round<int>(GetMin(0));
	rectOut.right = Round<int>(GetMax(0));
	rectOut.top = Round<int>(GetMin(1));
	rectOut.bottom = Round<int>(GetMax(1));

	return rectOut;
}

#ifdef USE_IPP
template<int DIM, class TYPE>
	CExtent<DIM, TYPE>::operator IppiSize() const
{
	IppiSize sz;
	sz.width = Round<int>(m_vMax[0] - m_vMin[0]);
	sz.height = Round<int>(m_vMax[1] - m_vMin[1]);
	return sz;
}

template<int DIM, class TYPE>
	CExtent<DIM, TYPE>::operator IppiRect() const
{
	IppiRect rect;
	rect.x = Round<int>(m_vMin[0]);
	rect.y = Round<int>(m_vMin[1]);
	rect.width = Round<int>(m_vMax[0] - m_vMin[0]);
	rect.height = Round<int>(m_vMax[1] - m_vMin[1]);
	return rect;
}
#endif

template<int DIM, class TYPE>
CVectorD<DIM, TYPE>
	CExtent<DIM, TYPE>::GetCenter(void) const
{
	CVectorD<DIM, TYPE> vCenter = GetMin();
	vCenter += GetMax();
	for (int nD = 0; nD < vCenter.GetDim(); nD++)
	{
		vCenter[nD] /= (TYPE) 2;
	}

	return vCenter;
}

template<int DIM, class TYPE>
void 
	CExtent<DIM, TYPE>::SetCenter(const CVectorD<DIM, TYPE>& vCenter)
{
	CVectorD<DIM, TYPE> vSize = GetSize();
	SetMin(vCenter);
	SetMax(vCenter);
	SetSize(vSize, FIX_CENTER);
}

template<int DIM, class TYPE>
void 
	CExtent<DIM, TYPE>::SetSize(const CVectorD<DIM, TYPE>& vSize, int nFix)
{
	switch (nFix)
	{
	case FIX_MIN :
		{
		SetMax(GetMin() + vSize);
		break;
		}

	case FIX_MAX :
		{
		SetMin(GetMax() - vSize);
		break;
		}

	case FIX_CENTER :
	default :
		{
		CVectorD<DIM, TYPE> vCenter = GetCenter();
		SetMin(vCenter - (TYPE) 0.5 * vSize);
		SetMax(vCenter + (TYPE) 0.5 * vSize);
		break;
		}
	}
}

template<int DIM, class TYPE>
void 
	CExtent<DIM, TYPE>::SetSize(int nDim, TYPE size, int nFix) 
{
	switch (nFix)
	{
	case FIX_MIN :
		{
		SetMax(nDim, GetMin(nDim) + size);
		break;
		}

	case FIX_MAX :
		{
		SetMin(nDim, GetMax(nDim) - size);
		break;
		}
	
	case FIX_CENTER :
	default:
		{
		CVectorD<DIM, TYPE> vCenter = GetCenter();
		SetMin(nDim, vCenter[nDim] - (TYPE) 0.5 * size);
		SetMax(nDim, vCenter[nDim] + (TYPE) 0.5 * size);
		break;
		}
	}
}

// deflates extent
template<int DIM, class TYPE>
void 
	CExtent<DIM, TYPE>::Deflate(TYPE left, TYPE top, TYPE right, TYPE bottom)
{
	m_vMin[0] += left;
	m_vMin[1] += top;
	m_vMax[0] -= right;
	m_vMax[1] -= bottom;
}

template<int DIM, class TYPE>
void 
	CExtent<DIM, TYPE>::Interpolate(const CExtent<DIM,TYPE>& with, TYPE frac)
{
	m_vMin = frac * m_vMin + (R(1.0) - frac) * with.m_vMin;
	m_vMax = frac * m_vMax + (R(1.0) - frac) * with.m_vMax;
}
