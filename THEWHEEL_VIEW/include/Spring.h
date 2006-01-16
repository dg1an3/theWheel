#pragma once

#include <VectorD.h>

class CSpring
{
public:
	CSpring(void);
	~CSpring(void);

	void UpdateSpring(REAL h);

	// gives current position of spring
	CVectorD<3> m_vPosition;
	CVectorD<3> m_vVelocity;
	CVectorD<3> m_vAccel;

	REAL m_k;
	REAL m_mass;
	REAL m_b;


	CVectorD<3> f(const CVectorD<3> x, const CVectorD<3> v);
	CVectorD<3> g(const CVectorD<3> x, const CVectorD<3> v);

};
