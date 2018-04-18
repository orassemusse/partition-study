//
//  ray.h
//  partition
//
//  Created by Olivier Rassemusse on 04/09/2016.
//  All rights reserved.
//

#ifndef partition_ray_h
#define partition_ray_h

#include <simd/simd.h>
#include <math.h>

using namespace simd;

#define FLT_EPSILON         1.192092896e-07f

struct ray_t
{
    float4 direction;
    float4 position;
    
    ray_t(const float4 &_dir, const float4 &_pos)
    :   direction(_dir), position(_pos)
    {}
    
    ray_t()
    :   direction(), position()
    {}
    
    float4 point_at(float t) const
    {
        return (position + direction * t);
    }
    
    float distance(const float4 &p)
    {
        return sqrtf(length_squared(position - p) - dot(position - p, direction) * dot(position - p, direction) / length_squared(direction));
    }
    
    bool is_parallel(const ray_t &r, float *val)
    {
        float n = dot(direction, r.direction) * dot(direction, r.direction) - dot(direction, direction) * dot(r.direction, r.direction);
        if (val) *val=n;
        n = (n<0) ? -n : n;
        return (n <= FLT_EPSILON);
    }
    
    float distance_from(const ray_t &r)
    {
        float n;
        bool ret = is_parallel(r, &n);
        if (ret)
            return distance(r.point_at(0));
        else {
            float2x2 m; // = { length_squared(position), dot(-position, r.position), -dot(-position, r.position), length_squared(r.position) };
            float2 v;
            float2 vv = m * v;
        }
        return 0.0f;
    }
};



typedef struct simd_sphere_t
{
    vector_float3 c;
    float r;
} simd_sphere_t;

typedef struct simd_aabb_t
{
    vector_float3 c;
    vector_float3 r;
} simd_aabb_t;

#if 0
static int simd_test_aabb_aabb(simd_aabb_t a, simd_aabb_t b)
{
    vector_float3 w = vector_abs(a.c - b.c);
    vector_float3 v = a.r + b.r;
    if (w.x > v.x) return 0;
    if (w.y > v.y) return 0;
    if (w.z > v.z) return 0;
    return 1;
}

static int simd_test_sphere_sphere(simd_sphere_t a, simd_sphere_t b)
{
    float dist2 = vector_distance_squared(a.c, b.c);
    float radius_sum = a.r + b.r;
    return (dist2 <= radius_sum * radius_sum);
}

static int simd_point_in_triangle(vector_float3 p, vector_float3 a, vector_float3 b, vector_float3 c)
{
    a = a - p;
    b = b - p;
    c = c - p;
    vector_float3 u = vector_cross(b, c);
    vector_float3 v = vector_cross(c, a);
    if (vector_dot(u, v) < 0.0f) return 0;
    vector_float3 w = vector_cross(a, b);
    if (vector_dot(u, w) < 0.0f) return 0;
    return 1;
}

#endif

#endif
