#include "stdafx.h"
#include ".\spring.h"

CSpring::CSpring(void) :
	m_k(3.0),
	m_mass(1.0),
	m_b(10.0)
{
}

CSpring::~CSpring(void)
{
}

CVectorD<3> CSpring::f(const CVectorD<3> x, const CVectorD<3> v)
{
	return v;
}

CVectorD<3> CSpring::g(const CVectorD<3> x, const CVectorD<3> v)
{
	return -m_k / m_mass * x - m_b / m_mass  * v;
}


void CSpring::UpdateSpring(REAL h)
{
	CVectorD<3>& xn = m_vPosition;
	CVectorD<3>& vn = m_vVelocity;

	CVectorD<3> k1 = f (xn,   vn);
	CVectorD<3> j1 = g (xn,   vn);
	CVectorD<3> k2 = f (xn + h / R(2.0) * k1,   vn + h / R(2.0) * j1);
	CVectorD<3> j2 = g (xn + h / R(2.0) * k1,   vn + h / R(2.0) * j1);
	CVectorD<3> k3 = f (xn + h / R(2.0) * k2,   vn + h / R(2.0) * j2);
	CVectorD<3> j3 = g (xn + h / R(2.0) * k2,   vn + h / R(2.0) *  j2);
	CVectorD<3> k4 = f (xn + h * k3,   vn + h * j3);
	CVectorD<3> j4 = g (xn + h * k3,   vn + h * j3);

	xn += h / R(6.0) * (k1 + R(2.0) * k2 + R(2.0) * k3 + k4);
	vn += h / R(6.0) * (j1 + R(2.0) * j2 + R(2.0) * j3 + j4);
	
	// compute pure accel
	REAL old_m_b = m_b;
	m_b = 0.0;
	j1 = g (xn,   vn);
	k2 = f (xn + h / R(2.0) * k1,   vn + h / R(2.0) * j1);
	j2 = g (xn + h / R(2.0) * k1,   vn + h / R(2.0) * j1);
	k3 = f (xn + h / R(2.0) * k2,   vn + h / R(2.0) * j2);
	j3 = g (xn + h / R(2.0) * k2,   vn + h / R(2.0) *  j2);
	k4 = f (xn + h * k3,   vn + h * j3);
	j4 = g (xn + h * k3,   vn + h * j3);

	m_vAccel = R(1.0) / R(6.0) * (j1 + R(2.0) * j2 + R(2.0) * j3 + j4);

	m_b = old_m_b;
}
