//////////////////////////////////////////////////////////////////////
// Pseudoinverse.h: helper function for calculating M-P 
//		pseudo-inverse
//
// Copyright (C) 1999-2006 Derek G Lane
// $Id: MatrixNxM.h,v 1.1 2007/05/09 01:45:49 Derek Lane Exp $
//////////////////////////////////////////////////////////////////////

#pragma once

#include <MatrixNxM.h>

// constant for pseudo-inverse
const REAL PSEUDO_EPS = 1e-8f;

//////////////////////////////////////////////////////////////////
template<class TYPE>
bool 
	Pseudoinvert(const CMatrixNxM<TYPE>& mFrom, CMatrixNxM<TYPE>& mTo)
	// in-place Moore-Penrose pseudoinversion
{
	mTo.Reshape(mFrom.GetCols(), mFrom.GetRows());
	mTo = mFrom;

	CVectorN<TYPE> w(mTo.GetCols());
	CMatrixNxM<TYPE> v(mTo.GetCols(), mTo.GetCols());
	if (!SVD(mTo, w, v))
	{
		return FALSE;
	}

	// using the formula (A+)^T = U * {1/w} * V^T

	// form the S matrix (S^T * S)^-1 * S^T
	CMatrixNxM<TYPE> s(mTo.GetCols(), mTo.GetCols());
	for (int nAt = 0; nAt < mTo.GetCols(); nAt++)
	{
		s[nAt][nAt] = (w[nAt] > PSEUDO_EPS) ? 1.0 / w[nAt] : 0.0;
	}

	// now form final product
	mTo *= s;

	// multiply by V^T
	v.Transpose();
	mTo *= v;

	// and transpose the result to finish with A+
	mTo.Transpose();

	return TRUE;

}	// CMatrixNxM<TYPE>::Pseudoinvert


// maximum iterations for SVD
#define MAX_ITER 30

#define SQR(a) ((a)*(a))
#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

//////////////////////////////////////////////////////////////////////
template<class TYPE>
TYPE 
	pythag(TYPE a, TYPE b)
	// computes the c side of a triangle from a and b
{
	TYPE absa = (TYPE) fabs(a);
	TYPE absb = (TYPE) fabs(b);

	if (absa > absb)
	{
		return absa * sqrtf(1.0f + SQR(absb / absa));
	}
	else
	{
		return (absb == 0.0f ? 0.0f : absb * sqrtf(1.0f + SQR(absa / absb)));
	}

}	// pythag


//////////////////////////////////////////////////////////////////////
template<class TYPE>
bool 
	SVD(CMatrixNxM<>& mFrom, CVectorN<TYPE>& w, CMatrixNxM<TYPE>& v)
	// computes the singular-valued decomposition of this matrix,
	//		leaving U in the matrix and returning the singular values
	//		in w and v (not v^T)
{
	BEGIN_LOG_SECTION(CMatrixNxM::SVD);

	// stored reduction vector
	CVectorN<TYPE> rv1(mFrom.GetCols());

	// perform the householder reduction
	TYPE anorm = Householder(mFrom, w, rv1);
	LOG(_T("Householder result"));
	LOG_EXPR_EXT(w);
	LOG_EXPR_EXT(rv1);

	// accumulation of right-hand transformations
	AccumulateRH(mFrom, v, rv1);
	LOG(_T("AccumulateRH result"));
	LOG_EXPR_EXT(v);
	LOG_EXPR_EXT(rv1);

	// accumulation of left-hand transformations
	AccumulateLH(mFrom, w);
	LOG(_T("AccumulateLH result"));
	LOG_EXPR_EXT(w);

	// Diagonalization of the bidiagonal form
	for (int nK = mFrom.GetCols()-1; nK > 0; nK--)
	{
		int nIter;
		for (nIter = 1; nIter < MAX_ITER; nIter++)
		{
			BOOL bFlag = TRUE;
			int nM;
			int nL;
			for (nL = nK; nL > 0; nL--)
			{
				nM = nL - 1;

				if (((TYPE) fabs(rv1[nL]) + anorm) == anorm)
				{
					bFlag = FALSE;
					break;
				}
				if (nL > 0 
					&& ((TYPE) fabs(w[nL - 1]) + anorm) == anorm)
				{
					break;
				}
			}

			if (bFlag)
			{
				TYPE c = 0.0;
				TYPE s = 1.0;
				for (int nI = nL; nI <= nK; nI++)
				{
					TYPE f = s * rv1[nI];
					rv1[nI] = c * rv1[nI];

					if ((TYPE)(fabs(f) + anorm) == anorm)
					{
						break;
					}

					TYPE g = w[nI];
					TYPE h = pythag(f,g);
					w[nI] = h;
					h = 1.0f / h;
					c = g * h;
					s = -f * h;
					for (int nJ = 0; nJ < mFrom.GetRows(); nJ++)
					{
						TYPE y = mFrom[nM][nJ];
						TYPE z = mFrom[nI][nJ];
						mFrom[nM][nJ] = y * c + z * s;
						mFrom[nI][nJ] = z * c - y * s;
					}	// for
				}	// for
			}	// if

			TYPE z = w[nK];

			// test for convergence
			if (nL == nK)
			{
				if (z < 0.0)
				{
					// singular value is made non-negative
					w[nK] = -z;
					for (int nJ = 0; nJ < mFrom.GetCols(); nJ++)
					{
						v[nK][nJ] = -v[nK][nJ];
					}
				}

				break;

			}	// if

			TYPE x = w[nL];	// shift from bottom 2x2 minor
			TYPE y = w[nK - 1];

			TYPE g = rv1[nK - 1];
			TYPE h = rv1[nK];
			TYPE f = ((y - z) * (y + z) + (g - h) * (g + h)) 
				/ (2.0f * h * y);
			g = pythag<TYPE>(f, 1.0f);
			f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g,f))) - h))
				/ x;

			TYPE c = 1.0;	// next QR transformation:
			TYPE s = 1.0;		
			for (int nJ = 0; nJ <= nK - 1; nJ++)
			{
				g = rv1[nJ + 1];
				y = w[nJ + 1];
				h = s * g;
				g = c * g;
				z = pythag<TYPE>(f, h);
				rv1[nJ] = z;
				c = f / z;
				s = h / z;
				f = x * c + g * s;
				g = g * c - x * s;
				h = y * s;
				y *= c;
				int nJJ;
				for (nJJ = 0; nJJ < mFrom.GetCols(); nJJ++)
				{
					x = v[nJ][nJJ];
					z = v[nJ + 1][nJJ];
					v[nJ][nJJ] = x * c + z * s;
					v[nJ + 1][nJJ] = z * c - x * s;
				}
				z = pythag(f,h);
				w[nJ] = z;
				if (z != 0.0)
				{
					z = 1.0f/z;
					c = f * z;
					s = h * z;
				}
				f = c * g + s * y;
				x = c * y - s * g;
				for (nJJ = 0; nJJ < mFrom.GetRows(); nJJ++)
				{
					y = mFrom[nJ][nJJ];
					z = mFrom[nJ + 1][nJJ];
					mFrom[nJ][nJJ] = y * c + z * s;
					mFrom[nJ + 1][nJJ] = z * c - y * s;
				}
			}	// for

			rv1[nL] = 0.0;
			rv1[nK] = f;
			w[nK] = x;
		}

		if (nIter == MAX_ITER)
		{
			LOG(_T("SVD: no convergence in %i steps\n"), MAX_ITER);
			EXIT_LOG_SECTION();

			return FALSE;
		}
	}

	END_LOG_SECTION();	// CMatrixNxM::SVD

	return TRUE;

}	// CMatrixNxM<TYPE>::SVD


//////////////////////////////////////////////////////////////////////
template<class TYPE>
TYPE 
	Householder(CMatrixNxM<TYPE>& m, 
			CVectorN<TYPE>& w, 
			CVectorN<TYPE>& rv1)
	// helper function for SVD to perform Householder decomposition
{
	TYPE anorm = 0.0;

	// Householder reduction to bidiagonal form
	TYPE g = 0.0;
	TYPE scale = 0.0;
	for (int nI = 0; nI < m.GetCols(); nI++)
	{
		rv1[nI] = scale * g;
		g = scale = 0.0;
		if (nI <= m.GetRows()-1)
		{
			for (int nK = nI; nK < m.GetRows(); nK++)
			{
				scale += fabs(m[nI][nK]);
			}
			if (scale != 0.0)
			{
				TYPE s = 0.0;
				for (int nK = nI; nK < m.GetRows(); nK++)
				{
					m[nI][nK] /= scale;
					s += m[nI][nK] * m[nI][nK];
				}

				TYPE f = m[nI][nI];
				g = -SIGN(sqrt(s),f);
				TYPE h = f * g - s;
				m[nI][nI] = f - g;

				for (int nJ = nI + 1; nJ < m.GetCols(); nJ++)
				{
					TYPE s = 0.0;
					for (int nK = nI; nK < m.GetRows(); nK++)
					{
						s += m[nI][nK] * m[nJ][nK];
					}
					TYPE f = s / h;
					for (int nK = nI; nK < m.GetRows(); nK++)
					{
						m[nJ][nK] += f * m[nI][nK];
					}
				}	// for

				for (int nK = nI; nK < m.GetRows(); nK++)
				{
					m[nI][nK] *= scale;
				}
			}	// if
		}	// if

		w[nI] = scale * g;
		g = scale = 0.0;
		if (nI < m.GetRows() && nI != m.GetCols()-1)
		{
			for (int nK = nI + 1; nK < m.GetCols(); nK++)
			{
				scale += fabs(m[nK][nI]);
			}
			if (scale != 0.0)
			{
				TYPE s = 0.0;
				for (int nK = nI + 1; nK < m.GetCols(); nK++)
				{
					m[nK][nI] /= scale;
					s += m[nK][nI] * m[nK][nI];
				}

				TYPE f = m[nI + 1][nI];
				g = -SIGN(sqrt(s), f);
				TYPE h = f * g - s;
				m[nI + 1][nI] = f - g;

				for (int nK = nI + 1; nK < m.GetCols(); nK++)
				{
					rv1[nK] = m[nK][nI] / h;
				}

				for (int nJ = nI + 1; nJ < m.GetRows(); nJ++)
				{
					TYPE s = 0.0;
					for (int nK = nI + 1; nK < m.GetCols(); nK++)
					{
						s += m[nK][nJ] * m[nK][nI];
					}
					for (int nK = nI + 1; nK < m.GetCols(); nK++)
					{
						m[nK][nJ] += s * rv1[nK];
					}
				}

				for (int nK = nI + 1; nK < m.GetCols(); nK++)
				{
					m[nK][nI] *= scale;
				}
			}	// if
		}	// if

		anorm = __max(anorm, (fabs(w[nI]) + fabs(rv1[nI])));

	}	// for

	return anorm;

}	// CMatrixNxM<TYPE>::Householder


//////////////////////////////////////////////////////////////////////
template<class TYPE>
void 
	AccumulateRH(const CMatrixNxM<TYPE>& m, 
			CMatrixNxM<TYPE>& v, 
			const CVectorN<TYPE>& rv1)
	// helper function for SVD to accumulate right-hand products
{
	// Accumulation of right-hand transformations
	for (int nI = m.GetCols() - 2; nI >= 0; nI--)
	{
		v[nI + 1][nI + 1] = 1.0;

		if (rv1[nI + 1] != 0.0)
		{
			for (int nJ = nI + 1; nJ < m.GetCols(); nJ++)
			{
				v[nI][nJ] = (m[nJ][nI] / m[nI + 1][nI]) 
					/ rv1[nI + 1];
			}

			for (int nJ = nI + 1; nJ < m.GetCols(); nJ++)
			{
				TYPE s = 0.0;
				for (int nK = nI + 1; nK < m.GetCols(); nK++)
				{
					s += m[nK][nI] * v[nJ][nK];
				}

				for (int nK = nI + 1; nK < m.GetCols(); nK++)
				{
					v[nJ][nK] += s * v[nI][nK];
				}
			}	// for
		}	// if

		for (int nJ = nI + 1; nJ < m.GetCols(); nJ++)
		{
			v[nJ][nI] = v[nI][nJ] = 0.0;
		}
	}

}	// CMatrixNxM<TYPE>::AccumulateRH


//////////////////////////////////////////////////////////////////////
template<class TYPE>
void 
	AccumulateLH(CMatrixNxM<TYPE>& m, CVectorN<TYPE>& w)
	// helper function for SVD to accumulate left-hand products
{
	for (int nI = __min(m.GetRows()-1, m.GetCols()-1); nI >= 0; nI--)
	{
		for (int nJ = nI + 1; nJ < m.GetCols(); nJ++)
		{
			m[nJ][nI] = 0.0;
		}

		if (w[nI] != 0.0)
		{
			TYPE g = 1.0f / w[nI];
			for (int nJ = nI + 1; nJ < m.GetCols(); nJ++)
			{
				TYPE s = 0.0;
				for (int nK = nI + 1; nK < m.GetRows(); nK++)
				{
					s += m[nI][nK] * m[nJ][nK];
				}

				TYPE f = (s / m[nI][nI]) * g;
				for (int nK = nI; nK < m.GetRows(); nK++)
				{
					m[nJ][nK] += f * m[nI][nK];
				}

			}	// for

			for (int nJ = nI; nJ < m.GetRows(); nJ++)
			{
				m[nI][nJ] *= g;
			}
		}
		else
		{
			for (int nJ = nI; nJ < m.GetRows(); nJ++)
			{
				m[nI][nJ] = 0.0;
			}
		}

		++m[nI][nI];
	}

}	// CMatrixNxM<TYPE>::AccumulateLH
