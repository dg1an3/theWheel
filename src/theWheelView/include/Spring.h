#pragma once

#include <VectorD.h>

//////////////////////////////////////////////////////////////////////////////
class CSpring
{
public:
	CSpring(void);
	~CSpring(void);

	// spring constants
	DECLARE_ATTRIBUTE(K, REAL);
	DECLARE_ATTRIBUTE(Mass, REAL);
	DECLARE_ATTRIBUTE(B, REAL);

	// gives current position of spring
	DECLARE_ATTRIBUTE(Origin, CVectorD<3>);
	DECLARE_ATTRIBUTE(Position, CVectorD<3>);
	DECLARE_ATTRIBUTE(Velocity, CVectorD<3>);

	// updates the spring a define # of iterations
	void UpdateSpring(REAL h, int nCount = 1);

protected:
	// evaluates the 'f' and 'g' functions for RK4 eval
	CVectorD<3> f(const CVectorD<3> x, const CVectorD<3> v);
	CVectorD<3> g(const CVectorD<3> x, const CVectorD<3> v);
};
