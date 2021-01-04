#include "stdafx.h"
#include ".\spring.h"

//////////////////////////////////////////////////////////////////////////////
CSpring::CSpring(void) :
	m_K(3.0),
	m_Mass(1.0),
	m_B(10.0)
{
}

//////////////////////////////////////////////////////////////////////////////
CSpring::~CSpring(void)
{
}

//////////////////////////////////////////////////////////////////////////////
void 
	CSpring::UpdateSpring(REAL h, int nCount)
{
	for (int nAt = 0; nAt < nCount; nAt++)
	{
		CVectorD<3> xn = GetPosition() - GetOrigin();
		CVectorD<3>& vn = m_Velocity;

		CVectorD<3> k1 = f (xn,   vn);
		CVectorD<3> j1 = g (xn,   vn);
		CVectorD<3> k2 = f (xn + h / R(2.0) * k1,   vn + h / R(2.0) * j1);
		CVectorD<3> j2 = g (xn + h / R(2.0) * k1,   vn + h / R(2.0) * j1);
		CVectorD<3> k3 = f (xn + h / R(2.0) * k2,   vn + h / R(2.0) * j2);
		CVectorD<3> j3 = g (xn + h / R(2.0) * k2,   vn + h / R(2.0) *  j2);
		CVectorD<3> k4 = f (xn + h * k3,   vn + h * j3);
		CVectorD<3> j4 = g (xn + h * k3,   vn + h * j3);

		m_Position += h / R(6.0) * (k1 + R(2.0) * k2 + R(2.0) * k3 + k4);
		vn += h / R(6.0) * (j1 + R(2.0) * j2 + R(2.0) * j3 + j4);
	}
}

//////////////////////////////////////////////////////////////////////////////
CVectorD<3> 
	CSpring::f(const CVectorD<3> x, const CVectorD<3> v)
{
	return v;
}

//////////////////////////////////////////////////////////////////////////////
CVectorD<3> CSpring::g(const CVectorD<3> x, const CVectorD<3> v)
{
	return -GetK() / GetMass() * x - GetB() / GetMass() * v;
}


