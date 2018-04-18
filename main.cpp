//
//  main.cpp
//  partition
//
//  Created by Olivier Rassemusse on 01/01/16.
//  All rights reserved.
//

#include "partition.h"

/****************************
             o-----o-----o 
            /#7   /|    /|
           / |   / |   /#6
          o-----o-----o  |
         /| /  /| o--/|--o  
        / |/  / |   / | /|  
       o-----o-----o  |/ | 
 #5    |  o--|--o--|--o#4|
       | #2  | #3  | /|  o
       |/ |  |/ |  |/ | /
       o-----o-----o  |/
       |  o--|--o--|--o
       | /   | /   | /
       |/ #0 |/#1  |/
       o-----o-----o
 
 y  z
 | /
 |/
 o--x
 
 #0: (0,0,0)
 #1: (1,0,0)
 #2: (0,1,0)
 #3: (1,1,0)
 #4: (1,0,1)
 #5: (0,0,1)
 #6: (1,1,1)
 #7: (0,1,1)     
 *****************************/

partition_t * gen_partition_at_level (int ax, int ay, int az, int level, const partition_t * parent)
{
    bbox_t abb;
    abb = partition_t::gen_bbox_from_parent_box (ax, ay, az, parent->bb);
    partition_t * partition = partition_t::make_new(ax, ay, az, level, parent->pid, abb);
    return partition;
}

void gen_partitions (int start_level, int max_level, partition_t * parent, partitions_t& container)
{
    int level = start_level;
    if (level > max_level)
        return;
    
    int indices [] = {
        0, 0, 0,
        1, 0, 0,
        1, 1, 0,
        1, 0, 1,
        0, 1, 0,
        0, 1, 1,
        0, 0, 1,
        1, 1, 1
    };
    
    for (int i=0; i<8; i++) {
        partition_t * p = gen_partition_at_level(indices[0+3*i], indices[1+3*i], indices[2+3*i], level, parent);
        partition_item_t item = make_pair(p->pid, p);
        container.insert(item);
        gen_partitions (level+1, max_level, p, container);
    }
}

void gen_world_partitions (const struct bbox_t& wb, int max_level, partitions_t& container)
{
    int level;

    assert(max_level>0);
    if (max_level > MAX_LEVEL) {
        max_level = MAX_LEVEL;
    }
    partition_t * p = partition_t::make_new(0, 0, 0, 0, 0, wb);
    partition_item_t item = make_pair(p->pid, p);
    container.insert(item);
    
    level = 1;
    gen_partitions (level, max_level, p, container);
    
    printf("Num level = %d Num partitions = %lu\n", max_level+1, container.size());
}

partition_t * partition_from_pos (const vec3_t& pos, const partitions_t& wp)
{
    partitions_t::const_iterator iter;
    for (iter = wp.begin(); iter != wp.end(); ++iter) {
        if (iter->second->bb.contains(pos))
            return iter->second;
    }
    return NULL;
}

void partitions_from_pos (const vec3_t& pos, const partitions_t& wp, vector<partition_t *>& res)
{
    partitions_t::const_iterator iter;
    for (iter = wp.begin(); iter != wp.end(); ++iter) {
        if (iter->second->bb.contains(pos))
            res.push_back(iter->second);
    }
}

struct dummy_cube_t
{
    vec3_t pos;
    vec3_t size;
    int ori;
    
    dummy_cube_t () : pos (), size (), ori (0)
    {}
};

void partitions_from_cube_dim (dummy_cube_t * c,  const partitions_t& wp, vector<partition_t *>& res)
{
    vec3_t ve[8];
    ve[0] = vec3_t (c->pos.x - c->size.x * 0.5f, c->pos.y + c->size.y * 0.5f, c->pos.z + c->size.z * 0.5f);
    ve[1] = vec3_t (c->pos.x + c->size.x * 0.5f, c->pos.y + c->size.y * 0.5f, c->pos.z + c->size.z * 0.5f);
    ve[2] = vec3_t (c->pos.x - c->size.x * 0.5f, c->pos.y - c->size.y * 0.5f, c->pos.z + c->size.z * 0.5f);
    ve[3] = vec3_t (c->pos.x + c->size.x * 0.5f, c->pos.y - c->size.y * 0.5f, c->pos.z + c->size.z * 0.5f);
    ve[4] = vec3_t (c->pos.x - c->size.x * 0.5f, c->pos.y + c->size.y * 0.5f, c->pos.z - c->size.z * 0.5f);
    ve[5] = vec3_t (c->pos.x + c->size.x * 0.5f, c->pos.y + c->size.y * 0.5f, c->pos.z - c->size.z * 0.5f);
    ve[6] = vec3_t (c->pos.x - c->size.x * 0.5f, c->pos.y - c->size.y * 0.5f, c->pos.z - c->size.z * 0.5f);
    ve[7] = vec3_t (c->pos.x + c->size.x * 0.5f, c->pos.y - c->size.y * 0.5f, c->pos.z - c->size.z * 0.5f);
    
    partitions_t::const_iterator iter;
    for (iter = wp.begin(); iter != wp.end(); ++iter) {
        bool is_in = true;
        for (int i=0; i<8; i++) {
            if (!iter->second->bb.contains(ve[i]))
                is_in = false;
            
        }
        if (is_in)
            res.push_back(iter->second);
    }
}

int main (int argc, const char * argv[])
{
#include "ray.h"
    
    ray_t ray;
    ray.point_at(0.56);
    
    partition_t tt;
    int test_pid = tt.make_pid(0, 0, 0, 0, 0);
    test_pid = tt.make_pid(test_pid, 0, 1, 0, 2);
    int resp = test_decode2(test_pid);
    int vals[5];
    bool ans = test_decode(test_pid, vals);
    
    bbox_t wb;
    partitions_t container;
    int max_level;
    int num_cubes;
    vec3_t max_cube_dim;
    int max_cube_ori;
    dummy_cube_t * cubes;
    
    wb.min = vec3_t(0, 0, 0);
    wb.max = vec3_t(200, 100, 50);
    max_level = 4;
    num_cubes = 200;
    cubes = new dummy_cube_t [num_cubes];
    max_cube_dim.x = 50;
    max_cube_dim.y = 30;
    max_cube_dim.z = 25;
    max_cube_ori = 80;
    gen_world_partitions (wb, max_level, container);

    vector<partition_t *> res;
    
    for (int i=0; i<num_cubes; i++) {
        cubes[i].pos.x = arc4random_uniform(wb.height());
        cubes[i].pos.y = arc4random_uniform(wb.width());
        cubes[i].pos.z = arc4random_uniform(wb.depth());
        cubes[i].size.x = arc4random_uniform(max_cube_dim.x);
        cubes[i].size.y = arc4random_uniform(max_cube_dim.y);
        cubes[i].size.z = arc4random_uniform(max_cube_dim.z);
        cubes[i].ori = arc4random_uniform(max_cube_ori);
#if 0
        partitions_from_pos (cubes[i].pos, container, res);
        if (!res.empty()) {
            for (int j=0; j<res.size(); j++) {
                printf("cube [%d] is included in partition pid:%u\n", i, res[j]->pid);
            }
        }
        res.clear();
#endif   
#if 1
        partitions_from_cube_dim(&cubes[i], container, res);
        if (!res.empty()) {
            for (int j=0; j<res.size(); j++) {
                printf("cube [%d] is included in partition pid:%u\n", i, res[j]->pid);
            }
        }
        res.clear();
    }
#endif
    
    if (cubes) {
        delete [] cubes;
        cubes = NULL;
    }
    return 0;
}









































