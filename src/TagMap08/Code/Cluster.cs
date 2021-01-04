//////////////////////////////////////////////////////////////////////
// Cluster.h: interface for the CCluster class.
//
// Copyright (C) 2003 Derek G Lane
// $Id: Cluster.h,v 1.1 2003/04/29 02:50:12 default Exp $
//////////////////////////////////////////////////////////////////////

// need general float stuff
// #include <float.h>

// vector is default template class
// #include <VectorD.h>

// forward define
//template<typename TYPE>
//class CCluster;
using System.Collections.Generic;

namespace TagMap08.Code
{

    //////////////////////////////////////////////////////////////////////
    // class CDistanceFunction
    // 
    // a distance function must be provided for the cluster analysis
    //////////////////////////////////////////////////////////////////////

#if NEVER
template<typename TYPE>
class CDistanceFunction
{
public:
	virtual double operator()(const CCluster<TYPE> *pRight, 
		const CCluster<TYPE> *pLeft) = 0;
};
#endif

    //////////////////////////////////////////////////////////////////////
    // class CCluster
    // 
    // represents a single cluster within a cluster analysis.  don't 
    // construct directly, just call the static function Analyze.
    //////////////////////////////////////////////////////////////////////
    class CCluster
    {
        // constructor / destructor
        public CCluster(Tag tag)
        {
        }

        public CCluster(CCluster right, CCluster left)
        {
            SetChildren(right, left);
        }

        // accessors for labels
        public string Label;

        // accessors for cluster value (mean if an amalgamation)
        public Vector Value
        {
            get
            {
                // see if the value (mean) needs to be computed
                if (Right != null
                    && Left != null)
                {
                    // determine the weights for right and left child values
                    Vector valueRight = Right.Value;
                    float weightRight = (float)
                        (1 + Right.DescendantCount)
                        / (float)DescendantCount;
                    valueRight.Scale(weightRight);

                    Vector valueLeft = Left.Value;
                    float weightLeft = (float)(1 + Left.DescendantCount)
                        / (float)DescendantCount;
                    valueLeft.Scale(weightLeft);

                    valueLeft.Add(valueRight);
                    return valueLeft;
                }
                return null;
            }
        }

        public delegate float DistanceFunction(CCluster right, CCluster left);


        // returns descendant count (all children, grandchildren, etc.)
        public int DescendantCount;

        // child accessors
        public CCluster Right;
        public CCluster Left;

        // returns the depth (levels of children)
        public int Depth
        {
            get
            {
                if (Right == null)
                    return 0;

                return System.Math.Max(Left.Depth, Right.Depth) + 1;
            }
        }

        // returns the distance measure between the two children
        public float Dist;

        // static function to create the analysis tree from an array of TYPE
        public static CCluster Analyze(IEnumerable<Tag> tags, DistanceFunction func)
        {
            // allocate all of the clusters for the analysis
            List<CCluster> arrClusters = new List<CCluster>();

            // declare the array of pointers for remaining (unamalgamated) clusters
            List<CCluster> arrRemainClusters = new List<CCluster>();

            // initialize the leaf clusters and add to remaining pointer array
            foreach (Tag tag in tags)
            {
                arrRemainClusters.Add(new CCluster(tag));
            }

            // the next cluster specifies where to place the next
            //		amalgamation
            int nNextCluster = nCount;

            // as long as more than one cluster remains to be amalgamated
            while (arrRemainClusters.Count > 2)
            {
                // these store the indices of the two children
                int nChild1 = 0;
                int nChild2 = 0;

                // initialize the next amalgamation cluster
                arrClusters.Add(new CCluster(null, null, System.Single.MaxValue));

                foreach (CCluster cluster in arrRemainCluster)
                {
                    // stores info about closest cluster
                    float dist = 0.0f;

                    // find the closest cluster to the current one
                    CCluster closest = cluster.FindClosest(arrRemainClusters.ToArray(), nAt + 1,
                        ref dist, func);

                    // see if the distance is less than minimum found thus far
                    if (dist < arrClusters[nNextCluster].Dist)
                    {
                        // if so, store the indices of the children
                        nChild1 = nAt;
                        nChild2 = nAtClosest;

                        // and set the children in the current amalgamation
                        cluster.SetChildren(
                            arrRemainClusters[nChild1],
                            closest, dist);
                    }
                }

                // finally, removing the children from the remaining list
                //		and adding new amalgamation
                arrRemainClusters[nChild1] = arrClusters[nNextCluster];
                arrRemainClusters.RemoveAt(nChild2);

                // move to the next available amalgamation
                nNextCluster++;
            }

            // compute the distance between two remaining clusters
            float dist2 = func(arrRemainClusters[0], arrRemainClusters[1]);

            // create the final root cluster
            CCluster pRootCluster = new CCluster();

            // set two children of root 
            pRootCluster.SetChildren(arrRemainClusters[0], arrRemainClusters[1], dist2);

            // set the AllClusters pointer to the top-most amalgation
            //pRootCluster.m_pAllClusters = arrClusters;

            // and return the topmost amalgamation
            return pRootCluster;

        }

        // sets the children and distance
        void SetChildren(CCluster pRight, CCluster pLeft, float dist)
        {
            // either both must be NULL, or neither
            System.Diagnostics.Trace.Assert((pRight == null && pLeft == null)
                || (pRight != null && pLeft != null));

            // set the children
            Right = pRight;
            Left = pLeft;

            // set the distance
            Dist = dist;

            // if the children exist,
            if (Right != null
                && Left != null)
            {
                // update the descendant count -- assumes children's count is accurate
                DescendantCount = Right.DescendantCount
                    + Left.DescendantCount + 2;

                // flag to re-compute value
                // m_bComputeValue = TRUE;
            }
            else
            {
                // clear descendant count
                DescendantCount = 0;
            }

            // set the label to reflect number of children
            //m_strLabel.Format("(%i nodes)", m_nDescendantCount);
        }

        // helper function to find the closest cluster in an array
        CCluster FindClosest(IEnumerable<CCluster> inClusters,
            int nStart, ref float pMinDist,
                DistanceFunction func)
        {
            // initialize the index of the closest cluster
            CCluster pClosest = null;

            // initialize the minimum distance to max value
            pMinDist = System.Single.MaxValue;

            // loop through the other clusters
            foreach (CCluster cluster in inClusters)
            {
                // compute the distance
                float dist = func(this, cluster);

                // check to see if this cluster is closer than the current one
                if (dist < pMinDist)
                {
                    // store the distance
                    pMinDist = dist;

                    // and set the new closest cluster
                    pClosest = cluster;
                }
            }

            return pClosest;
        }
    }

#if DRAW
	void Draw(CDC *pDC, CRect *pRect, float maxDist, float minDist)
    {
	double fraction = (maxDist > 0.0) ? GetDist() / maxDist : 1.0;
	ASSERT(fraction >= 0.0 && fraction <= 1.0);

	int nCenterX = pRect->right - ((double) pRect->Width()) * fraction;

	if (GetRight() && GetDist() > minDist)
	{
		CRect rectUpper(*pRect);
		rectUpper.left = nCenterX; // pRect->left + nCenterX;
		GetRight()->Draw(pDC, &rectUpper, GetDist());

		CRect rectLower(*pRect);
		rectLower.left = nCenterX; // pRect->left + nCenterX;
		rectLower.top = rectUpper.bottom;
		GetLeft()->Draw(pDC, &rectLower, GetDist());

		pDC->MoveTo(nCenterX, rectUpper.CenterPoint().y);
		pDC->LineTo(nCenterX, rectLower.CenterPoint().y);

		pRect->bottom = rectLower.bottom;

		int nCenterY = pRect->CenterPoint().y;
		pDC->MoveTo(pRect->left, nCenterY);
		pDC->LineTo(nCenterX, nCenterY);
	}
	else
	{
		pRect->bottom = pRect->top + 20;

		int nCenterY = pRect->CenterPoint().y;
		pDC->MoveTo(pRect->left, nCenterY);
		pDC->LineTo(pRect->right, nCenterY);

		pDC->SetTextAlign(TA_LEFT);

		// otherwise, just output the name of the cluster
		pDC->TextOut(pRect->right, pRect->top, m_strLabel);
	}
    }
#endif

#if NEVER
//////////////////////////////////////////////////////////////////////
// class CEuclideanVectorDistance
// 
// class to perform cluster analysis with euclidean distance
//////////////////////////////////////////////////////////////////////
template<typename TYPE>
class CEuclideanVectorDistance : public CDistanceFunction<TYPE>
{
public:
	// computes the euclidean distance between the vector values of 
	//		the input clusters
	virtual double operator()(const CCluster<TYPE> *pRight, 
			const CCluster<TYPE> *pLeft)
	{
		// form vector difference
		TYPE vec = pRight->GetValue();
		vec -= pLeft->GetValue();

		if (vec.GetLength() == 0.0)
		{ 
			TRACE_VECTOR("vec", pRight->GetValue());
			TRACE_VECTOR("other", pLeft->GetValue());
		}

		// returns length of vector difference
		return vec.GetLength();

	}	// operator()
};


//////////////////////////////////////////////////////////////////////
// class CInverseSquareDistance
// 
// distance function for inverse square distance of two vectors/scalars
//////////////////////////////////////////////////////////////////////
template<typename TYPE>
class CInverseProductDistance : public CDistanceFunction<TYPE>
{
	// flag for squaring
	BOOL m_bSquare;

	// epsilon for inverse
	double m_epsilon;

public:
	// constructor -- pass parameters
	CInverseProductDistance(double epsilon = EPSILON, BOOL bSquare = TRUE) 
		: m_epsilon(epsilon),
			m_bSquare(bSquare) 
	{ 
	}	// CInverseProductDistance

	// computes the inverse product distance
	virtual double operator()(const CCluster<TYPE> *pRight, 
			const CCluster<TYPE> *pLeft)
	{
		// form dot product
		double product = pRight->GetValue() * pLeft->GetValue();

		if (m_bSquare)
		{
			// returns inverse product squared
			return 1.0 / (product * product + m_epsilon);
		}
		else
		{
			// returns inverse absolute value
			return 1.0 / (abs(product) + m_epsilon);
		}

	}	// operator()
};


//////////////////////////////////////////////////////////////////////
// class CNearestNeighborDistance
// 
// class to perform cluster analysis with a nearest neighbor 
//		comparison
//////////////////////////////////////////////////////////////////////
template<typename TYPE>
class CNearestNeighborDistance : public CDistanceFunction<TYPE>
{
	// helper to perform regular euclidean distance
	CDistanceFunction<TYPE> *m_pDist;

public:
	// constructor -- pass the base distance function
	CNearestNeighborDistance(CDistanceFunction<TYPE> *pDist) 
		: m_pDist(pDist) 
	{ 
	}	// CNearestNeighborDistance


	// computes the NN distance, traversing children if present
	virtual double operator()(const CCluster<TYPE> *pRight, 
			const CCluster<TYPE> *pLeft)
	{
		// are children present?
		if (pRight->GetRight() != NULL)
		{
			if (pLeft->GetRight() != NULL)
			{
				// get NN distance for children combos
				double distRR = (*this)(pRight->GetRight(), pLeft->GetRight());
				double distRL = (*this)(pRight->GetRight(), pLeft->GetLeft());
				double distLR = (*this)(pRight->GetLeft(), pLeft->GetRight());
				double distLL = (*this)(pRight->GetLeft(), pLeft->GetLeft());

				// and return the minimum
				return __min(__min(distRR, distRL), __min(distLR, distLL));
			}
			else
			{
				// and return the minimum
				return __min(
					(*this)(pRight->GetRight(), pLeft), 
					(*this)(pRight->GetLeft(), pLeft));
			}
		} 
		else if (pLeft->GetRight() != NULL)
		{
			// and return the minimum
			return __min(
				(*this)(pRight, pLeft->GetRight()), 
				(*this)(pRight, pLeft->GetLeft()));
		}


		// otherwise, use other distance function
		return (*m_pDist)(pRight, pLeft);

	}	// operator()
};


//////////////////////////////////////////////////////////////////////
// class CFurthestNeighborDistance
// 
// class to perform cluster analysis with a nearest neighbor 
//		comparison
//////////////////////////////////////////////////////////////////////
template<typename TYPE>
class CFurthestNeighborDistance : public CDistanceFunction<TYPE>
{
	// helper to perform regular euclidean distance
	CDistanceFunction<TYPE> *m_pDist;

public:
	// constructor -- pass the base distance function
	CFurthestNeighborDistance(CDistanceFunction<TYPE> *pDist) 
		: m_pDist(pDist) { }

	// computes the FN distance, traversing children if present
	virtual double operator()(const CCluster<TYPE> *pRight, 
			const CCluster<TYPE> *pLeft)
	{
		// are children present?
		if (pRight->GetRight() != NULL)
		{
			if (pLeft->GetRight() != NULL)
			{
				// get NN distance for children combos
				double distRR = (*this)(pRight->GetRight(), pLeft->GetRight());
				double distRL = (*this)(pRight->GetRight(), pLeft->GetLeft());
				double distLR = (*this)(pRight->GetLeft(), pLeft->GetRight());
				double distLL = (*this)(pRight->GetLeft(), pLeft->GetLeft());

				// and return the maximum
				return __max(__max(distRR, distRL), __max(distLR, distLL));
			}
			else
			{
				// return the maximum
				return __max(
					(*this)(pRight->GetRight(), pLeft), 
					(*this)(pRight->GetLeft(), pLeft));
			}
		} 
		else if (pLeft->GetRight() != NULL)
		{
			// return the maximum
			return __max(
				(*this)(pRight, pLeft->GetRight()), 
				(*this)(pRight, pLeft->GetLeft()));
		}

		// otherwise, use other distance function
		return (*m_pDist)(pRight, pLeft);

	}	// operator()
};
#endif

}

//#endif // !defined(AFX_CLUSTER_H__9B149912_00AC_49D9_ADF7_84B18B474FD4__INCLUDED_)
