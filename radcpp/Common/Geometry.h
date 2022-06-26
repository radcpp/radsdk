#ifndef RADCPP_GEOMETRY_H
#define RADCPP_GEOMETRY_H
#pragma once

#include "radcpp/Common/Math.h"

struct Sphere
{
    glm::vec3 center;
    float radius;
};

struct BoundingBox
{
    glm::vec3 m_minCorner;
    glm::vec3 m_maxCorner;

    BoundingBox()
    {
        m_minCorner = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
        m_maxCorner = glm::vec3(FLT_MIN, FLT_MIN, FLT_MIN);
    }

    BoundingBox(const glm::vec3& p)
    {
        m_minCorner = p;
        m_maxCorner = p;
    }

    BoundingBox(const glm::vec3& minCorner, const glm::vec3& maxCorner)
    {
        m_minCorner = minCorner;
        m_maxCorner = maxCorner;
    }

    ~BoundingBox()
    {
    }

    const glm::vec3& operator[](int i) const
    {
        return (i == 0) ? m_minCorner : m_maxCorner;
    }

    glm::vec3& operator[](int i)
    {
        return (i == 0) ? m_minCorner : m_maxCorner;
    }

    bool operator==(const BoundingBox& other)
    {
        return (m_minCorner == other.m_minCorner) && (m_maxCorner == other.m_maxCorner);
    }

    bool operator!=(const BoundingBox& other)
    {
        return (m_minCorner != other.m_minCorner) || (m_maxCorner != other.m_maxCorner);
    }

    glm::vec3 Corner(int corner) const
    {
        return glm::vec3(
            (*this)[(corner & 1)].x,
            (*this)[(corner & 2) ? 1 : 0].y,
            (*this)[(corner & 4) ? 1 : 0].z);
    }

    glm::vec3 Diagonal() const
    {
        return m_maxCorner - m_minCorner;
    }

    float DiagonalLength() const
    {
        return glm::length(Diagonal());
    }

    float SurfaceArea() const
    {
        glm::vec3 d = Diagonal();
        return 2.0f * (d.x * d.y + d.x * d.z + d.y * d.z);
    }

    float Volume() const
    {
        glm::vec3 d = Diagonal();
        return d.x * d.y * d.z;
    }

    friend BoundingBox Union(const BoundingBox& b, const glm::vec3& p)
    {
        BoundingBox ret;
        ret.m_minCorner = glm::min(b.m_minCorner, p);
        ret.m_maxCorner = glm::max(b.m_maxCorner, p);
        return ret;
    }

    friend BoundingBox Union(const BoundingBox& b1, const BoundingBox& b2)
    {
        BoundingBox ret;
        ret.m_minCorner = glm::min(b1.m_minCorner, b2.m_minCorner);
        ret.m_maxCorner = glm::max(b1.m_maxCorner, b2.m_maxCorner);
        return ret;
    }

    friend BoundingBox Intersect(const BoundingBox& b1, const BoundingBox& b2)
    {
        BoundingBox ret;
        ret.m_minCorner = glm::max(b1.m_minCorner, b2.m_minCorner);
        ret.m_maxCorner = glm::min(b1.m_maxCorner, b2.m_maxCorner);
        return ret;
    }

    friend bool Overlaps(const BoundingBox& b1, const BoundingBox& b2)
    {
        bool x = (b1.m_maxCorner.x >= b2.m_minCorner.x) && (b1.m_minCorner.x <= b2.m_maxCorner.x);
        bool y = (b1.m_maxCorner.y >= b2.m_minCorner.y) && (b1.m_minCorner.y <= b2.m_maxCorner.y);
        bool z = (b1.m_maxCorner.z >= b2.m_minCorner.z) && (b1.m_minCorner.z <= b2.m_maxCorner.z);
        return (x && y && z);
    }

    friend bool IsInside(const BoundingBox& b, const glm::vec3& p)
    {
        return (p.x >= b.m_minCorner.x && p.x <= b.m_maxCorner.x && p.y >= b.m_minCorner.y &&
            p.y <= b.m_maxCorner.y && p.z >= b.m_minCorner.z && p.z <= b.m_maxCorner.z);
    }

    friend bool InsideExclusive(const BoundingBox& b, const glm::vec3& p)
    {
        return (p.x >= b.m_minCorner.x && p.x < b.m_maxCorner.x&& p.y >= b.m_minCorner.y &&
            p.y < b.m_maxCorner.y&& p.z >= b.m_minCorner.z && p.z < b.m_maxCorner.z);
    }

    friend BoundingBox Expand(const BoundingBox& b, float delta)
    {
        return BoundingBox(
            b.m_minCorner - delta,
            b.m_maxCorner + delta
        );
    }

    void Expand(float delta)
    {
        m_minCorner += delta;
        m_maxCorner += delta;
    }

    Sphere GetBoundingSphere()
    {
        Sphere s;
        s.center = (m_minCorner + m_maxCorner) / 2.0f;
        s.radius = IsInside(*this, s.center) ? glm::distance(s.center, m_maxCorner) : 0.0f;
        return s;
    }

    glm::vec3 GetCenter() const
    {
        return (m_minCorner + m_maxCorner) / 2.0f;
    }

}; // struct BoundingBox

// Coordinate System from a vector
void ConstructCoordinateSystem(const glm::vec3& v1, glm::vec3& v2, glm::vec3& v3);

#endif // RADCPP_GEOMETRY_H