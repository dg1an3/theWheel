//////////////////////////////////////////////////////////////////////
// VectorN.h: declaration and definition of the VectorN dynamic 
//		vector template class.
//
// Copyright (C) 1999-2006 Derek G Lane
// $Id: VectorN.h,v 1.1 2007/05/09 01:45:48 Derek Lane Exp $
//////////////////////////////////////////////////////////////////////

#if !defined(VECTORN_H)
#define VECTORN_H

#ifdef USE_IPP
#include <ippm.h>
#endif

#include <MathUtil.h>

#include <VectorOps.h>

//////////////////////////////////////////////////////////////////////
// class VectorN<TYPE>
//
// represents a dynamically sizable mathematical vector with element
//		type given
//////////////////////////////////////////////////////////////////////
template<class TYPE = REAL>
class VectorN
{
	// dimension of vector
	int m_nDim;

	// vector elements
	TYPE *m_pElements;

	// flag to indicate whether the elements should be freed
	bool m_bFreeElements;

public:
	// constructors / destructor
	VectorN();
	explicit VectorN(int nDim);
	VectorN(const VectorN& vFrom);
	~VectorN();

	// assignment operator
	VectorN& operator=(const VectorN& vFrom);

	// template helper for element type
	typedef TYPE ELEM_TYPE;

	// initializes all elements to zero
	void SetZero() { ZeroValues(&(*this)[0], GetDim()); }

	// element accessors
	TYPE& operator[](int nAtRow);
	const TYPE& operator[](int nAtRow) const;

	// the dimensionality of this vector
	int GetDim() const;
	void SetDim(int nDim);

	// TYPE * conversion -- returns a pointer to the first element
	//		WARNING: this allows for no-bounds-checking access
	operator TYPE *();
	operator const TYPE *() const;

	// vector length and normalization
	TYPE GetLength() const;
	void Normalize();

	// approximate equality using the epsilon
	bool IsApproxEqual(const VectorN& v, TYPE epsilon = DEFAULT_EPSILON) const;

	// in-place vector arithmetic
	VectorN& operator+=(const VectorN& vRight);
	VectorN& operator-=(const VectorN& vRight);
	VectorN& operator*=(const TYPE& scalar);

	//////////////////////////////////////////////////////////////////////
	// low-level element management

	// external element management
	void SetElements(int nDim, TYPE *pElements, bool bFreeElements);

	// copy elements from v, starting at start, for length elements, 
	//		and copy them to the destination position
	int CopyElements(const VectorN<TYPE>& v, int nStart, int nLength, 
		int nDest = 0);

};	// class VectorN<TYPE>


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
VectorN<TYPE>::VectorN() 
	: m_nDim(0),
		m_pElements(NULL),
		m_bFreeElements(TRUE)
	// default constructor
{
}	// VectorN<TYPE>::VectorN<TYPE>


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
VectorN<TYPE>::VectorN(int nDim) 
	: m_nDim(0),
		m_pElements(NULL),
		m_bFreeElements(TRUE)
	// construct an arbitrary dimensioned vector
{
	// set the dimensionality of the vector
	SetDim(nDim);

}	// VectorN<TYPE>::VectorN<TYPE>(int nDim) 


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
VectorN<TYPE>::VectorN(const VectorN<TYPE>& vFrom)
	: m_nDim(0),
		m_pElements(NULL),
		m_bFreeElements(TRUE)
	// copy constructor
{
	// set the dimensionality of the vector
	SetDim(vFrom.GetDim());

	// copy the elements
	(*this) = vFrom;

}	// VectorN<TYPE>::VectorN<TYPE>(const VectorN<TYPE>& vFrom)


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
VectorN<TYPE>::~VectorN()
	// destructor 
{
	if (m_bFreeElements 
		&& m_pElements != NULL)
	{
		// free elements
		FreeValues(m_pElements);
	}

}	// VectorN<TYPE>::~VectorN<TYPE>


//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
VectorN<TYPE>& 
	VectorN<TYPE>::operator=(const VectorN<TYPE>& vFrom)
	// assignment operator
{
	// check the dimensionality of the vector
	ASSERT(GetDim() == vFrom.GetDim());

	if (GetDim() > 0)
	{
		CopyValues(&(*this)[0], &vFrom[0], __min(GetDim(), vFrom.GetDim()));
	}

	// set remainder of elements to 0
	if (GetDim() > vFrom.GetDim())
	{
		// zero initial
		ZeroValues(&(*this)[vFrom.GetDim()], GetDim() - vFrom.GetDim());
	}

	// return a reference to this
	return (*this);

}	// VectorN<TYPE>::operator=


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
TYPE& 
	VectorN<TYPE>::operator[](int nAtRow)
	// returns a reference to the specified element.
{
	// check dimensions
	ASSERT(nAtRow >= 0 && nAtRow < GetDim());

	return m_pElements[nAtRow];

}	// VectorN<TYPE>::operator[]


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
const TYPE& 
	VectorN<TYPE>::operator[](int nAtRow) const
	// returns a const reference to the specified element.
{
	// check dimensions
	ASSERT(nAtRow >= 0 && nAtRow < GetDim());

	return m_pElements[nAtRow];

}	// VectorN<TYPE>::operator[] const


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
VectorN<TYPE>::operator TYPE *()
	// TYPE * conversion -- returns a pointer to the first element
	//		WARNING: this allows for no-bounds-checking access
{
	return &m_pElements[0];

}	// VectorN<TYPE>::operator TYPE *


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
VectorN<TYPE>::operator const TYPE *() const
	// const TYPE * conversion -- returns a pointer to the first 
	//		element.
	//		WARNING: this allows for no-bounds-checking access
{
	return &m_pElements[0];

}	// VectorN<TYPE>::operator const TYPE *


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
int 
	VectorN<TYPE>::GetDim() const
	// returns the dimensionality of the vector
{
	return m_nDim;

}	// VectorN<TYPE>::GetDim


//////////////////////////////////////////////////////////////////
template<class TYPE>
void 
	VectorN<TYPE>::SetDim(int nDim)
	// sets the dimensionality of this vector
{
	// do nothing if the dim is already correct
	if (m_nDim != nDim)
	{
		// store pointer to old elements
		TYPE *pOldElements = m_pElements;
		m_pElements = NULL;

		// set the new dimensionality
		int nOldDim = m_nDim;
		m_nDim = nDim;

		// allocate new elements, if needed
		if (m_nDim > 0)
		{
			m_pElements = NULL;
			AllocValues(m_nDim, m_pElements);

			if (pOldElements)
			{
				// copy the elements
				CopyValues(&(*this)[0], pOldElements, __min(GetDim(), nOldDim));
			}

			// set remainder of elements to 0
			if (GetDim() > nOldDim)
			{
				ZeroValues(&(*this)[nOldDim], GetDim() - nOldDim);
			}
		}

		// free the elements, if needed
		if (pOldElements != NULL)
		{
			FreeValues(pOldElements);
		}
	}

}	// VectorN<TYPE>::SetDim



//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
TYPE 
	VectorN<TYPE>::GetLength() const
	// returns the euclidean length of the vector
{
	return VectorLength(&(*this)[0], GetDim());

}	// VectorN<TYPE>::GetLength


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
void 
	VectorN<TYPE>::Normalize()
	// scales the vector so its length is 1.0
{
	TYPE len = GetLength();
	if (len > 0.0)
	{
		MultValues(&(*this)[0], (TYPE) 1.0 / len, GetDim());
	}

}	// VectorN<TYPE>::Normalize


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
bool 
	VectorN<TYPE>::IsApproxEqual(const VectorN<TYPE>& v, 
			TYPE epsilon) const
	// tests for approximate equality using the EPS defined at 
	//		the top of this file
{
	// form the difference vector
	VectorN<TYPE> vDiff(*this);
	vDiff -= v;

	return (vDiff.GetLength() < epsilon);

}	// VectorN<TYPE>::IsApproxEqual


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
VectorN<TYPE>& 
	VectorN<TYPE>::operator+=(const VectorN<TYPE>& vRight)
	// in-place vector addition; returns a reference to this	
{
	SumValues(&(*this)[0], &vRight[0], GetDim());

	return (*this);

}	// VectorN<TYPE>::operator+=


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
VectorN<TYPE>& 
	VectorN<TYPE>::operator-=(const VectorN<TYPE>& vRight)
	// in-place vector subtraction; returns a reference to this
{
	DiffValues(&(*this)[0], &vRight[0], GetDim());

	return (*this);

}	// VectorN<TYPE>::operator-=


//////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
VectorN<TYPE>& 
	VectorN<TYPE>::operator*=(const TYPE& scalar)
	// in-place scalar multiplication; returns a reference to this
{
	MultValues(&(*this)[0], scalar, GetDim());

	return (*this);

}	// VectorN<TYPE>::operator*=


//////////////////////////////////////////////////////////////////
template<class TYPE>
void 
	VectorN<TYPE>::SetElements(int nDim, TYPE *pElements,
			bool bFreeElements)
	// management for external elements
{
	if (m_bFreeElements 
		&& m_pElements != NULL)
	{
		delete [] m_pElements;
		m_pElements = NULL;
	}

	m_nDim = nDim;
	m_pElements = pElements;
	m_bFreeElements = bFreeElements;

}	// VectorN<TYPE>::SetElements


//////////////////////////////////////////////////////////////////
template<class TYPE>
int 
	VectorN<TYPE>::CopyElements(const VectorN<TYPE>& v, 
			int nStart, int nLength, int nDest)
	// copy elements from v, starting at start, for length elements, 
	//		and copy them to the destination position
{
	int nLastSrcPos = __min(nStart + nLength, v.GetDim());
	int nLastDstPos = __min(nDest + nLength, GetDim());
	nLength = __min(nLength, nLastSrcPos - nStart);
	nLength = __min(nLength, nLastDstPos - nDest);
	if (nLength > 0
		&& &(*this)[nDest] != &v[nStart])
	{
		CopyValues(&(*this)[nDest], &v[nStart], nLength);
	}

	return nLength;

}	// VectorN<TYPE>::CopyElements


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
bool 
	operator==(const VectorN<TYPE>& vLeft, 
			const VectorN<TYPE>& vRight)
	// friend function to provide exact equality comparison for vectors.
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

}	// operator==(const VectorN, const VectorN)


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
bool 
	operator!=(const VectorN<TYPE>& vLeft, 
			const VectorN<TYPE>& vRight)
	// friend function to provide exact inequality comparison for vectors.
	// use !IsApproxEqual for approximate inequality.
{
	return !(vLeft == vRight);

}	// operator!=(const VectorN, const VectorN)


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
VectorN<TYPE> 
	operator+(const VectorN<TYPE>& vLeft, 
			const VectorN<TYPE>& vRight)
	// friend function to add two vectors, returning the sum as a new 
	//		vector
{
	VectorN<TYPE> vSum(vLeft);
	vSum += vRight;

	return vSum;

}	// operator+(const VectorN, const VectorN)


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
VectorN<TYPE> 
	operator-(const VectorN<TYPE>& vLeft, 
			const VectorN<TYPE>& vRight)
	// friend function to subtract one vector from another, returning 
	//		the difference as a new vector
{
	VectorN<TYPE> vDiff(vLeft);
	vDiff -= vRight;

	return vDiff;

}	// operator-(const VectorN, const VectorN)


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
TYPE 
	operator*(const VectorN<TYPE>& vLeft, 
			const VectorN<TYPE>& vRight)
	// friend function for vector inner product
{
	return DotProduct(&vLeft[0], &vRight[0], vLeft.GetDim());

}	// operator*(const VectorN, const VectorN)


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
VectorN<TYPE> 
	operator*(const TYPE& scalar, 
			const VectorN<TYPE>& vRight)
	// friend function for scalar multiplication of a vector
{
	// copy vector to intermediate product value
	VectorN<TYPE> vProd(vRight);

	// in-place scalar multiplication
	vProd *= scalar;

	// return the result
	return vProd;

}	// operator*(TYPE scalar, const VectorN)


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
VectorN<TYPE> 
	operator*(const VectorN<TYPE>& vLeft, 
			const TYPE& scalar)
	// friend function for scalar multiplication of a vector
{
	// copy vector to intermediate product value
	VectorN<TYPE> vProd(vLeft);

	// in-place scalar multiplication
	vProd *= scalar;

	// return the result
	return vProd;

}	// operator*(const VectorN, TYPE scalar)


///////////////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE
void 
	CalcBinomialCoeff(VectorN<TYPE>& vCoeff)
	// calculates the binomial coefficients, returns in the vector
{
	VectorN<TYPE> vAltCoeff;
	vAltCoeff.SetDim(vCoeff.GetDim());

	for (int nAt = 0; nAt < vCoeff.GetDim()-1; nAt++)
	{
		vCoeff[0] = 1.0;
		for (int nAtElem = 0; nAtElem < nAt; nAtElem++)
		{
			vCoeff[nAtElem+1] = vAltCoeff[nAtElem] + vAltCoeff[nAtElem+1];
		}
		vCoeff[nAt+1] = 1.0;

		vAltCoeff = vCoeff;
	}

}	// CalcBinomialCoeff


//////////////////////////////////////////////////////////////////////
template<class TYPE>
void 
	TraceVector(const VectorN<TYPE>& vTrace)
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
#undef TRACE_VECTOR
#ifdef _DEBUG
#define TRACE_VECTOR(strMessage, v) \
	TRACE(strMessage);				\
	TraceVector(v);					\
	TRACE("\n");
#else
#define TRACE_VECTOR(strMessage, v)
#endif



#ifdef __AFX_H__

//////////////////////////////////////////////////////////////////////
template<class TYPE>
CArchive& 
	operator<<(CArchive &ar, VectorN<TYPE> v)
	// CArchive serialization of a vector
{
	// store the dimension first
	ar << v.GetDim();

	// store the elements
	for (int nAt = 0; nAt < v.GetDim(); nAt++)
	{
		ar << v[nAt];
	}

	return ar;

}	// operator<<(CArchive &ar, VectorN<TYPE> v)

//////////////////////////////////////////////////////////////////////
template<class TYPE>
CArchive& 
	operator>>(CArchive &ar, VectorN<TYPE>& v)
	// CArchive de-serialization of a vector
{
	// retrieve the dimension first
	int nDim;
	ar >> nDim;
	v.SetDim(nDim);

	// retrieve the elements
	for (int nAt = 0; nAt < v.GetDim(); nAt++)
	{
		ar >> v[nAt];
	}

	return ar;

}	// operator>>(CArchive &ar, VectorN<TYPE>& v)

#endif	// __AFX_H__


//////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
template<class TYPE>
void
	LogExprExt(const VectorN<TYPE> & vVec,
			const char *pszName, const char *pszModule)
	// helper function for XML logging of vectors
{
	// get the global log file
	CXMLLogFile *pLog = CXMLLogFile::GetLogFile();

	// only if we are logging --
	if (pLog->IsLogging())
	{
		// create a new expression element
		CXMLElement *pVarElem = pLog->NewElement("lx", pszModule);

		// if there is a name,
		if (strlen(pszName) > 0)
		{
			// set it.
			pVarElem->Attribute("name", pszName);
		}

		// set type to generice "CVector"
		pVarElem->Attribute("type", "CVector");

		// get the current format for the element type
		const char *pszFormat = pLog->GetFormat((TYPE) 0);
		for (int nAt = 0; nAt < vVec.GetDim(); nAt++)
		{
			// format each element
			pLog->Format(pszFormat, vVec[nAt]);
		}

		// done.
		pLog->GetLogFile()->CloseElement();
	}

}	// LogExprExt
#endif // _MSC_VER


// Backward compatibility
template<class TYPE = REAL>
using CVectorN = VectorN<TYPE>;

#endif	// !defined(VECTORN_H)