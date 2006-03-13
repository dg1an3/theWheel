#pragma once

#include <VectorD.h>

template<class TYPE = REAL>
class CExtent
{
public:
	CExtent(void);
	CExtent(const CRect& rect);
	~CExtent(void);


	operator CRect() const;

	TYPE Width(void) const;
	void SetWidth(TYPE width);

	TYPE Height(void) const;
	void SetHeight(TYPE height);

	TYPE Left(void) const;
	void SetLeft(TYPE left);

	TYPE Right(void) const;
	void SetRight(TYPE left);

	TYPE Top(void) const;
	void SetTop(TYPE top);

	TYPE Bottom(void) const;
	void SetBottom(TYPE bottom);

	// deflates extent
	void Deflate(TYPE left, TYPE top, TYPE right, TYPE bottom);

	void Interpolate(const CExtent<TYPE>& with, TYPE frac);

private:
	// stores upper-left corner
	CVectorD<3, TYPE> m_vMin;

	// stores lower-right corner
	CVectorD<3, TYPE> m_vMax;

};

template<class TYPE>
CExtent<TYPE>::CExtent(void)
{
}

template<class TYPE>
CExtent<TYPE>::CExtent(const CRect& rect)
{
	m_vMin[0] = rect.left;
	m_vMin[1] = rect.top;
	m_vMax[0] = rect.right;
	m_vMax[1] = rect.bottom;
}


template<class TYPE>
CExtent<TYPE>::~CExtent(void)
{
}


template<class TYPE>
CExtent<TYPE>::operator CRect() const
{
	return CRect(Round<int>(Left()), Round<int>(Top()), 
		Round<int>(Right()), Round<int>(Bottom()));
}

template<class TYPE>
TYPE CExtent<TYPE>::Width() const
{
	return m_vMax[0] - m_vMin[0];
}

template<class TYPE>
void CExtent<TYPE>::SetWidth(TYPE width)
{
	m_vMax[0] = m_vMin[0] + width;
}

template<class TYPE>
TYPE CExtent<TYPE>::Height() const
{
	return m_vMax[1] - m_vMin[1];
}

template<class TYPE>
void CExtent<TYPE>::SetHeight(TYPE height)
{
	m_vMax[1] = m_vMin[1] + height;
}

template<class TYPE>
TYPE CExtent<TYPE>::Left() const
{
	return m_vMin[0];
}

template<class TYPE>
void CExtent<TYPE>::SetLeft(TYPE left)
{
	m_vMin[0] = left;
}

template<class TYPE>
TYPE CExtent<TYPE>::Right() const
{
	return m_vMax[0];
}

template<class TYPE>
void CExtent<TYPE>::SetRight(TYPE right)
{
	m_vMax[0] = right;
}

template<class TYPE>
TYPE CExtent<TYPE>::Top() const
{
	return m_vMin[1];
}

template<class TYPE>
void CExtent<TYPE>::SetTop(TYPE top)
{
	m_vMin[1] = top;
}

template<class TYPE>
TYPE CExtent<TYPE>::Bottom() const
{
	return m_vMax[1];
}

template<class TYPE>
void CExtent<TYPE>::SetBottom(TYPE bottom)
{
	m_vMax[1] = bottom;
}

// deflates extent
template<class TYPE>
void CExtent<TYPE>::Deflate(TYPE left, TYPE top, TYPE right, TYPE bottom)
{
	m_vMin[0] += left;
	m_vMin[1] += top;
	m_vMax[0] -= right;
	m_vMax[1] -= bottom;
}

template<class TYPE>
void CExtent<TYPE>::Interpolate(const CExtent<TYPE>& with, TYPE frac)
{
	m_vMin = frac * m_vMin + (R(1.0) - frac) * with.m_vMin;
	m_vMax = frac * m_vMax + (R(1.0) - frac) * with.m_vMax;
}
