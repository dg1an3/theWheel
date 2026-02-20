//////////////////////////////////////////////////////////////////////
// Spring.h: Standalone spring physics for wxWidgets port
//
// Ported from theWheelView/include/Spring.h - uses CVectorD<3>
// directly without MFC CSpring class dependencies.
//////////////////////////////////////////////////////////////////////

#pragma once

#include <VectorD.h>

class Spring
{
public:
    Spring()
        : m_K(3.0f), m_Mass(1.0f), m_B(10.0f)
    {
    }

    void SetOrigin(const CVectorD<3>& origin) { m_Origin = origin; }
    const CVectorD<3>& GetOrigin() const { return m_Origin; }

    void SetPosition(const CVectorD<3>& pos) { m_Position = pos; }
    const CVectorD<3>& GetPosition() const { return m_Position; }

    const CVectorD<3>& GetVelocity() const { return m_Velocity; }

    void SetK(REAL k) { m_K = k; }
    void SetMass(REAL m) { m_Mass = m; }
    void SetB(REAL b) { m_B = b; }

    // RK4 spring update
    void Update(REAL h, int nCount = 1)
    {
        for (int i = 0; i < nCount; i++)
        {
            CVectorD<3> x = m_Position - m_Origin;
            CVectorD<3>& v = m_Velocity;

            CVectorD<3> k1 = v;
            CVectorD<3> j1 = g(x, v);
            CVectorD<3> k2 = v + h / R(2.0) * j1;
            CVectorD<3> j2 = g(x + h / R(2.0) * k1, v + h / R(2.0) * j1);
            CVectorD<3> k3 = v + h / R(2.0) * j2;
            CVectorD<3> j3 = g(x + h / R(2.0) * k2, v + h / R(2.0) * j2);
            CVectorD<3> k4 = v + h * j3;
            CVectorD<3> j4 = g(x + h * k3, v + h * j3);

            m_Position += h / R(6.0) * (k1 + R(2.0) * k2 + R(2.0) * k3 + k4);
            v += h / R(6.0) * (j1 + R(2.0) * j2 + R(2.0) * j3 + j4);
        }
    }

private:
    CVectorD<3> g(const CVectorD<3>& x, const CVectorD<3>& v)
    {
        return -m_K / m_Mass * x - m_B / m_Mass * v;
    }

    REAL m_K;
    REAL m_Mass;
    REAL m_B;
    CVectorD<3> m_Origin;
    CVectorD<3> m_Position;
    CVectorD<3> m_Velocity;
};

// 1D spring for activation animation
class Spring1D
{
public:
    Spring1D()
        : m_K(3.0f), m_Mass(1.0f), m_B(10.0f)
        , m_Origin(0), m_Position(0), m_Velocity(0)
    {
    }

    void SetOrigin(REAL origin) { m_Origin = origin; }
    REAL GetOrigin() const { return m_Origin; }

    void SetPosition(REAL pos) { m_Position = pos; }
    REAL GetPosition() const { return m_Position; }

    void Update(REAL h, int nCount = 1)
    {
        for (int i = 0; i < nCount; i++)
        {
            REAL x = m_Position - m_Origin;
            REAL v = m_Velocity;

            REAL k1 = v;
            REAL j1 = -m_K / m_Mass * x - m_B / m_Mass * v;
            REAL k2 = v + h / R(2.0) * j1;
            REAL j2 = -m_K / m_Mass * (x + h / R(2.0) * k1) - m_B / m_Mass * (v + h / R(2.0) * j1);
            REAL k3 = v + h / R(2.0) * j2;
            REAL j3 = -m_K / m_Mass * (x + h / R(2.0) * k2) - m_B / m_Mass * (v + h / R(2.0) * j2);
            REAL k4 = v + h * j3;
            REAL j4 = -m_K / m_Mass * (x + h * k3) - m_B / m_Mass * (v + h * j3);

            m_Position += h / R(6.0) * (k1 + R(2.0) * k2 + R(2.0) * k3 + k4);
            m_Velocity += h / R(6.0) * (j1 + R(2.0) * j2 + R(2.0) * j3 + j4);
        }
    }

private:
    REAL m_K, m_Mass, m_B;
    REAL m_Origin, m_Position, m_Velocity;
};
