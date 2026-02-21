//////////////////////////////////////////////////////////////////////
// MatrixD.h: declaration and definition of the CMatrixD template class.
//
// Copyright (C) 1999-2003 Derek G Lane
// $Id: MatrixD.h,v 1.1 2007/05/09 01:45:49 Derek Lane Exp $
//////////////////////////////////////////////////////////////////////

#if !defined(MATRIXD_H)
#define MATRIXD_H

#include <MathUtil.h>

#include "VectorD.h"

#include "MatrixOps.h"

//////////////////////////////////////////////////////////////////////
// class CMatrixD<DIM, TYPE>
//
// represents a square matrix with dimension and type given.
//////////////////////////////////////////////////////////////////////
template<int DIM = 4, class TYPE = REAL>
class CMatrixD
{
	// column vectors
	CVectorD<DIM, TYPE> m_arrColumns[DIM];

public:
	// constructors / destructore
	CMatrixD();
	CMatrixD(const CMatrixD& fromMatrix);
	~CMatrixD();

#ifdef DIRECT3D_VERSION
	// conversion to / from D3DMATRIX
	CMatrixD(const D3DMATRIX& matr);
	operator D3DMATRIX() const;
#endif

	// assignment operator
	CMatrixD& operator=(const CMatrixD& fromMatrix);

	// SetIdentity -- sets the matrix to an identity matrix
	void SetIdentity();

	// operator[] -- retrieves a reference to a Column vector
	CVectorD<DIM, TYPE>& operator[](int nAtCol);
	const CVectorD<DIM, TYPE>& operator[](int nAtCol) const;

	// TYPE * conversion -- returns a pointer to the first element
	//		WARNING: this allows for no-bounds-checking access
	operator TYPE *();
	operator const TYPE *() const;

	// matrix size
	int GetCols() const;
	int GetRows() const;

	// row-vector access
	void GetRow(int nAtRow, CVectorD<DIM, TYPE>& vRow) const;
	void SetRow(int nAtRow, const CVectorD<DIM, TYPE>& vRow);

	// helper typedefs
	typedef TYPE ELEM_TYPE;
	typedef CVectorD<DIM, TYPE> COL_TYPE;

	// IsApproxEqual -- tests for approximate equality using the EPS
	//		defined at the top of this file
	bool IsApproxEqual(const CMatrixD& m, 
		TYPE epsilon = DEFAULT_EPSILON) const;

	// in-place operators
	CMatrixD& operator+=(const CMatrixD& mRight);
	CMatrixD& operator-=(const CMatrixD& mRight);
	CMatrixD& operator*=(const TYPE& scale);
	CMatrixD& operator*=(const CMatrixD& mRight);

	// Transpose -- transposes elements of the matrix
	void Transpose();

	// matrix inversion
	bool Invert(bool bFlag = FALSE);

};	// class CMatrixD<DIM, TYPE>


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CMatrixD<DIM,TYPE>::CMatrixD()
	// default constructor -- initializes to an identity matrix
{
	// populate as an identity matrix
	SetIdentity();

}	// CMatrixD<DIM,TYPE>::CMatrixD<DIM,TYPE>


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CMatrixD<DIM,TYPE>::CMatrixD(const CMatrixD& fromMatrix)
	// copy constructor
{
	(*this) = fromMatrix;

}	// CMatrixD<DIM,TYPE>::CMatrixD<DIM,TYPE>


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CMatrixD<DIM,TYPE>::~CMatrixD()
	// destructor -- frees the row vectors
{
}	// CMatrixD<DIM,TYPE>::~CMatrixD<DIM,TYPE>


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CMatrixD<DIM,TYPE>& 
	CMatrixD<DIM,TYPE>::operator=(const CMatrixD<DIM,TYPE>& fromMatrix)
	// assignment operator
{
	CopyValues(&(*this)[0][0], &fromMatrix[0][0], DIM * DIM);

	return (*this);

}	// CMatrixD<DIM,TYPE>::operator=


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
void 
	CMatrixD<DIM, TYPE>::SetIdentity()
	// sets the matrix to an identity matrix
{
	ZeroValues(&(*this)[0][0], DIM * DIM);
	// memset(m_arrColumns, 0, sizeof(m_arrColumns));

	// for each element in the matrix,
	for (int nAt = 0; nAt < GetCols(); nAt++)
	{
		(*this)[nAt][nAt] = 1.0;
	}

}	// CMatrixD<TYPE>::SetIdentity


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE>& 
	CMatrixD<DIM,TYPE>::operator[](int nAtCol)
	// retrieves a reference to a Column vector
{
	// return a reference to the column vector
	return m_arrColumns[nAtCol];

}	// CMatrixD<DIM,TYPE>::operator[]


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
const CVectorD<DIM, TYPE>& 
	CMatrixD<DIM,TYPE>::operator[](int nAtCol) const
	// retrieves a const reference to a Column vector
{
	// return a reference to the column vector
	return m_arrColumns[nAtCol];

}	// CMatrixD<DIM,TYPE>::operator[] const


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
int 
	CMatrixD<DIM, TYPE>::GetCols() const
	// returns the number of columns of the matrix
{
	return DIM;

}	// CMatrixD<DIM, TYPE>::GetCols


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
int 
	CMatrixD<DIM, TYPE>::GetRows() const
	// returns the number of rows of the matrix
{
	return DIM;

}	// CMatrixD<DIM, TYPE>::GetRows


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
void 
	CMatrixD<DIM, TYPE>::GetRow(int nAtRow, 
			CVectorD<DIM, TYPE>& vRow) const
	// constructs and returns a row vector
{
	// populate the row vector
	for (int nAtCol = 0; nAtCol < GetCols(); nAtCol++)
	{
		vRow[nAtCol] = (*this)[nAtCol][nAtRow];
	}

}	// CMatrixBase<TYPE>::GetRow


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
void 
	CMatrixD<DIM, TYPE>::SetRow(int nAtRow, 
			const CVectorD<DIM, TYPE>& vRow)
	// sets the rows vector
{
	// de-populate the row vector
	for (int nAtCol = 0; nAtCol < GetCols(); nAtCol++)
	{
		(*this)[nAtCol][nAtRow] = vRow[nAtCol];
	}

}	// CMatrixBase<TYPE>::SetRow


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
bool 
	CMatrixD<DIM,TYPE>::IsApproxEqual(const CMatrixD<DIM,TYPE>& m, 
			TYPE epsilon) const
	// tests for approximate equality using the EPS defined at the 
	//		top of this file
{
	for (int nAtCol = 0; nAtCol < GetCols(); nAtCol++)
	{
		if (!(*this)[nAtCol].IsApproxEqual(m[nAtCol], epsilon))
		{
			return FALSE;
		}
	}

	return TRUE;

}	// CMatrixD<DIM,TYPE>::IsApproxEqual


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CMatrixD<DIM,TYPE>& 
	CMatrixD<DIM,TYPE>::operator+=(const CMatrixD<DIM,TYPE>& mRight)
	// in-place matrix addition; returns a reference to this
{
	SumValues(&(*this)[0][0], &mRight[0][0], DIM * DIM);

	// return a reference to this
	return (*this);

}	// CMatrixD<DIM,TYPE>::operator+=


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CMatrixD<DIM,TYPE>& 
	CMatrixD<DIM,TYPE>::operator-=(const CMatrixD& mRight)
	// in-place matrix subtraction; returns a reference to this
{
	DiffValues(&(*this)[0][0], &mRight[0][0], DIM * DIM);

	// return a reference to this
	return (*this);

}	// CMatrixD<DIM,TYPE>::operator-= 


//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
	CMatrixD<DIM,TYPE>& CMatrixD<DIM,TYPE>::operator*=(const TYPE& scale)
	// in-place scalar multiplication
{
	MultValues(&(*this)[0][0], scale, DIM * DIM);

	// return a reference to this
	return (*this);

}	// CMatrixD<DIM,TYPE>::operator*= 

	
//////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CMatrixD<DIM,TYPE>& 
	CMatrixD<DIM,TYPE>::operator*=(const CMatrixD<DIM,TYPE>& mRight)
	// in-place matrix multiplication; returns a reference to this
{
	// assign final product to this
	(*this) = (*this) * mRight;

	// return a reference to this
	return (*this);

}	// CMatrixD<DIM,TYPE>::operator*=


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
void 
	CMatrixD<DIM,TYPE>::Transpose()
	// transposes the matrix
{
	for (int nCol = 0; nCol < DIM; nCol++)
	{
		for (int nRow = nCol+1; nRow < DIM; nRow++)
		{
			TYPE temp = (*this)[nCol][nRow];
			(*this)[nCol][nRow] = (*this)[nRow][nCol];
			(*this)[nRow][nCol] = temp;
		}
	}

}	// CMatrixD<TYPE>::Transpose


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE						
bool 
	CMatrixD<DIM, TYPE>::Invert(bool bFlag)				
	// calls generic matrix Invert (from MatrixOps)
{															
	return ::Invert((*this));

}	// CMatrixD<DIM, TYPE>::Invert(bool bFlag)				



#ifdef USE_IPP

//////////////////////////////////////////////////////////////////////
#define DECLARE_MATRIXD_INVERT(TYPE, TYPE_IPP, DIM)		\
template<> INLINE														\
bool																	\
	CMatrixD<DIM, TYPE>::Invert(bool bFlag)								\
{																		\
	TYPE arrElements[DIM][DIM];											\
	__declspec(thread) static TYPE arrBuffer[2 * DIM][DIM];				\
	IppStatus stat = ippmInvert_m_##TYPE_IPP(&(*this)[0][0],			\
		DIM * sizeof(TYPE),												\
		DIM, 															\
		&arrBuffer[0][0],												\
		&arrElements[0][0], DIM * sizeof(TYPE));						\
	if (stat == ippStsOk)												\
	{																	\
		CopyValues(&(*this)[0][0], &arrElements[0][0],					\
			DIM * DIM);													\
	}																	\
	else if (stat == ippStsDivByZeroErr)								\
	{																	\
		TRACE("Singular matrix\n");										\
	}																	\
	return (stat == ippStsOk);											\
}

DECLARE_MATRIXD_INVERT(float, 32f, 2);
DECLARE_MATRIXD_INVERT(float, 32f, 3);
DECLARE_MATRIXD_INVERT(float, 32f, 4);
DECLARE_MATRIXD_INVERT(double, 64f, 2);
DECLARE_MATRIXD_INVERT(double, 64f, 3);
DECLARE_MATRIXD_INVERT(double, 64f, 4);

#endif


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
bool 
	operator==(const CMatrixD<DIM, TYPE>& mLeft, 
			const CMatrixD<DIM, TYPE>& mRight)
	// exact matrix equality
{
	// element-by-element comparison
	for (int nCol = 0; nCol < mLeft.GetCols(); nCol++)
	{
		if (mLeft[nCol] != mRight[nCol])
		{
			return false;
		}
	}

	return true;

}	// operator==(const CMatrixD<DIM, TYPE>&, const CMatrixD<DIM, TYPE>&)


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
bool 
	operator!=(const CMatrixD<DIM, TYPE>& mLeft, 
			const CMatrixD<DIM, TYPE>& mRight)
	// exact matrix inequality
{
	// call 
	return !(mLeft == mRight);

}	// operator==(const CMatrixD<DIM, TYPE>&, const CMatrixD<DIM, TYPE>&)


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CMatrixD<DIM, TYPE> 
	operator+(const CMatrixD<DIM, TYPE>& vLeft, 
			const CMatrixD<DIM, TYPE>& vRight)
	// friend function to add two vectors, returning the sum as a new 
	//		vector
{
	CMatrixD<DIM, TYPE> vSum = vLeft;
	vSum += vRight;

	return vSum;

}	// operator+(const CMatrixD, const CMatrixD)


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CMatrixD<DIM, TYPE> 
	operator-(const CMatrixD<DIM, TYPE>& vLeft, 
			const CMatrixD<DIM, TYPE>& vRight)
	// friend function to subtract one vector from another, returning 
	//		the difference as a new vector
{
	CMatrixD<DIM, TYPE> vSum = vLeft;
	vSum -= vRight;

	return vSum;

}	// operator-(const CMatrixD, const CMatrixD)


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CVectorD<DIM, TYPE> 
	operator*(const CMatrixD<DIM, TYPE>& mat,
			const CVectorD<DIM, TYPE>& v)
	// matrix-vector multiplication
{
	// stored the product
	CVectorD<DIM, TYPE> vProduct;
	MultMatrixVector(&vProduct[0], 
		&mat[0][0], mat.GetCols(), mat.GetRows(), &v[0]);

	// return the product
	return vProduct;

}	// operator*(const CMatrixD<DIM, TYPE>&, 
	//			const CVectorD<DIM, TYPE>&)


#ifdef USE_IPP

//////////////////////////////////////////////////////////////////////
template<> INLINE	
CVectorD<3, float> 
	operator*(const CMatrixD<3, float>& mat,		
			const CVectorD<3, float>& v)
	// matrix-vector multiplication over-rides
{																
	CVectorD<3, float> vProduct;
	CK_IPP(ippmMul_mTv_32f_3x3(&mat[0][0], 3 * sizeof(float),	
		&v[0],			
		&vProduct[0]));	
								
	return vProduct;

}	// operator*(const CMatrixD<3, float>& mat, 
	//			const CVectorD<3, float>& v)

//////////////////////////////////////////////////////////////////////
template<> INLINE	
CVectorD<4, float> 
	operator*(const CMatrixD<4, float>& mat,		
			const CVectorD<4, float>& v)
	// matrix-vector multiplication over-rides
{																
	CVectorD<4, float> vProduct;
	CK_IPP(ippmMul_mTv_32f_4x4(&mat[0][0], 4 * sizeof(float),	
		&v[0],			
		&vProduct[0]));	
								
	return vProduct;

}	// operator*(const CMatrixD<4, float>& mat, 
	//			const CVectorD<4, float>& v)

//////////////////////////////////////////////////////////////////////
template<> INLINE	
CVectorD<3, double> 
	operator*(const CMatrixD<3, double>& mat,		
			const CVectorD<3, double>& v)
	// matrix-vector multiplication over-rides
{																
	CVectorD<3, double> vProduct;
	CK_IPP(ippmMul_mTv_64f_3x3(&mat[0][0], 3 * sizeof(double),	
		&v[0],			
		&vProduct[0]));	
								
	return vProduct;

}	// operator*(const CMatrixD<3, double>& mat, 
	//			const CVectorD<3, double>& v)

//////////////////////////////////////////////////////////////////////
template<> INLINE	
CVectorD<4, double> 
	operator*(const CMatrixD<4, double>& mat,		
			const CVectorD<4, double>& v)
	// matrix-vector multiplication over-rides
{																
	CVectorD<4, double> vProduct;
	CK_IPP(ippmMul_mTv_64f_4x4(&mat[0][0], 4 * sizeof(double),	
		&v[0],			
		&vProduct[0]));	
								
	return vProduct;

}	// operator*(const CMatrixD<4, double>& mat, const CVectorD<4, double>& v)


#endif


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CMatrixD<DIM, TYPE> 
	operator*(const CMatrixD<DIM, TYPE>& mLeft, 
			const CMatrixD<DIM, TYPE>& mRight)
	// matrix multiplication
{
	// holds the final product
	CMatrixD<DIM, TYPE> mProduct;
	MultMatrixMatrix(mProduct, mLeft, mRight);

	// return the product
	return mProduct;

}	// operator*(const CMatrixD<DIM, TYPE>&, 
	//			const CMatrixD<DIM, TYPE>&)


#ifdef USE_IPP

//////////////////////////////////////////////////////////////////////
template<> INLINE													
CMatrixD<3, float> 
	operator*(const CMatrixD<3, float>& mLeft,		
			const CMatrixD<3, float>& mRight)			
	// matrix multiplication
{																	
	CMatrixD<3, float> mProduct;
	CK_IPP(ippmMul_mTmT_32f_3x3(&mLeft[0][0], 3 * sizeof(float),											
		&mRight[0][0], 3 * sizeof(float),							
		&mProduct[0][0], 3 * sizeof(float)));						
																	
	mProduct.Transpose();											
																	
	return mProduct;												

}	// operator*(const CMatrixD<3, float>& mLeft,		
	//			const CMatrixD<3, float>& mRight)			

//////////////////////////////////////////////////////////////////////
template<> INLINE													
CMatrixD<4, float> 
	operator*(const CMatrixD<4, float>& mLeft,		
			const CMatrixD<4, float>& mRight)			
	// matrix multiplication
{																	
	CMatrixD<4, float> mProduct;
	CK_IPP(ippmMul_mTmT_32f_4x4(&mLeft[0][0], 4 * sizeof(float),											
		&mRight[0][0], 4 * sizeof(float),							
		&mProduct[0][0], 4 * sizeof(float)));						
																	
	mProduct.Transpose();											
																	
	return mProduct;												

}	// operator*(const CMatrixD<4, float>& mLeft,		
	//			const CMatrixD<4, float>& mRight)			

//////////////////////////////////////////////////////////////////////
template<> INLINE													
CMatrixD<3, double> 
	operator*(const CMatrixD<3, double>& mLeft,		
			const CMatrixD<3, double>& mRight)			
{																	
	CMatrixD<3, double> mProduct;
	CK_IPP(ippmMul_mTmT_64f_3x3(&mLeft[0][0], 3 * sizeof(double),											
		&mRight[0][0], 3 * sizeof(double),							
		&mProduct[0][0], 3 * sizeof(double)));						
																	
	mProduct.Transpose();											
																	
	return mProduct;												

}	// operator*(const CMatrixD<3, double>& mLeft,		
	//			const CMatrixD<3, double>& mRight)			

//////////////////////////////////////////////////////////////////////
template<> INLINE													
CMatrixD<4, double> 
	operator*(const CMatrixD<4, double>& mLeft,		
			const CMatrixD<4, double>& mRight)			
{																	
	CMatrixD<4, double> mProduct;
	CK_IPP(ippmMul_mTmT_64f_4x4(&mLeft[0][0], 4 * sizeof(double),											
		&mRight[0][0], 4 * sizeof(double),							
		&mProduct[0][0], 4 * sizeof(double)));						
																	
	mProduct.Transpose();											
																	
	return mProduct;												

}	// operator*(const CMatrixD<4, double>& mLeft,		
	//			const CMatrixD<4, double>& mRight)			

#endif


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CMatrixD<DIM, TYPE> 
	operator*(const CMatrixD<DIM, TYPE>& mLeft, 
			const TYPE& scalar)
	// matrix multiplication
{
	// create the product
	CMatrixD<DIM, TYPE> mProduct(mLeft);
	mProduct *= scalar;

	// return the product
	return mProduct;

}	// operator*(const CMatrixD<DIM, TYPE>&, const CMatrixD<DIM, TYPE>&)

	
//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
CMatrixD<DIM, TYPE> 
	operator*(const TYPE& scalar, 
			const CMatrixD<DIM, TYPE>& mRight)
	// matrix multiplication
{
	// create the product
	CMatrixD<DIM, TYPE> mProduct(mRight);
	mProduct *= scalar;

	// return the product
	return mProduct;

}	// operator*(const CMatrixD<DIM, TYPE>&, const CMatrixD<DIM, TYPE>&)


#ifdef __AFX_H__

//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CArchive& 
	operator<<(CArchive &ar, CMatrixD<DIM, TYPE> m)
	// matrix serialization
{
	// serialize the individual row vectors
	for (int nAt = 0; nAt < DIM; nAt++)
	{
		ar << m[nAt];
	}

	// return the archive object
	return ar;

}	// operator<<


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CArchive& 
	operator>>(CArchive &ar, CMatrixD<DIM, TYPE>& m)
	// matrix serialization
{
	// serialize the individual row vectors
	for (int nAt = 0; nAt < DIM; nAt++)
	{
		ar >> m[nAt];
	}

	// return the archive object
	return ar;

}	// operator>>

#endif 


//////////////////////////////////////////////////////////////////////
INLINE 
CMatrixD<3> 
	CreateTranslate(const double& s, 
			const CVectorD<2>& vAxis)
	// creates a 3x3 homogeneous translation matrix from a vector and 
	//		a scalar for a 2D translation
{
	// start with an identity matrix
	CMatrixD<3> mTranslate;

	mTranslate[2][0] = (REAL) s * vAxis[0];
	mTranslate[2][1] = (REAL) s * vAxis[1];

	return mTranslate;

}	// CreateTranslate


//////////////////////////////////////////////////////////////////////
INLINE 
CMatrixD<4> 
	CreateTranslate(const double& s, 
			const CVectorD<3>& vAxis)
	// creates a 4x4 homogeneous translation matrix from a vector and 
	//		a scalar for a 3D translation
{
	// start with an identity matrix
	CMatrixD<4> mTranslate;

	mTranslate[3][0] = (REAL) s * vAxis[0];
	mTranslate[3][1] = (REAL) s * vAxis[1];
	mTranslate[3][2] = (REAL) s * vAxis[2];

	return mTranslate;

}	// CreateTranslate


//////////////////////////////////////////////////////////////////////
INLINE 
CMatrixD<4> 
	CreateTranslate(const CVectorD<3>& vAxis)
	// creates a translation matrix from an offset vector
{
	return CreateTranslate(1.0, vAxis);

}	// CreateTranslate


//////////////////////////////////////////////////////////////////////
INLINE 
CMatrixD<2> 
	CreateRotate(const double& theta)
	// creates a 2D rotation matrix given an angle
{
	// start with an identity matrix
	CMatrixD<2> mRotate;

	// now compute the rotation matrix

	// angle helper values
	REAL st = (REAL) sin(theta);
	REAL ct = (REAL) cos(theta);

	mRotate[0][0] = ct;
	mRotate[1][0] = - st;

	mRotate[0][1] = st;
	mRotate[1][1] = ct;

	return mRotate;

}	// CreateRotate


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE 
CMatrixD<3, TYPE> 
	CreateRotate(const TYPE& theta, 
			const CVectorD<3, TYPE>& vAxis)
	// creates a 3D rotation matrix given an angle and an axis of rotation
{
	// start with an identity matrix
	CMatrixD<3, TYPE> mRotate;

	// normalize the axis
	CVectorD<3, TYPE> vNormAxis = vAxis;
	vNormAxis.Normalize();

	// now compute the rotation matrix

	// angle helper values
	TYPE st = sin(theta);
	TYPE ct = cos(theta);
	TYPE vt = (TYPE) 1.0 - cos(theta);

	mRotate[0][0] = vNormAxis[0] * vNormAxis[0] * vt + ct;
	mRotate[1][0] = vNormAxis[0] * vNormAxis[1] * vt - vNormAxis[2] * st;
	mRotate[2][0] = vNormAxis[0] * vNormAxis[2] * vt + vNormAxis[1] * st;

	mRotate[0][1] = vNormAxis[0] * vNormAxis[1] * vt + vNormAxis[2] * st;
	mRotate[1][1] = vNormAxis[1] * vNormAxis[1] * vt + ct;
	mRotate[2][1] = vNormAxis[1] * vNormAxis[2] * vt - vNormAxis[0] * st;

	mRotate[0][2] = vNormAxis[0] * vNormAxis[2] * vt - vNormAxis[1] * st;
	mRotate[1][2] = vNormAxis[1] * vNormAxis[2] * vt + vNormAxis[0] * st;
	mRotate[2][2] = vNormAxis[2] * vNormAxis[2] * vt + ct;

	return mRotate;

}	// CreateRotate


//////////////////////////////////////////////////////////////////////
template<class TYPE> INLINE 
CMatrixD<3, TYPE> 
	CreateRotate(const CVectorD<3, TYPE>& vInitPt,
			const CVectorD<3, TYPE>& vFinalPt, TYPE scale)
	// creates a 3D rotation matrix given two positions: a pre-rotated
	//		position and a post-rotated position. scale is a scale factor
	//		for the resulting rotation.
{
	// compute the length of the two legs of the triangle (a & b)
	TYPE a = vInitPt.GetLength(); 
	TYPE b = vFinalPt.GetLength();
	
	// compute the length of the segment connecting the initial and final
	//		point (c)
	TYPE c = (vFinalPt - vInitPt).GetLength();

	// compute the angle theta using the law of cosines
	TYPE theta = 0.0;
	if (a * b != 0.0)
	{
		theta = (2.0 * PI - acos((a * a + b * b - c * c) 
			/ (2.0 * a * b)));
	}

	// compute the axis of rotation = normalized cross product of 
	//		initial and current drag points
	CVectorD<3, TYPE> u = Cross(vFinalPt, vInitPt);
	u.Normalize();

	// now compute the rotation matrix
	return CreateRotate(scale * theta, u);

}	// CreateRotate


//////////////////////////////////////////////////////////////////////
INLINE
CMatrixD<2> 
	CreateScale(const CVectorD<2>& vScale)
	// creates a scaling matrix from a vector whose element's lengths 
	//		are scale factors
{
	// start with an identity matrix
	CMatrixD<2> mScale;

	mScale[0][0] = vScale[0];
	mScale[1][1] = vScale[1];

	return mScale;

}	// CreateScale


//////////////////////////////////////////////////////////////////////
INLINE
CMatrixD<3> 
	CreateScale(const CVectorD<3>& vScale)
	// creates a scaling matrix from a vector whose element's lengths 
	//		are scale factors
{
	// start with an identity matrix
	CMatrixD<3> mScale;

	mScale[0][0] = vScale[0];
	mScale[1][1] = vScale[1];
	mScale[2][2] = vScale[2];

	return mScale;

}	// CreateScale


//////////////////////////////////////////////////////////////////////
INLINE 
CMatrixD<4> 
	CreateProjection(const double& n, const double& f)
	// creates a projection matrix from a near-plane and a far-plane
{
	CMatrixD<4> mProj;

	// set the near plane elements
	mProj[0][0] = (REAL) n;
	mProj[1][1] = (REAL) n;

	// set the ratio elements
	mProj[2][2] = (REAL) (-(f + n)	  / (f - n));
	mProj[3][2] = (REAL) (-2.0 * f * n / (f - n));

	// set the projection elements
	mProj[2][3] = (REAL) -1.0;
	mProj[3][3] = (REAL)  0.0;

	// return the created matrix
	return mProj;

}	// CreateProjection


#ifdef DIRECT3D_VERSION

//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CMatrixD<DIM, TYPE>::CMatrixD(const D3DMATRIX& matr)
	// constructs from a D3DMATRIX
{
	// we have to assign the columns before calling 
	//		CMatrixBase::SetElements, because otherwise
	//		SetElements will allocate the column vectors
	//		(and ours are part of the CMatrixD)
	m_pColumns = &m_arrColumns[0];
	m_nCols = DIM;

	// allocate the elements
	SetElements(DIM, DIM, &m_arrElements[0], FALSE);

	// assign matrix elements
	(*this)[0][0] = matr._11;

	if (DIM > 1)
	{
		(*this)[0][1] = matr._12;

		(*this)[1][0] = matr._21;
		(*this)[1][1] = matr._22;
	}

	if (DIM > 2)
	{
		(*this)[0][2] = matr._13;
		(*this)[1][2] = matr._23;

		(*this)[2][0] = matr._31;
		(*this)[2][1] = matr._32;
		(*this)[2][2] = matr._33;
	}

	if (DIM > 3)
	{
		(*this)[0][3] = matr._14;
		(*this)[1][3] = matr._24;
		(*this)[2][3] = matr._34;

		(*this)[3][0] = matr._41;
		(*this)[3][1] = matr._42;
		(*this)[3][2] = matr._43;
		(*this)[3][3] = matr._44;
	}

}	// CMatrixD<DIM, TYPE>::CMatrixD(const D3DMATRIX& matr)


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE>
CMatrixD<DIM, TYPE>::operator D3DMATRIX() const
	// converts to a D3DMATRIX
{
	D3DMATRIX matr;

	matr._11 = (*this)[0][0];

	if (DIM > 1)
	{
		matr._12 = (*this)[0][1];

		matr._21 = (*this)[1][0];
		matr._22 = (*this)[1][1];
	}

	if (DIM > 2)
	{
		matr._13 = (*this)[0][2];
		matr._23 = (*this)[1][2];

		matr._31 = (*this)[2][0];
		matr._32 = (*this)[2][1];
		matr._33 = (*this)[2][2];
	}

	if (DIM > 3)
	{
		matr._14 = (*this)[0][3];
		matr._24 = (*this)[1][3];
		matr._34 = (*this)[2][3];

		matr._41 = (*this)[3][0];
		matr._42 = (*this)[3][1];
		matr._43 = (*this)[3][2];
		matr._44 = (*this)[3][3];
	}

	return matr;

}	// CMatrixD<DIM, TYPE>::operator D3DMATRIX()

#endif


//////////////////////////////////////////////////////////////////////
template<int DIM, class TYPE> INLINE
TYPE 
	Determinant(const CMatrixD<DIM, TYPE>& mMat)
	// computes the determinant of the matrix, for square matrices
	// TODO: move this to MatrixOps
	// TODO: implement IPP determinant
{
	TYPE det = 0.0;
	for (int nAtCol = 0; nAtCol < mMat.GetCols(); nAtCol++) 
	{
		CMatrixD<DIM-1, TYPE> mMinor;
		for (int nAtRow = 1; nAtRow < mMat.GetRows(); nAtRow++) 
		{
			int nAtMinorCol = 0;
			for (int nAtCol2 = 0; nAtCol2 < mMat.GetCols(); nAtCol2++) 
			{
			   if (nAtCol2 != nAtCol)
			   {
				   mMinor[nAtMinorCol][nAtRow-1] = mMat[nAtCol2][nAtRow];
				   nAtMinorCol++;
			   }
			}
		}
		det += ((nAtCol % 2 == 0) ? 1.0 : -1.0) 
			* mMat[nAtCol][0] * Determinant(mMinor);
	}

	return det;

}	// CMatrixBase<TYPE>::Determinant

template<> INLINE
float 
	Determinant(const CMatrixD<2, float>& mMat)
{
	return mMat[0][0] * mMat[1][1] - mMat[1][0] * mMat[0][1];

}	// CMatrixBase<TYPE>::Determinant


template<> INLINE
double 
	Determinant(const CMatrixD<2, double>& mMat)
{
	return mMat[0][0] * mMat[1][1] - mMat[1][0] * mMat[0][1];

}	// CMatrixBase<TYPE>::Determinant


template<> INLINE
float 
	Determinant(const CMatrixD<1, float>& mMat)
{
	return mMat[0][0];

}	// CMatrixBase<TYPE>::Determinant

template<> INLINE
double 
	Determinant(const CMatrixD<1, double>& mMat)
{
	return mMat[0][0];

}	// CMatrixBase<TYPE>::Determinant


//////////////////////////////////////////////////////////////////////
INLINE
REAL 
	Eigen(CMatrixD<2> m, int nOrder = 1, 
			CVectorD<2> *pVector = NULL)
	// computes the eigenvector of a 2x2 matrix, or returns <0.0, 0.0> if
	//		no real-valued eigenvectors exist
	// order is either 1 or 2, depending on whether the principle or
	//		remaining eigenvector is wanted
{
	// compute factors of the characteristic quadratic equation
	REAL b = -(m[0][0] + m[1][1]);
	REAL c = m[0][0] * m[1][1] - m[0][1] * m[1][0];

	// check to see if characteristic equation has real roots
	if (b * b < 4.0 * c)
	{
		// if not, return 0.0 to indicate no eigenvalue computed
		return 0.0;
	}

	// and find the roots of the equation
	REAL r1 = (REAL) -((b + sqrt(b * b - 4.0 * c)) / 2.0);
	REAL r2 = (REAL) -((b - sqrt(b * b - 4.0 * c)) / 2.0);

	// choose root which is either largest or smallest, depending on order
	REAL r = ((fabs(r1) > fabs(r2)) && (nOrder <= 1)) 
		? r1 : r2;

	// find the larger off-diagonal element
	if (fabs(m[1][0]) > fabs(m[0][1]))
	{
		// and solve for the vector 
		(*pVector)[0] = 1.0;
		(*pVector)[1] = (r - m[0][0]) / m[1][0];
	}
	// otherwise use the other element if it is non-zero
	else if (fabs(m[0][1]) > 0.0)
	{
		// and solve for the vector 
		(*pVector)[0] = (r - m[1][1]) / m[0][1];
		(*pVector)[1] = 1.0;
	}
	// otherwise,
	else
	{
		// if not, return 0.0 to indicate no eigenvalue computed
		return 0.0;
	}

	// normalize the computed eigenvector
	pVector->Normalize();

#ifdef _DEBUG
	// assert that our eigenvector is really an eigenvector
	CVectorD<2> vLeft = m * (*pVector);
	CVectorD<2> vRight = r * (*pVector);

	ASSERT(vLeft.IsApproxEqual(vRight));
#endif

	// return the computed eigenvalue
	return r;

}	// Eigenvector


#ifdef _MSC_VER
//////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
template<int DIM, class TYPE>
void
	LogExprExt(const CVectorD<DIM, TYPE> & vVec,
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
		const char *pszFormat = pLog->GetFormat((VECTOR_TYPE::ELEM_TYPE) 0);
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


#endif