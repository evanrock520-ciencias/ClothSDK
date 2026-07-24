#pragma once

#include "Eigen/Dense"
#include "Functions.hpp"
#include "Types.hpp"

namespace Tissu {

struct SegmentTriangleHit {
    bool hit = false;
    double t = 1.0;
    Eigen::Vector3d point;
    Eigen::Vector3d normal;
    double u = 0.0, v = 0.0, w = 0.0;
};

struct EdgeEdgeHit {
    bool hit = false;
    double distance = 0.0;
    Eigen::Vector3d closestP; // Closest point in edge 1
    Eigen::Vector3d closestQ; // Closest point in edge 2
    Eigen::Vector3d normal;
    double s = 0.0; // Parameter in edge 1
    double t = 0.0; // Parameter in edge 2
};

inline Eigen::Vector3d closestPointOnTriangle(const Eigen::Vector3d& point,
                                              const Eigen::Vector3d& a,
                                              const Eigen::Vector3d& b,
                                              const Eigen::Vector3d& c) {
    Eigen::Vector3d ab = b - a;
    Eigen::Vector3d ac = c - a;
    Eigen::Vector3d ap = point - a;

    double d1 = ab.dot(ap);
    double d2 = ac.dot(ap);

    if (d1 <= 0.0 && d2 <= 0.0)
        return a;

    Eigen::Vector3d bp = point - b;
    double d3 = ab.dot(bp);
    double d4 = ac.dot(bp);

    if (d3 >= 0.0 && d4 <= d3)
        return b;

    Eigen::Vector3d cp = point - c;
    double d5 = ab.dot(cp);
    double d6 = ac.dot(cp);

    if (d6 >= 0.0 && d5 <= d6)
        return c;

    double vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0 && d1 >= 0.0 && d3 <= 0.0) {
        double t = d1 / (d1 - d3);
        return a + t * ab;
    }

    double vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0 && d2 >= 0.0 && d6 <= 0.0) {
        double t = d2 / (d2 - d6);
        return a + t * ac;
    }

    double va = d3 * d6 - d5 * d4;
    if (va <= 0.0 && (d4 - d3) >= 0.0 && (d5 - d6) >= 0.0) {
        double t = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return b + t * (c - b);
    }

    double denom = 1.0 / (va + vb + vc);
    double v = vb * denom;
    double w = vc * denom;
    return a + v * ab + w * ac;
}

inline SegmentTriangleHit
intersectSegmentTriangle(const Eigen::Vector3d& oldPos,
                         const Eigen::Vector3d& newPos,
                         const Eigen::Vector3d& a, const Eigen::Vector3d& b,
                         const Eigen::Vector3d& c) {
    SegmentTriangleHit hit;
    const Eigen::Vector3d dir = newPos - oldPos; // no normalized
    const Eigen::Vector3d edge1 = b - a;
    const Eigen::Vector3d edge2 = c - a;
    const Eigen::Vector3d pvec = dir.cross(edge2);
    const double det = edge1.dot(pvec);

    // Parallel Ray
    if (std::abs(det) < 1e-9)
        return hit;

    const double invDet = 1.0 / det;
    const Eigen::Vector3d tvec = oldPos - a;
    const double u = tvec.dot(pvec) * invDet;
    if (u < 0.0 || u > 1.0)
        return hit;

    const Eigen::Vector3d qvec = tvec.cross(edge1);
    const double v = dir.dot(qvec) * invDet;
    if (v < 0.0 || u + v > 1.0)
        return hit;

    const double t = edge2.dot(qvec) * invDet;
    if (t < 0.0 || t > 1.0)
        return hit;

    hit.hit = true;
    hit.t = t;
    hit.point = oldPos + t * dir;
    hit.u = u;
    hit.v = v;
    hit.w = 1.0 - u - v;
    Eigen::Vector3d normal = edge1.cross(edge2).normalized();
    if (normal.dot(dir) > 0.0)
        normal = -normal;
    hit.normal = normal;
    return hit;
}

inline EdgeEdgeHit closestPointsEdgeEdge(const Eigen::Vector3d& p1,
                                         const Eigen::Vector3d& p2,
                                         const Eigen::Vector3d& q1,
                                         const Eigen::Vector3d& q2,
                                         double thickness = 0.0) {
    const Eigen::Vector3d dirEdge1 = p2 - p1;
    const Eigen::Vector3d dirEdge2 = q2 - q1;
    const Eigen::Vector3d r = p1 - q1;

    const double a = dirEdge1.dot(dirEdge1);
    const double e = dirEdge2.dot(dirEdge2);
    const double f = dirEdge2.dot(r);
    double s = 0.0;
    double t = 0.0;

    const double kEps = 1e-9;

    if (a <= kEps && e <= kEps) {
        s = 0.0;
        t = 0.0;
    } else if (a <= kEps) {
        s = 0.0;
        t = std::clamp(f / e, 0.0, 1.0);
    } else {
        const double c = dirEdge1.dot(r);
        if (e <= kEps) {
            t = 0.0;
            s = std::clamp(-c / a, 0.0, 1.0);
        } else {
            const double b = dirEdge1.dot(dirEdge2);

            if (const double det = a * e - b * b; det > kEps) {
                s = std::clamp((b * f - c * e) / det, 0.0, 1.0);
            } else {
                s = 0.0;
            }

            t = (b * s + f) / e;

            if (t < 0.0) {
                t = 0.0;
                s = std::clamp(-c / a, 0.0, 1.0);
            } else if (t > 1.0) {
                t = 1.0;
                s = std::clamp((b - c) / a, 0.0, 1.0);
            }
        }
    }

    const Eigen::Vector3d closestP = lerp(p1, p2, s);
    const Eigen::Vector3d closestQ = lerp(q1, q2, t);
    const Eigen::Vector3d diff = closestP - closestQ;
    const double dist = diff.norm();

    EdgeEdgeHit hit;
    hit.s = s;
    hit.t = t;
    hit.closestP = closestP;
    hit.closestQ = closestQ;
    hit.distance = dist;

    if (dist > kEps)
        hit.normal = diff / dist;
    else
        hit.normal = dirEdge1.cross(dirEdge2).normalized();

    hit.hit = (dist <= thickness);
    return hit;
}

} // namespace Tissu
