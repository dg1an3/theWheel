// SSELayoutManager.cpp: implementation of the CSSELayoutManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <math.h>
#include <mathf.h>
#include <xmmintrin.h>

#include "Node.h"
#include "Space.h"

#include "SSELayoutManager.h"

#define XMMFLOAT \
	__declspec(align(16)) float


#define START_ASM __asm {
#define END_ASM			}

#define ASM_FUNC(func) \
__declspec(naked) void func() {		\
	__asm

#define END_ASM_FUNC \
	__asm ret						\
}


///////////////////////////////////////////////////////////////////
// parameters
///////////////////////////////////////////////////////////////////

// max vector size
#define VEC_SIZE 256

// optimization parameters
#define EPS_SQ 0.001f
#define EPS_INV 3.0f
#define POS_SCALE 10.0f
#define REP_SCALE 20.0f


///////////////////////////////////////////////////////////////////
// shared structures for eval_obj and field_asm
///////////////////////////////////////////////////////////////////


// holds the x- y- coordinates, and returns dx- and dy-
typedef struct vec_xy_tag
{
	XMMFLOAT x[4];
	XMMFLOAT y[4];
	XMMFLOAT dx[4];
	XMMFLOAT dy[4];

} vec_xy_struct;


// holds the per-pair parameters
typedef struct ssx_ssy_tag
{
	XMMFLOAT ssx[4];
	XMMFLOAT ssy[4];
	XMMFLOAT weight_act[4];
	XMMFLOAT act[4];
} ssx_ssy_struct;



///////////////////////////////////////////////////////////////////
// shared variables for eval_obj and field_asm
///////////////////////////////////////////////////////////////////

// list of vectors
vec_xy_struct vec_xy[VEC_SIZE];

// list of per-pair parameters
ssx_ssy_struct ssx_ssy[VEC_SIZE * VEC_SIZE / 4];

// constants used during optimization
static XMMFLOAT opt_dist[4]; 
static XMMFLOAT posscale[4]; 
static XMMFLOAT repscale[4];
static XMMFLOAT eps_sq[] = { EPS_SQ, EPS_SQ, EPS_SQ, EPS_SQ };
static XMMFLOAT eps_inv[] = { EPS_INV, EPS_INV, EPS_INV, EPS_INV };

// temporary store for sum
static XMMFLOAT sum_all[4];

// used to return the sum
static XMMFLOAT sum;

// passes args in xmm_xterm, xmm_xshift, xmm_yterm, xmm_yshift
// returns args in xmm_dsum_dx, xmm_dsum_dy, and xmm_sum
// xmm_sum is not re-initialized



///////////////////////////////////////////////////////////////////
// register assignments for eval_obj
///////////////////////////////////////////////////////////////////

#define xmm_sum			xmm0		// in/out

#define xmm_xterm		xmm1		// in
#define xmm_xshift		xmm2		// in
#define xmm_yterm		xmm3		// in
#define xmm_yshift		xmm4		// in

#define xmm_xratio		xmm2		// local
#define xmm_yratio		xmm4		// local

#define xmm_term		xmm_xterm	// local

#define xmm_sqrt		xmm6		// local
#define xmm_pos			xmm3		// local
#define xmm_rep			xmm3		// local

#define xmm_dpos_dx		xmm5		// local
#define xmm_dpos_dy		xmm6		// local

#define xmm_drep_dx		xmm_xratio	// local
#define xmm_drep_dy		xmm_rep		// local

#define xmm_d_dx		xmm_dpos_dx	// out
#define xmm_d_dy		xmm_dpos_dy	// out


///////////////////////////////////////////////////////////////////
// eval_obj
// 
// function to evaluate a set of four pairs of values and return
// the objective function value 
///////////////////////////////////////////////////////////////////
ASM_FUNC(eval_obj)
{
	// x_diff
	subps xmm_xterm, xmm_xshift
	
	// x_diff * x_diff / ssx
	mulps xmm_xterm, xmm_xterm
	divps xmm_xterm, XMMWORD PTR [ecx]ssx_ssy.ssx

	// y_diff
	subps xmm_yterm, xmm_yshift

	// y_diff * y_diff / ssy
	mulps xmm_yterm, xmm_yterm
	divps xmm_yterm, XMMWORD PTR [ecx]ssx_ssy.ssy

	// x*x / ssx + y*y / ssy
	; movaps xmm_term, xmm_xterm
	addps xmm_term, xmm_yterm

	// xmm_pos = POS_SCALE * sqrtf(xmm_term + EPS_SQ)
	movaps xmm_pos, xmm_term
	addps xmm_pos, XMMWORD PTR [eps_sq]
	sqrtps xmm_pos, xmm_pos
	subps xmm_pos, XMMWORD PTR [opt_dist]
	mulps xmm_pos, xmm_pos
	mulps xmm_pos, XMMWORD PTR [posscale]
	mulps xmm_pos, XMMWORD PTR [ecx]ssx_ssy.weight_act
	addps xmm_sum, xmm_pos

	// xmm_rep = REP_SCALE / xmm_term
	movaps xmm_rep, XMMWORD PTR [repscale]
	mulps xmm_rep, XMMWORD PTR [ecx]ssx_ssy.act
	addps xmm_term, XMMWORD PTR [eps_inv]
	divps xmm_rep, xmm_term

	// sum both rep and pos to xmm_sum
	addps xmm_sum, xmm_rep

}	END_ASM_FUNC	// eval_obj



///////////////////////////////////////////////////////////////////
// eval_obj_grad
// 
// function to evaluate a set of four pairs of values and return
// the objective function value and the dx- and dy-
///////////////////////////////////////////////////////////////////
ASM_FUNC(eval_obj_grad)
{
	// x_diff
	subps xmm_xterm, xmm_xshift
	

	// x_diff * x_diff / ssx
	movaps xmm_xratio, xmm_xterm
	divps xmm_xratio, XMMWORD PTR [ecx]ssx_ssy.ssx
	mulps xmm_xterm, xmm_xratio

	// y_diff
	subps xmm_yterm, xmm_yshift

	// y_diff * y_diff / ssy
	movaps xmm_yratio, xmm_yterm
	divps xmm_yratio, XMMWORD PTR [ecx]ssx_ssy.ssy
	mulps xmm_yterm, xmm_yratio

	// x*x / ssx + y*y / ssy
	; movaps xmm_term, xmm_xterm
	addps xmm_term, xmm_yterm

	// xmm_pos = POS_SCALE * sqrtf(xmm_term + EPS_SQ)
	movaps xmm_pos, xmm_term
	addps xmm_pos, XMMWORD PTR [eps_sq]
	sqrtps xmm_pos, xmm_pos
	movaps xmm_sqrt, xmm_pos		// store sqrt
	subps xmm_pos, XMMWORD PTR [opt_dist]
	movaps xmm_dpos_dx, xmm_pos		// store opt_dist

	//. compute opt_dist * POS_SCALE * weight_act
	mulps xmm_dpos_dx, XMMWORD PTR [posscale]
	mulps xmm_dpos_dx, XMMWORD PTR [ecx]ssx_ssy.weight_act

	// compute opt_dist * POS_SCALE * weight_act * opt_dist
	mulps xmm_pos, xmm_dpos_dx
	addps xmm_sum, xmm_pos

	divps xmm_dpos_dx, xmm_sqrt
	movaps xmm_dpos_dy, xmm_dpos_dx
	mulps xmm_dpos_dx, xmm_xratio 
	; movaps xmm_d_dx, xmm_dpos_dx

	mulps xmm_dpos_dy, xmm_yratio
	; movaps xmm_d_dy, xmm_dpos_dy


	// xmm_rep = REP_SCALE / xmm_term
	movaps xmm_rep, XMMWORD PTR [repscale]
	mulps xmm_rep, XMMWORD PTR [ecx]ssx_ssy.act
	addps xmm_term, XMMWORD PTR [eps_inv]
	divps xmm_rep, xmm_term

	// sum both rep and pos to xmm_sum
	addps xmm_sum, xmm_rep

	divps xmm_rep, xmm_term
	mulps xmm_xratio, xmm_rep 
	; movaps xmm_drep_dx, xmm_xratio
	subps xmm_d_dx, xmm_drep_dx

	; movaps xmm_drep_dy, xmm_rep
	mulps xmm_drep_dy, xmm_yratio
	subps xmm_d_dy, xmm_drep_dy

}	END_ASM_FUNC	// eval_obj_grad


///////////////////////////////////////////////////////////////////
// register assignments for eval_obj_all
///////////////////////////////////////////////////////////////////

#define xmm_d_dx_shift  xmm1
#define xmm_d_dy_shift  xmm1
#define xmm_zeroes      xmm2



///////////////////////////////////////////////////////////////////
// helper macros for eval_obj_all
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// LOAD_XY_SHUF
//
//
///////////////////////////////////////////////////////////////////
#define LOAD_XY_SHUF(s3, s2, s1, s0) \
	END_ASM																\
																		\
	/* load x registers */												\
	__asm movaps xmm_xterm, XMMWORD PTR [eax]vec_xy.x					\
	__asm movaps xmm_xshift, XMMWORD PTR [ebx]vec_xy.x					\
	__asm shufps xmm_xshift, xmm_xshift, _MM_SHUFFLE(s3, s2, s1, s0)	\
																		\
	/* load y registers */												\
	__asm movaps xmm_yterm, XMMWORD PTR [eax]vec_xy.y					\
	__asm movaps xmm_yshift, XMMWORD PTR [ebx]vec_xy.y					\
	__asm shufps xmm_yshift, xmm_yshift, _MM_SHUFFLE(s3, s2, s1, s0)	\
																		\
	START_ASM


///////////////////////////////////////////////////////////////////
// LOAD_XY_SHUF
//
//
///////////////////////////////////////////////////////////////////
#define LOAD_XY() \
	END_ASM																\
																		\
	/* load x registers */												\
	__asm movaps xmm_xterm, XMMWORD PTR [eax]vec_xy.x					\
	__asm movaps xmm_xshift, XMMWORD PTR [ebx]vec_xy.x					\
																		\
	/* load y registers */												\
	__asm movaps xmm_yterm, XMMWORD PTR [eax]vec_xy.y					\
	__asm movaps xmm_yshift, XMMWORD PTR [ebx]vec_xy.y					\
																		\
	START_ASM


///////////////////////////////////////////////////////////////////
// SUM_HORIZ
//
//
///////////////////////////////////////////////////////////////////
#define SUM_HORIZ(xmm_dst, xmm_src) \
	END_ASM																\
																		\
	__asm movaps xmm_dst, xmm_src										\
																		\
	__asm shufps xmm_src, xmm_src, _MM_SHUFFLE(2, 1, 0, 3)				\
	__asm addps xmm_dst, xmm_src										\
																		\
	__asm shufps xmm_src, xmm_src, _MM_SHUFFLE(2, 1, 0, 3)				\
	__asm addps xmm_dst, xmm_src										\
																		\
	__asm shufps xmm_src, xmm_src, _MM_SHUFFLE(2, 1, 0, 3)				\
	__asm addps xmm_dst, xmm_src										\
																		\
	START_ASM



///////////////////////////////////////////////////////////////////
// eval_obj_all
// 
// function to evaluate a set of four pairs of values and return
// the objective function value
///////////////////////////////////////////////////////////////////
inline float eval_obj_all(long nCoords, long nConstNodes)
{
	// clear xmm_sum
	__asm xorps xmm_sum, xmm_sum

	// base offset and counter for shift operations
	vec_xy_struct *p_src = &vec_xy[(nCoords-1) / 4];
	vec_xy_struct *p_const = &vec_xy[(nConstNodes-1) / 4];

	// parameter block
	ssx_ssy_struct *p_ssx_ssy= &ssx_ssy[0];

	do
	{
		vec_xy_struct *p_dst = &vec_xy[(nCoords-1) / 4];
		while (p_dst >= p_src)
		{
			if (p_dst >= p_const)
			{
				START_ASM

				// initialize source and destination pointers
				mov eax, DWORD PTR [p_src]
				mov ebx, DWORD PTR [p_dst]

				// initialize parameter block pointer
				mov ecx, DWORD PTR [p_ssx_ssy]

				// call for first permutation
				LOAD_XY_SHUF(2, 1, 0, 3)
				call eval_obj

				// next parameter block
				add ecx, SIZE ssx_ssy_struct


				// call for second permutation
				LOAD_XY_SHUF(1, 0, 3, 2)
				call eval_obj

				// next parameter block
				add ecx, SIZE ssx_ssy_struct
				

				// call for third permutation
				LOAD_XY_SHUF(0, 3, 2, 1)
				call eval_obj

				// next parameter block
				add ecx, SIZE ssx_ssy_struct
				

				// call for final (no-shuffle) permutation
				LOAD_XY()
				call eval_obj

				// next parameter block, and store the pointer
				add ecx, SIZE ssx_ssy_struct
				mov DWORD PTR [p_ssx_ssy], ecx

				END_ASM
			}
			else
			{
				p_ssx_ssy += 4;
			}

			p_dst--;
		}

		p_src--;

	} while (p_src >= &vec_xy[0]);

	START_ASM

	// sum up the four individual sum elements
	SUM_HORIZ(xmm1, xmm_sum)

	// multiply sum by two, as we only iterated over one set of pairs
	movss DWORD PTR [sum], xmm1

	END_ASM

	return sum;

}	// eval_obj_all



///////////////////////////////////////////////////////////////////
// SUM_DX_DY_SHUF
//
//
///////////////////////////////////////////////////////////////////
#define SUM_DX_DY_SHUF(s3, s2, s1, s0) \
	END_ASM																\
																		\
	/* sum up dx */														\
	__asm movaps xmm_d_dx_shift, xmm_d_dx								\
	__asm addps xmm_d_dx, XMMWORD PTR [eax]vec_xy.dx					\
	__asm movaps XMMWORD PTR [eax]vec_xy.dx, xmm_d_dx					\
																		\
	__asm shufps xmm_d_dx_shift, xmm_d_dx_shift, _MM_SHUFFLE(s3, s2, s1, s0)	\
	__asm movaps xmm_d_dx, XMMWORD PTR [ebx]vec_xy.dx					\
	__asm subps xmm_d_dx, xmm_d_dx_shift								\
	__asm movaps XMMWORD PTR [ebx]vec_xy.dx, xmm_d_dx					\
																		\
	/* sum up dy */														\
	__asm movaps xmm_d_dy_shift, xmm_d_dy								\
	__asm addps xmm_d_dy, XMMWORD PTR [eax]vec_xy.dy					\
	__asm movaps XMMWORD PTR [eax]vec_xy.dy, xmm_d_dy					\
																		\
	__asm shufps xmm_d_dy_shift, xmm_d_dy_shift, _MM_SHUFFLE(s3, s2, s1, s0)	\
	__asm movaps xmm_d_dy, XMMWORD PTR [ebx]vec_xy.dy					\
	__asm subps xmm_d_dy, xmm_d_dy_shift								\
	__asm movaps XMMWORD PTR [ebx]vec_xy.dy, xmm_d_dy					\
																		\
	START_ASM



///////////////////////////////////////////////////////////////////
// SUM_DX_DY
//
//
///////////////////////////////////////////////////////////////////
#define SUM_DX_DY() \
	END_ASM																\
																		\
	/* sum up dx */														\
	__asm movaps xmm_d_dx_shift, xmm_d_dx								\
	__asm addps xmm_d_dx, XMMWORD PTR [eax]vec_xy.dx					\
	__asm movaps XMMWORD PTR [eax]vec_xy.dx, xmm_d_dx					\
																		\
	__asm movaps xmm_d_dx, XMMWORD PTR [ebx]vec_xy.dx					\
	__asm subps xmm_d_dx, xmm_d_dx_shift								\
	__asm movaps XMMWORD PTR [ebx]vec_xy.dx, xmm_d_dx					\
																		\
	/* sum up dy */														\
	__asm movaps xmm_d_dy_shift, xmm_d_dy								\
	__asm addps xmm_d_dy, XMMWORD PTR [eax]vec_xy.dy					\
	__asm movaps XMMWORD PTR [eax]vec_xy.dy, xmm_d_dy					\
																		\
	__asm movaps xmm_d_dy, XMMWORD PTR [ebx]vec_xy.dy					\
	__asm subps xmm_d_dy, xmm_d_dy_shift								\
	__asm movaps XMMWORD PTR [ebx]vec_xy.dy, xmm_d_dy					\
																		\
	START_ASM



///////////////////////////////////////////////////////////////////
// eval_obj_grad_all
// 
// function to evaluate a set of four pairs of values and return
// the objective function value and the dx- and dy-
///////////////////////////////////////////////////////////////////
inline float eval_obj_grad_all(long nCoords, long nConstNodes)
{
	// clear xmm_sum
	__asm xorps xmm_sum, xmm_sum

	// base offset and counter for shift operations
	vec_xy_struct *p_src = &vec_xy[(nCoords-1) / 4];
	vec_xy_struct *p_const = &vec_xy[(nConstNodes-1) / 4 -1];

	// parameter block
	ssx_ssy_struct *p_ssx_ssy= &ssx_ssy[0];

	do
	{
		vec_xy_struct *p_dst = &vec_xy[(nCoords-1) / 4];
		while (p_dst >= p_src)
		{
			if (p_dst >= p_const)
			{
				START_ASM

				// initialize source and destination pointers
				mov eax, DWORD PTR [p_src]
				mov ebx, DWORD PTR [p_dst]

				// initialize parameter block pointer
				mov ecx, DWORD PTR [p_ssx_ssy]

				// call for first permutation
				LOAD_XY_SHUF(2, 1, 0, 3)
				call eval_obj_grad
				SUM_DX_DY_SHUF(0, 3, 2, 1)

				// next parameter block
				add ecx, SIZE ssx_ssy_struct


				// call for second permutation
				LOAD_XY_SHUF(1, 0, 3, 2)
				call eval_obj_grad
				SUM_DX_DY_SHUF(1, 0, 3, 2)

				// next parameter block
				add ecx, SIZE ssx_ssy_struct
				

				// call for third permutation
				LOAD_XY_SHUF(0, 3, 2, 1)
				call eval_obj_grad
				SUM_DX_DY_SHUF(2, 1, 0, 3)

				// next parameter block
				add ecx, SIZE ssx_ssy_struct
				

				// call for final (no-shuffle) permutation
				LOAD_XY()
				call eval_obj_grad
				SUM_DX_DY()

				// next parameter block, and store the pointer
				add ecx, SIZE ssx_ssy_struct
				mov DWORD PTR [p_ssx_ssy], ecx

				END_ASM
			}
			else
			{
				p_ssx_ssy += 4;
			}

			p_dst--;
		}

		p_src--;

	} while (p_src >= &vec_xy[0]);

	START_ASM

	// sum up the four individual sum elements
	SUM_HORIZ(xmm1, xmm_sum)

	// multiply sum by two, as we only iterated over one set of pairs
	movss DWORD PTR [sum], xmm1

	END_ASM

	return sum;

}	// eval_obj_grad_all



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSSELayoutManager::CSSELayoutManager(CSpace *pSpace)
: CSpaceLayoutManager(pSpace)
{

}

CSSELayoutManager::~CSSELayoutManager()
{

}



//////////////////////////////////////////////////////////////////////
// CSSELayoutManager::LoadSizesLinks()
// 
// loads the sizes and links for quick access
//////////////////////////////////////////////////////////////////////
void CSSELayoutManager::LoadSizesLinks()
{
	CSpaceLayoutManager::LoadSizesLinks();

	// stores the computed sizes for the nodes, based on current act
	int nNodeCount = __min(GetStateDim() / 2, m_pSpace->GetNodeCount());
	int nConstNodes = m_vConstPositions.GetDim() / 2;

	// populate rotated matrix elements
	long nAt = 0;
	for (long nBlockRow = ((nNodeCount-1) / 4) * 4; nBlockRow >= 0; nBlockRow -= 4)
	{
		for (long nBlockCol = ((nNodeCount-1) / 4) * 4; nBlockCol >= nBlockRow; nBlockCol -= 4)
		{
			for (int nCol = 3; nCol >= 0; nCol--)
			{
				for (int nRow = 0; nRow < 4; nRow++)
				{
					int nMatrixRow = nBlockRow + nRow;
					int nMatrixCol = nBlockCol + (nRow+nCol) % 4;

					if (nMatrixRow < nNodeCount			// zero trailing elements
						&& nMatrixCol < nNodeCount
						&& (nMatrixCol >= nConstNodes	// and constant nodes
							|| nMatrixRow >= nConstNodes))
					{
						ssx_ssy[nAt].ssx[nRow] = m_mSSX[nMatrixCol][nMatrixRow];
						ssx_ssy[nAt].ssy[nRow] = m_mSSY[nMatrixCol][nMatrixRow];
						ssx_ssy[nAt].weight_act[nRow] = m_mLinks[nMatrixCol][nMatrixRow];
						ssx_ssy[nAt].act[nRow] = m_mAvgAct[nMatrixCol][nMatrixRow];
					}
					else
					{
						ssx_ssy[nAt].ssx[nRow] = 1.0;
						ssx_ssy[nAt].ssy[nRow] = 1.0;
						ssx_ssy[nAt].weight_act[nRow] = 0.0;
						ssx_ssy[nAt].act[nRow] = 0.0;
					}
				}
				nAt++;
			}
		}
	}

	for (nAt = 0; nAt < 4; nAt++)
	{
		opt_dist[nAt] = OPT_DIST;
		posscale[nAt] = GetKPos();
		repscale[nAt] = GetKRep();
	}

}	// CSpaceLayoutManager::LoadSizesLinks()


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::operator()
// 
// evaluates the energy function at the given point
//////////////////////////////////////////////////////////////////////
REAL CSSELayoutManager::operator()(const CVectorN<REAL>& vInput,
									 CVectorN<> *pGrad)
{
// #define _COMPARE_WITH_BASE
#ifdef _COMPARE_WITH_BASE
	static CVectorN<> vSubGrad;
	REAL sub_energy = CSpaceLayoutManager::operator()(vInput, &vSubGrad);
#endif

	// reset the energy
	m_energy = m_energyConst;

	if (NULL != pGrad)
	{
		// initialize the gradient vector to zeros
		m_vGrad.SetDim(GetStateDim());
		m_vGrad.SetZero();
	}

	// prepare the input vector for the full positional information
	ASSERT(m_vConstPositions.GetDim() + vInput.GetDim() == GetStateDim());
	m_vInput.SetDim(GetStateDim());
	m_vInput.CopyElements(m_vConstPositions, 0, 
		m_vConstPositions.GetDim());
	m_vInput.CopyElements(vInput, 0, vInput.GetDim(),
		m_vConstPositions.GetDim());

	// and the total number of nodes plus clusters
	int nNodeCount = __min(GetStateDim() / 2, m_pSpace->GetNodeCount());

	// compute the weighted center of the nodes, for the center repulsion
	REAL vCenter[2];
	REAL totalAct = 0.0;
	for (int nAt = 0; nAt < nNodeCount; nAt++)
	{
		vCenter[0] = m_act[nAt] * m_vInput[nAt * 2];
		vCenter[1] = m_act[nAt] * m_vInput[nAt * 2 + 1];
		totalAct += m_act[nAt];
	}

	// scale weighted center by total weight
	vCenter[0] /= totalAct;
	vCenter[1] /= totalAct;

	for (nAt = 0; nAt < nNodeCount; nAt++)
	{
		vec_xy[nAt / 4].x[nAt % 4] = m_vInput[nAt*2 + 0];
		vec_xy[nAt / 4].y[nAt % 4] = m_vInput[nAt*2 + 1];
		memset(vec_xy[nAt / 4].dx, 0, 
			sizeof(XMMFLOAT) * 8);
	}

	for (nAt = nNodeCount; nAt < (nNodeCount / 4 + 1) * 4; nAt++)
	{
		vec_xy[nAt / 4].x[nAt % 4] = 0.0;
		vec_xy[nAt / 4].y[nAt % 4] = 0.0;
		memset(vec_xy[nAt / 4].dx, 0, 
			sizeof(XMMFLOAT) * 8);
	}


	if (NULL != pGrad)
	{
		m_energy += eval_obj_grad_all(nNodeCount, m_vConstPositions.GetDim() / 2);

		for (nAt = 0; nAt < nNodeCount; nAt++)
		{
			m_vGrad[nAt*2 + 0] = 2.0 * vec_xy[nAt / 4].dx[nAt % 4];
			m_vGrad[nAt*2 + 1] = 2.0 * vec_xy[nAt / 4].dy[nAt % 4];
		}
	}
	else
	{
		m_energy += eval_obj_all(nNodeCount, m_vConstPositions.GetDim() / 2);
	}

	if (m_bCalcCenterRep)
	{
		// iterate over all current visualized node views
		int nAtNode;
		for (nAtNode = nNodeCount-1; nAtNode >= 0; nAtNode--)
		{
			//////////////////////////////////////////////////////////////
			// compute the centering repulsion field

			if (m_pSpace->GetNodeAt(nAtNode)->GetActivation() <= CENTER_REP_MAX_ACT)
			{
				// compute the x- and y-offset between the views
				const REAL x = m_vInput[nAtNode*2 + 0] - vCenter[0];
				const REAL y = m_vInput[nAtNode*2 + 1] - vCenter[1];

				// compute the x- and y-scales for the fields -- average of
				//		two rectangles
				const REAL aspect_sq = (13.0 / 16.0) * (13.0 / 16.0);
				const REAL ssx_sq =
					(CENTER_REP_SCALE * CENTER_REP_SCALE / aspect_sq );
				const REAL ssy_sq = 
					(CENTER_REP_SCALE * CENTER_REP_SCALE * aspect_sq );

				// compute the energy due to this interation
				const REAL dx_ratio = x / ssx_sq; 
				const REAL x_ratio = x * dx_ratio; 
				const REAL dy_ratio = y / ssy_sq;
				const REAL y_ratio = y * dy_ratio;

				// compute the energy term
				const REAL inv_sq = ((REAL) 1.0) / (x_ratio + y_ratio + ((REAL) 3.0));
				const REAL factor_rep = m_k_rep * CENTER_REP_WEIGHT
					* (CENTER_REP_MAX_ACT - abs(m_act[nAtNode])) 
					* (CENTER_REP_MAX_ACT - abs(m_act[nAtNode]));

				// add to total energy
				m_energy += factor_rep * inv_sq;

				if (NULL != pGrad)
				{
					// compute gradient terms
					const REAL inv_sq_sq = inv_sq * inv_sq;
					const REAL dRepulsion_dx = factor_rep * dx_ratio * inv_sq_sq;
					const REAL dRepulsion_dy = factor_rep * dy_ratio * inv_sq_sq;

					// add to the gradient vectors
					m_vGrad[nAtNode*2   + 0] -= (dRepulsion_dx + dRepulsion_dx);
					m_vGrad[nAtNode*2   + 1] -= (dRepulsion_dy + dRepulsion_dy);
				}
			} 
		}
	}
	
	// store the gradient vector if one was passed
	if (NULL != pGrad)
	{
		// set correct dimensionality
		pGrad->SetDim(vInput.GetDim());

		// and assign
		pGrad->CopyElements(m_vGrad, m_vConstPositions.GetDim(), vInput.GetDim());
	}

#ifdef _COMPARE_WITH_BASE
	if (abs(m_energy - sub_energy) > 1.0)
	{
		// output stats
		TRACE("Const nodes = %i\n", m_vConstPositions.GetDim() / 2);
		TRACE("Energy = %f | sub  = %f\n", m_energy, sub_energy);
		if (NULL != pGrad)
		{
			TRACE_VECTOR("Grad = ", (*pGrad));
			TRACE_VECTOR("Sub grad = ", vSubGrad);
		}
	}
#endif

	// increment evaluation count
	m_nEvaluations++;

	// return the total energy
	return m_energy;
}