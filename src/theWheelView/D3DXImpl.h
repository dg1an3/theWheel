#pragma once

#include <d3d9.h>

struct D3DXVECTOR3 {
    float x, y, z;

    D3DXVECTOR3() : x(0.0f), y(0.0f), z(0.0f) {}
    D3DXVECTOR3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    // Multiply by scalar
    D3DXVECTOR3& operator *= (float f)
    {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }

    // Add vector
    D3DXVECTOR3& operator += (const D3DXVECTOR3& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
};

struct D3DXMATRIX {
    union {
        struct {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
    };

    D3DXMATRIX() {
        _11 = _22 = _33 = _44 = 1.0f;
        _12 = _13 = _14 = _21 = _23 = _24 = _31 = _32 = _34 = _41 = _42 = _43 = 0.0f;
    }

    D3DXMATRIX(
        float m11, float m12, float m13, float m14,
        float m21, float m22, float m23, float m24,
        float m31, float m32, float m33, float m34,
        float m41, float m42, float m43, float m44)
    {
        _11 = m11; _12 = m12; _13 = m13; _14 = m14;
        _21 = m21; _22 = m22; _23 = m23; _24 = m24;
        _31 = m31; _32 = m32; _33 = m33; _34 = m34;
        _41 = m41; _42 = m42; _43 = m43; _44 = m44;
    }

    // You can also add operator overloads if needed:
    D3DXMATRIX operator * (const D3DXMATRIX& mat) const {
        D3DXMATRIX result;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i][j] =
                    m[i][0] * mat.m[0][j] +
                    m[i][1] * mat.m[1][j] +
                    m[i][2] * mat.m[2][j] +
                    m[i][3] * mat.m[3][j];
            }
        }
        return result;
    }
};

inline void D3DXMatrixTranslation(D3DXMATRIX* pOut, float x, float y, float z)
{
    pOut->_11 = 1.0f; pOut->_12 = 0.0f; pOut->_13 = 0.0f; pOut->_14 = 0.0f;
    pOut->_21 = 0.0f; pOut->_22 = 1.0f; pOut->_23 = 0.0f; pOut->_24 = 0.0f;
    pOut->_31 = 0.0f; pOut->_32 = 0.0f; pOut->_33 = 1.0f; pOut->_34 = 0.0f;
    pOut->_41 = x;    pOut->_42 = y;    pOut->_43 = z;    pOut->_44 = 1.0f;
}

inline void D3DXMatrixScaling(D3DXMATRIX* pOut, float sx, float sy, float sz)
{
    pOut->_11 = sx;   pOut->_12 = 0.0f;  pOut->_13 = 0.0f;  pOut->_14 = 0.0f;
    pOut->_21 = 0.0f; pOut->_22 = sy;    pOut->_23 = 0.0f;  pOut->_24 = 0.0f;
    pOut->_31 = 0.0f; pOut->_32 = 0.0f;  pOut->_33 = sz;    pOut->_34 = 0.0f;
    pOut->_41 = 0.0f; pOut->_42 = 0.0f;  pOut->_43 = 0.0f;  pOut->_44 = 1.0f;
}

inline void D3DXMatrixRotationZ(D3DXMATRIX* pOut, float angle)
{
    float sinA = sinf(angle);
    float cosA = cosf(angle);

    pOut->_11 = cosA;  pOut->_12 = sinA;  pOut->_13 = 0.0f;  pOut->_14 = 0.0f;
    pOut->_21 = -sinA; pOut->_22 = cosA;  pOut->_23 = 0.0f;  pOut->_24 = 0.0f;
    pOut->_31 = 0.0f;  pOut->_32 = 0.0f;  pOut->_33 = 1.0f;  pOut->_34 = 0.0f;
    pOut->_41 = 0.0f;  pOut->_42 = 0.0f;  pOut->_43 = 0.0f;  pOut->_44 = 1.0f;
}

inline void D3DXMatrixOrthoLH(D3DXMATRIX* pOut, float width, float height, float zNear, float zFar)
{
    pOut->_11 = 2.0f / width;
    pOut->_12 = 0.0f;
    pOut->_13 = 0.0f;
    pOut->_14 = 0.0f;

    pOut->_21 = 0.0f;
    pOut->_22 = 2.0f / height;
    pOut->_23 = 0.0f;
    pOut->_24 = 0.0f;

    pOut->_31 = 0.0f;
    pOut->_32 = 0.0f;
    pOut->_33 = 1.0f / (zFar - zNear);
    pOut->_34 = 0.0f;

    pOut->_41 = 0.0f;
    pOut->_42 = 0.0f;
    pOut->_43 = zNear / (zNear - zFar);
    pOut->_44 = 1.0f;
}

inline void D3DXMatrixLookAtLH(D3DXMATRIX* pOut, const D3DXVECTOR3* pEye, const D3DXVECTOR3* pAt, const D3DXVECTOR3* pUp)
{
    D3DXVECTOR3 zaxis; // forward
    zaxis.x = pAt->x - pEye->x;
    zaxis.y = pAt->y - pEye->y;
    zaxis.z = pAt->z - pEye->z;

    // Normalize Z
    float lenZ = sqrtf(zaxis.x * zaxis.x + zaxis.y * zaxis.y + zaxis.z * zaxis.z);
    zaxis.x /= lenZ;
    zaxis.y /= lenZ;
    zaxis.z /= lenZ;

    // Get X axis by cross product of up and Z
    D3DXVECTOR3 xaxis;
    xaxis.x = pUp->y * zaxis.z - pUp->z * zaxis.y;
    xaxis.y = pUp->z * zaxis.x - pUp->x * zaxis.z;
    xaxis.z = pUp->x * zaxis.y - pUp->y * zaxis.x;

    // Normalize X
    float lenX = sqrtf(xaxis.x * xaxis.x + xaxis.y * xaxis.y + xaxis.z * xaxis.z);
    xaxis.x /= lenX;
    xaxis.y /= lenX;
    xaxis.z /= lenX;

    // Get Y axis by cross product of Z and X
    D3DXVECTOR3 yaxis;
    yaxis.x = zaxis.y * xaxis.z - zaxis.z * xaxis.y;
    yaxis.y = zaxis.z * xaxis.x - zaxis.x * xaxis.z;
    yaxis.z = zaxis.x * xaxis.y - zaxis.y * xaxis.x;

    // Create view matrix
    pOut->_11 = xaxis.x;  pOut->_12 = yaxis.x;  pOut->_13 = zaxis.x;  pOut->_14 = 0.0f;
    pOut->_21 = xaxis.y;  pOut->_22 = yaxis.y;  pOut->_23 = zaxis.y;  pOut->_24 = 0.0f;
    pOut->_31 = xaxis.z;  pOut->_32 = yaxis.z;  pOut->_33 = zaxis.z;  pOut->_34 = 0.0f;
    pOut->_41 = -(xaxis.x * pEye->x + xaxis.y * pEye->y + xaxis.z * pEye->z);
    pOut->_42 = -(yaxis.x * pEye->x + yaxis.y * pEye->y + yaxis.z * pEye->z);
    pOut->_43 = -(zaxis.x * pEye->x + zaxis.y * pEye->y + zaxis.z * pEye->z);
    pOut->_44 = 1.0f;
}

inline void D3DXMatrixIdentity(D3DXMATRIX* pOut)
{
    pOut->_11 = 1.0f; pOut->_12 = 0.0f; pOut->_13 = 0.0f; pOut->_14 = 0.0f;
    pOut->_21 = 0.0f; pOut->_22 = 1.0f; pOut->_23 = 0.0f; pOut->_24 = 0.0f;
    pOut->_31 = 0.0f; pOut->_32 = 0.0f; pOut->_33 = 1.0f; pOut->_34 = 0.0f;
    pOut->_41 = 0.0f; pOut->_42 = 0.0f; pOut->_43 = 0.0f; pOut->_44 = 1.0f;
}

inline D3DXVECTOR3* D3DXVec3Normalize(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV)
{
    float length = sqrtf(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z);

    if (length > 0.0f)
    {
        float invLength = 1.0f / length;
        pOut->x = pV->x * invLength;
        pOut->y = pV->y * invLength;
        pOut->z = pV->z * invLength;
    }
    else
    {
        // Handle zero-length vector case
        pOut->x = 0.0f;
        pOut->y = 0.0f;
        pOut->z = 0.0f;
    }

    return pOut;
}

inline D3DXVECTOR3* D3DXVec3TransformNormal(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV, const D3DXMATRIX* pM)
{
    D3DXVECTOR3 temp;

    temp.x = pV->x * pM->_11 + pV->y * pM->_21 + pV->z * pM->_31;
    temp.y = pV->x * pM->_12 + pV->y * pM->_22 + pV->z * pM->_32;
    temp.z = pV->x * pM->_13 + pV->y * pM->_23 + pV->z * pM->_33;

    *pOut = temp;
    return pOut;
}

inline D3DXVECTOR3* D3DXVec3TransformCoord(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV, const D3DXMATRIX* pM)
{
    D3DXVECTOR3 temp;
    float w;

    // Transform the vector
    temp.x = pV->x * pM->_11 + pV->y * pM->_21 + pV->z * pM->_31 + pM->_41;
    temp.y = pV->x * pM->_12 + pV->y * pM->_22 + pV->z * pM->_32 + pM->_42;
    temp.z = pV->x * pM->_13 + pV->y * pM->_23 + pV->z * pM->_33 + pM->_43;
    w = pV->x * pM->_14 + pV->y * pM->_24 + pV->z * pM->_34 + pM->_44;

    // Perspective divide if w is not 1
    if (w != 1.0f && w != 0.0f)
    {
        float invW = 1.0f / w;
        pOut->x = temp.x * invW;
        pOut->y = temp.y * invW;
        pOut->z = temp.z * invW;
    }
    else
    {
        *pOut = temp;
    }

    return pOut;
}
