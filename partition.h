//
//  partition.h
//  partition
//
//  Created by Olivier Rassemusse on 01/01/16.
//  All rights reserved.
//

#ifndef __PARTITION_H__
#define __PARTITION_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <simd/simd.h>
#include <math.h>
#include <vector>
#include <map>
#include <unordered_map>


using namespace std;

#define BITSET(x,n)    (x | ( 1 << n))
#define BITCLEAR(x,n)  (x & ~(1 << n)) 
#define BITTOGGLE(x,n) (x ^ ( 1 << n))
#define ISBITSET(x,n)  ((x & (1 << n)) != 0)

#define NUM_LEVEL 0x20
#define MAX_LEVEL NUM_LEVEL-1

struct vec3_t
{
    float x;
    
    float y;
    float z;
    
    vec3_t() : x(0), y(0), z(0) {}
    vec3_t(const vec3_t& v) : x(v.x), y(v.y), z(v.z) {}
    vec3_t(float ax, float ay, float az) : x(ax), y(ay), z(az) {}
    vec3_t &operator=(const vec3_t& v) { x = v.x; y = v.y; z = v.z; return *this; }
    static float dot(const vec3_t& a, const vec3_t& b) { return (a.x * b.x + a.y * b.y + a.z * b.z); }
    static vec3_t cross(const vec3_t& p, const vec3_t& q) { return vec3_t(p.y*q.z - p.z*q.y, p.z*q.x - p.x*q.z, p.x*q.y - p.y*q.x); }
    float length() { return sqrtf(x*x + y*y + z*z); }
    void normalize() { x = x/length(); y = y/length(); z = z/length(); }
};

struct vec4_t
{
    float x;
    float y;
    float z;
    float w;
    
    vec4_t () : x(0), y(0), z(0), w(0) {}
    vec4_t (const vec4_t& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
    vec4_t &operator= (const vec4_t& v) { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }
    static float dot_product (const vec4_t& a, const vec4_t& b) { return (a.x * b.x + a.y * b.y + a.z * b.z + a.w + b.w); }

};

template <class T>
inline T Abs(T aVal)
{return(aVal<0 ? -aVal : aVal);}

const float VEC3_EPSILON =  0.0001f;
const float BBOX_MARGIN  =  0.01f;

struct bbox_t
{
    struct vec4_t color;
    struct vec3_t min;
    struct vec3_t max;
    
    bbox_t () : color(), min(), max() {}   
    bbox_t (const bbox_t& b) : color(b.color), min(b.min), max(b.max) {}
    bbox_t &operator= (const bbox_t& b) { color = b.color; min = b.min; max = b.max; return *this; }
    ~bbox_t () {}
    bool contains (const vec3_t& p)
    {
        if(p.x >= (min.x) && p.x <= (max.x) &&
           p.y >= (min.y) && p.y <= (max.y) &&
           p.z >= (min.z) && p.z <= (max.z))
            return true;
        
        return false;
    }
    bool contains_with_margin (const vec3_t& p, const float m = BBOX_MARGIN)
    {
        if(p.x >= (min.x-m) && p.x <= (max.x+m) &&
           p.y >= (min.y-m) && p.y <= (max.y+m) &&
           p.z >= (min.z-m) && p.z <= (max.z+m))
            return true;
        
        return false;
    }
    vec3_t center () const
    {
        return vec3_t( (min.x + max.x)* 0.5f, (min.y + max.y) * 0.5f, (min.z + max.z) * 0.5f );
    }
    float width () const
    {
        return Abs(min.x- max.x);
    }
    float height () const
    {
        return Abs(min.y - max.y);
    }
    float depth () const
    {
        return Abs(min.z - max.z);
    }
    
    bbox_t merge(const bbox_t& b)
    {
        bbox_t box;
        simd::float3 rmi, rma;
        simd::float3 bmi = {b.min.x, b.min.y, b.min.z};
        simd::float3 bma = {b.max.x, b.max.y, b.max.z};
        simd::float3 mi = {min.x, min.y, min.z};
        simd::float3 ma = {max.x, max.y, max.z};
        
        rmi = simd::min(mi, bmi);
        rma = simd::max(ma, bma);
        box.min = *((vec3_t*)&rmi);
        box.max = *((vec3_t*)&rma);
        return box;
    }
};

struct sphere_t
{
    struct vec3_t site;
    float radius;
    sphere_t () : site(), radius(0) {}
    sphere_t (const sphere_t& s) : site(s.site), radius(s.radius) {}
    sphere_t &operator= (const sphere_t& s) { site = s.site; radius =s.radius; return *this; }
};

struct plane_t
{
    struct vec3_t normal;
    float sd;
};

int test_decode2(int pid);
bool test_decode(int pid, int* vals);

struct partition_t;

typedef unordered_map <uint32_t, partition_t *>   partitions_t;
typedef pair<uint32_t, partition_t *>   partition_item_t;
typedef pair<partition_t *, bool>       partition_result_t;

struct partition_t
{
    unsigned char ix:1;
    unsigned char iy:1;
    unsigned char iz:1;
    unsigned int lev:5;
    unsigned int ppid;
    unsigned int pid;
    struct bbox_t bb;
    vector<bbox_t> items;

    partition_t () : ix(0), iy(0), iz(0), lev(0), ppid(0), pid(0), bb() {}
    ~partition_t () {}
    
    void encode ();
    int make_pid(int ppid, int ix, int iy, int iz, int level);
    int init (int ax, int ay, int az, int level, int parent_pid, const struct bbox_t& bb);
    static partition_t * make_new (int ax, int ay, int az, int level, int parent_pid, const struct bbox_t& bb);
    static bbox_t gen_bbox_from_parent_box (int ax, int ay, int az, const struct bbox_t& bb);
    size_t childs_from (vector<partition_t *> & vp, const partitions_t& p);
};

struct world_t
{
    vector<bbox_t> bbounds;
};

struct partition_generator_t
{
    partitions_t _container;
    void generate(const world_t& world, partitions_t& container);
    partition_t * gen_partition_at_level (int ax, int ay, int az, int level, const partition_t * parent);
    void gen_partitions (int start_level, int max_level, partition_t * parent, partitions_t& container);
    void gen_world_partitions (const struct bbox_t& wb, int max_level, partitions_t& container);
};

#endif // __PARTITION_H__
