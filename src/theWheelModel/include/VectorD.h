//////////////////////////////////////////////////////////////////////
// Vector.h: declaration and definition of the CVectorD template class.
//
// Copyright (C) 1999-2006 Derek G Lane
// $Id: VectorD.h,v 1.1 2007/05/09 01:45:49 Derek Lane Exp $
//////////////////////////////////////////////////////////////////////

#if !defined(VECTOR_H)
#define VECTOR_H

// math utilities
#include "MathUtil.h"

// common operations
#include "VectorOps.h"


//////////////////////////////////////////////////////////////////////
// class CVectorD<DIM, TYPE>
//
// represents a mathematical vector with dimension and type given
//////////////////////////////////////////////////////////////////////
template<int DIM = 4, class TYPE = REAL>
class CVectorD
{
	// the vector's elements
	TYPE m_arrElements[DIM];

public:
	// constructors / destructor
	CVectorD();
	CVectorD(TYPE x, TYPE y);
	CVectorD(TYPE x, TYPE y, TYPE z);
	CVectorD(TYPE x, TYPE y, TYPE z, TYPE w);
	CVectorD(const CVectorD& vFrom);

#ifdef __AFXWIN_H__
	CVectorD(const CPoint& pt);
#endif

	~CVectorD();

	// assignment
	CVectorD& operator=(const CVectorD& vFrom);

	// conversion
#ifdef __AFXWIN_H__
	operator CPoint() const;
#endif

#ifdef USE_IPP
	operator IppiSize() const;
#endif

	// template helper for element type
	typedef TYPE ELEM_TYPE;

	// element accessors
	TYPE& operator[](int nAtRow);
	const TYPE& operator[](int nAtRow) const;

	// dimensional accessors
	int GetDim() const;

	// TYPE * conversion -- returns a pointer to the first element
	//		WARNING: this allows for no-bounds-checking access
	operator TYPE *();
	operator const TYPE *() const;

	// vector length and normalization
	TYPE GetLength() const;
	void Normalize();

	// tests for approximate equality using the EPS defined at the 
	//		top of this file
	bool IsApproxEqual(const CVectorD& v, TYPE epsilon = DEFAULT_EPSILON) const;

	// assignment operators -- requires proper type for strict
	//		compile-time check
	CVectorD& operator+=(const CVectorD& vRight);
	CVectorD& operator-=(const CVectorD& vRight);
	CVectorD& operator*=(const TYPE& scalar);

};	// class CVectorD<DIM, TYPE>


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE>::CVectorD() 
	// default constructor
{
	// clear to all zeros
	ZeroValues(&(*this)[0], DIM);

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>() 


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE>::CVectorD(TYPE x, TYPE y) 
	// construct from two elements
{
	// set the given elements
	(*this)[0] = x;
	(*this)[1] = y;

	// clear to all zeros
	if (DIM > 2)
	{
		// zero initial
		ZeroValues(&(*this)[2], DIM-2);
	}

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x, TYPE y) 


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE>::CVectorD(TYPE x, TYPE y, TYPE z) 
	// construct from three elements
{
	// set the given elements
	(*this)[0] = x;
	(*this)[1] = y;
	(*this)[2] = z;

	// clear to all zeros
	if (DIM > 3)
	{
		ZeroValues(&(*this)[3], DIM-3);
	}

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x, TYPE y, TYPE z) 


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE>::CVectorD(TYPE x, TYPE y, TYPE z, TYPE w) 
	// construct from four elements
{ 
	// set the given elements
	(*this)[0] = x;
	(*this)[1] = y;
	(*this)[2] = z;
	(*this)[3] = w;

	// clear to all zeros
	if (DIM > 4)
	{
		ZeroValues(&(*this)[4], DIM-4);
	}

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(TYPE x, TYPE y, TYPE z, TYPE w) 


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE>::CVectorD(const CVectorD<DIM, TYPE>& vFrom) 
	// copy constructor
{
	// assign to copy 
	(*this) = vFrom;

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(const CVectorD<DIM, TYPE>& vFrom) 


#ifdef __AFXWIN_H__

//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE>::CVectorD(const CPoint& pt)
	// construct from a windows CPoint
{
	// set the given elements
	(*this)[0] = R(pt.x);
	(*this)[1] = R(pt.y);

	// clear to all zeros
	if (DIM > 2)
	{
		ZeroValues(&(*this)[2], DIM-2);
	}

}	// CVectorD<DIM, TYPE>::CVectorD<DIM, TYPE>(const CPoint& pt)

#endif


#ifdef USE_IPP

//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE>::operator IppiSize() const
{
	IppiSize sz;
	sz.width = (*this)[0];
	sz.height = (*this)[1];

	return sz;

}	// CVectorD<DIM, TYPE>::operator IppiSize

#endif

//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE>::~CVectorD()
	// destructor
{
}	// CVectorD<DIM, TYPE>::~CVectorD<DIM, TYPE>()


#ifdef __AFXWIN_H__

//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE>::operator CPoint() const
	// Windows CPoint conversion 
{
	return CPoint((int) floor((*this)[0] + 0.5), 
		(int) floor((*this)[1] + 0.5));

}	// CVectorD<DIM, TYPE>::operator CPoint() const

#endif


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
	// assignment operator
CVectorD<DIM, TYPE>& 
	CVectorD<DIM, TYPE>::operator=(const CVectorD<DIM, TYPE>& vFrom)
{
	// copy the elements
	CopyValues(&(*this)[0], &vFrom[0], GetDim());

	// return a reference to this
	return (*this);

}	// CVectorD<DIM, TYPE>& CVectorD<DIM, TYPE>::operator=


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
int 
	CVectorD<DIM, TYPE>::GetDim() const
	// returns the dimensionality of the vector
{
	return DIM;

}	// CVectorD<DIM, TYPE>::GetDim


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
TYPE& 
	CVectorD<DIM, TYPE>::operator[](int nAtRow)
	// returns a reference to the specified element.
{
	// check dimensions
	ASSERT(nAtRow >= 0 && nAtRow < GetDim());

	return m_arrElements[nAtRow];

}	// CVectorD<DIM, TYPE>::operator[]


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
const TYPE& 
	CVectorD<DIM, TYPE>::operator[](int nAtRow) const
	// returns a const reference to the specified element.
{
	// check dimensions
	ASSERT(nAtRow >= 0 && nAtRow < GetDim());

	return m_arrElements[nAtRow];

}	// CVectorD<DIM, TYPE>::operator[] const


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE>::operator TYPE *()
	// TYPE * conversion -- returns a pointer to the first element
	//		WARNING: this allows for no-bounds-checking access
{
	return &(*this)[0];

}	// CVectorD<DIM, TYPE>::operator TYPE *


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE>::operator const TYPE *() const
	// const TYPE * conversion -- returns a pointer to the first 
	//		element.
	//		WARNING: this allows for no-bounds-checking access
{
	return &(*this)[0];

}	// CVectorD<DIM, TYPE>::operator const TYPE *


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
TYPE 
	CVectorD<DIM, TYPE>::GetLength() const
	// returns the euclidean length of the vector
{
	return VectorLength(&(*this)[0], DIM);

}	// CVectorD<DIM, TYPE>::GetLength


#ifdef USE_IPP

#ifdef IPP_L2NORM_32F_3_BUG_FIXED
// DGL: this appears to be broken in IPP
//////////////////////////////////////////////////////////////////
template<> INLINE							
float
	CVectorD<3, float>::GetLength() const
{										
	float length;						
	CK_IPP(ippmL2Norm_v_32f_3x1(&(*this)[0], &length));

	return length;

}	// CVectorD<3, float>::GetLength() const
#endif

//////////////////////////////////////////////////////////////////
template<> INLINE							
float
	CVectorD<4, float>::GetLength() const
{										
	float length;						
	CK_IPP(ippmL2Norm_v_32f_4x1(&(*this)[0], &length));

	return length;

}	// CVectorD<4, float>::GetLength() const


//////////////////////////////////////////////////////////////////
template<> INLINE							
double
	CVectorD<3, double>::GetLength() const
{										
	double length;						
	CK_IPP(ippmL2Norm_v_64f_3x1(&(*this)[0], &length));

	return length;

}	// CVectorD<3, double>::GetLength() const


//////////////////////////////////////////////////////////////////
template<> INLINE							
double
	CVectorD<4, double>::GetLength() const
{										
	double length;						
	CK_IPP(ippmL2Norm_v_64f_4x1(&(*this)[0], &length));

	return length;

}	// CVectorD<4, double>::GetLength() const

#endif

//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
void 
	CVectorD<DIM, TYPE>::Normalize()
	// scales the vector so its length is 1.0
{
 	TYPE len = GetLength();
	if (len > 0.0)
 	{
 		MultValues(&(*this)[0], (TYPE) 1.0 / len, GetDim());
 	}

}	// CVectorD<DIM, TYPE>::Normalize


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
bool 
	CVectorD<DIM, TYPE>::IsApproxEqual(const CVectorD<DIM, TYPE>& v, 
			TYPE epsilon) const
	// tests for approximate equality using the given epsilon
{
	// form the difference vector
	CVectorD<DIM, TYPE> vDiff(*this);
	vDiff -= v;

	return (vDiff.GetLength() < epsilon);

}	// CVectorD<DIM, TYPE>::IsApproxEqual


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE>& 
	CVectorD<DIM, TYPE>::operator+=(const CVectorD<DIM, TYPE>& vRight)
	// in-place vector addition; returns a reference to this
{
	SumValues(&(*this)[0], &vRight[0], GetDim());

	return (*this);

}	// CVectorD<DIM, TYPE>::operator+=


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE>& 
	CVectorD<DIM, TYPE>::operator-=(const CVectorD<DIM, TYPE>& vRight)
	// in-place vector subtraction; returns a reference to this
{
	DiffValues(&(*this)[0], &vRight[0], GetDim());

	return (*this);

}	// CVectorD<DIM, TYPE>::operator-=


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE>& 
	CVectorD<DIM, TYPE>::operator*=(const TYPE& scalar)
	// in-place scalar multiplication; returns a reference to this
{
	MultValues(&(*this)[0], scalar, GetDim());

	return (*this);

}	// CVectorD<TYPE>::operator*=


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
bool 
	operator==(const CVectorD<DIM, TYPE>& vLeft, 
			const CVectorD<DIM, TYPE>& vRight)
	// friend function to provide equality comparison for vectors.
	// use IsApproxEqual for approximate equality.
{
	// test for element-wise equality
	for (int nAt = 0; nAt < vLeft.GetDim(); nAt++)
	{
		if (vLeft[nAt] != vRight[nAt])
		{
			return false;
		}
	}

	return true;

}	// operator==(const CVectorD, const CVectorD)


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
bool 
	operator!=(const CVectorD<DIM, TYPE>& vLeft, 
			const CVectorD<DIM, TYPE>& vRight)
	// friend function to provide inequality comparison for vectors.
	// use !IsApproxEqual for approximate inequality.
{
	return !(vLeft == vRight);

}	// operator!=(const CVectorD, const CVectorD)


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE> 
	operator+(const CVectorD<DIM, TYPE>& vLeft, 
			const CVectorD<DIM, TYPE>& vRight)
	// friend function to add two vectors, returning the sum as a new 
	//		vector
{
	CVectorD<DIM, TYPE> vSum = vLeft;
	vSum += vRight;

	return vSum;

}	// operator+(const CVectorD, const CVectorD)


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE> 
	operator-(const CVectorD<DIM, TYPE>& vLeft, 
			const CVectorD<DIM, TYPE>& vRight)
	// friend function to subtract one vector from another, returning 
	//		the difference as a new vector
{
	CVectorD<DIM, TYPE> vSum = vLeft;
	vSum -= vRight;

	return vSum;

}	// operator-(const CVectorD, const CVectorD)


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
TYPE 
	operator*(const CVectorD<DIM, TYPE>& vLeft, 
			const CVectorD<DIM, TYPE>& vRight)
	// friend function for vector inner product
{
	return DotProduct(&vLeft[0], &vRight[0], vLeft.GetDim());

}	// operator*(const CVectorD, const CVectorD)


#ifdef USE_IPP

//////////////////////////////////////////////////////////////////////
template<> INLINE								
float 
	operator*(const CVectorD<3, float>& vLeft,		
			const CVectorD<3, float>& vRight)		
{														
	float prod;											
	CK_IPP(ippmDotProduct_vv_32f_3x1(&vLeft[0], &vRight[0], &prod));

	return prod;										

}	// operator*(const CVectorD<3, float>& vLeft,		

//////////////////////////////////////////////////////////////////////
template<> INLINE								
float 
	operator*(const CVectorD<4, float>& vLeft,		
			const CVectorD<4, float>& vRight)		
{														
	float prod;											
	CK_IPP(ippmDotProduct_vv_32f_4x1(&vLeft[0], &vRight[0], &prod));

	return prod;										

}	// operator*(const CVectorD<4, float>& vLeft,		


//////////////////////////////////////////////////////////////////////
template<> INLINE								
double
	operator*(const CVectorD<3, double>& vLeft,		
			const CVectorD<3, double>& vRight)		
{														
	double prod;											
	CK_IPP(ippmDotProduct_vv_64f_3x1(&vLeft[0], &vRight[0], &prod));

	return prod;										

}	// operator*(const CVectorD<3, double>& vLeft,		

//////////////////////////////////////////////////////////////////////
template<> INLINE								
double
	operator*(const CVectorD<4, double>& vLeft,		
			const CVectorD<4, double>& vRight)		
{														
	double prod;											
	CK_IPP(ippmDotProduct_vv_64f_4x1(&vLeft[0], &vRight[0], &prod));

	return prod;										

}	// operator*(const CVectorD<4, double>& vLeft,		

#endif

//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE> 
	operator*(TYPE scalar, 
			const CVectorD<DIM, TYPE>& vRight)
	// friend function for scalar multiplication of a vector
{
	CVectorD<DIM, TYPE> vProd = vRight;
	vProd *= scalar;

	return vProd;

}	// operator*(TYPE scalar, const CVectorD)


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE> 
	operator*(const CVectorD<DIM, TYPE>& vLeft, 
			TYPE scalar)
	// friend function for scalar multiplication of a vector
{
	CVectorD<DIM, TYPE> vProd = vLeft;
	vProd *= scalar;

	return vProd;

}	// operator*(const CVectorD, TYPE scalar)


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
TYPE 
	Cross(const CVectorD<2, TYPE>& vLeft, 
			const CVectorD<2, TYPE>& vRight)
	// friend function for vector cross product of 2-d vectors
{
	return vLeft[0] * vRight[1] - vLeft[1] * vRight[0];

}	// Cross(const CVectorD<2>&, const CVectorD<2>&)


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
CVectorD<3, TYPE> 
	Cross(const CVectorD<3, TYPE>& vLeft, 
			const CVectorD<3, TYPE>& vRight)
	// friend function for vector cross product of 3-d vectors
{
	CVectorD<3, TYPE> vProd;

	vProd[0] =   vLeft[1] * vRight[2] - vLeft[2] * vRight[1];
	vProd[1] = -(vLeft[0] * vRight[2] - vLeft[2] * vRight[0]);
	vProd[2] =   vLeft[0] * vRight[1] - vLeft[1] * vRight[0];

	return vProd;

}	// Cross(const CVectorD<3>&, const CVectorD<3>&)


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM + 1, TYPE> 
	ToHG(const CVectorD<DIM, TYPE>& v) 
	// converts an N-dimensional vector to an N+1-dimensional homogeneous
	//		vector
{
	// create the homogeneous vector
	CVectorD<DIM + 1, TYPE> vh;

	// fill with the given vector's elements
	for (int nAt = 0; nAt < DIM; nAt++)
	{
		vh[nAt] = v[nAt];
	}

	// set last element to 1.0
	vh[DIM] = (TYPE) 1.0;

	return vh;

}	// ToHG(const CVectorD&)

#define ToHomogeneous ToHG


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE> 
	FromHG(const CVectorD<DIM + 1, TYPE>& vh) 
	// converts an N+1-dimensional homogeneous vector to an N-dimensional 
	//		vector
{
	// create the non-homogeneous vector
	CVectorD<DIM, TYPE> v;

	// normalize the first DIM-1 elements by the last element
	for (int nAt = 0; nAt < DIM; nAt++)
	{
		v[nAt] = vh[nAt] / vh[DIM];
	}

	return v;

}	// FromHG(const CVectorD&)

#define FromHomogeneous FromHG

#ifdef __AFX_H__

//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CArchive& 
	operator<<(CArchive &ar, CVectorD<DIM, TYPE> v)
	// CArchive serialization of a vector
{
	for (int nAt = 0; nAt < DIM; nAt++)
	{
		ar << v[nAt];
	}

	return ar;

}	// operator<<(CArchive &ar, CVectorD)


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CArchive& 
	operator>>(CArchive &ar, CVectorD<DIM, TYPE>& v)
	// CArchive de-serialization of a vector
{
	for (int nAt = 0; nAt < DIM; nAt++)
	{
		ar >> v[nAt];
	}

	return ar;

}	// operator>>(CArchive &ar, CVectorD)

#endif

//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
void 
	TraceVector(const CVectorD<DIM, TYPE>& vTrace)
	// helper function to output a vector for debugging
{
#ifdef _DEBUG
	TRACE("<");
	for (int nAt = 0; nAt < vTrace.GetDim(); nAt++)
	{
		TRACE("%lf\t", vTrace[nAt]);
	}
	TRACE(">\n");
#endif

}	// TraceVector


//////////////////////////////////////////////////////////////////////
// TRACE_VECTOR
//
// macro to trace matrix -- only compiles in debug version
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define TRACE_VECTOR(strMessage, v) \
	TRACE(strMessage);				\
	TraceVector(v);					\
	TRACE("\n");
#else
#define TRACE_VECTOR(strMessage, v)
#endif


#endif	// !defined(VECTOR_H)