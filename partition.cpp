//
//  partition.cpp
//  partition
//
//  Created by Olivier Rassemusse on 01/01/16.
//  All rights reserved.
//

#include "partition.h"

void print_bits(void* data, int dlen)
{
    int j;
    unsigned char* p = (unsigned char*)data;
    for (j=0; j<dlen; j++) {
        int res = ISBITSET(*(p+(j>>3)),(j&0x7));
        if (res) {
            printf("1");
        } else {
            printf("0");
        }
    }
    printf("\n");
}

bool test_decode(int pid, int* vals)
{
    int i, rx, ry, rz, rlev=0, rppid;
    if (pid > 0) {
        rppid = (pid >> 0x8);
        rx = ISBITSET(pid, 0) ? 1 : 0;
        ry = ISBITSET(pid, 1) ? 1 : 0;
        rz = ISBITSET(pid, 2) ? 1 : 0;
        for (i=0; i<5; i++) {
            rlev=(ISBITSET(pid, ((i+3)&0x7)) ? BITSET(rlev, i) : BITCLEAR(rlev, i));
        }
        vals[0] = rppid;
        vals[1] = rx;
        vals[2] = ry;
        vals[3] = rz;
        vals[4] = rlev;
        return true;
    }
    return false;
}

int test_decode2(int pid)
{
    int rppid;
    if (pid > 0) {
        rppid = (pid >> 0x8);
    }
    return rppid;
}

void partition_t::encode()
{
    int i;
    pid = 0;
    if (ppid > 0) {
        pid |= (ppid << 0x8);
    }
    
    pid  = ((ix == 1) ? BITSET(pid, 0) : BITCLEAR(pid, 0));
    pid  = ((iy == 1) ? BITSET(pid, 1) : BITCLEAR(pid, 1));
    pid  = ((iz == 1) ? BITSET(pid, 2) : BITCLEAR(pid, 2));
    for (i=0; i<5; i++) {
        pid = (ISBITSET(lev, i) ? BITSET(pid, ((i+3)&0x7)) : BITCLEAR(pid, ((i+3)&0x7)));
    }
}

int partition_t::make_pid(int ppid, int ix, int iy, int iz, int level)
{
    int i;
    int _pid;
    _pid = 0;
    if (ppid > 0) {
        _pid |= (ppid << 0x8);
    }
    
    _pid  = ((ix == 1) ? BITSET(_pid, 0) : BITCLEAR(_pid, 0));
    _pid  = ((iy == 1) ? BITSET(_pid, 1) : BITCLEAR(_pid, 1));
    _pid  = ((iz == 1) ? BITSET(_pid, 2) : BITCLEAR(_pid, 2));
    for (i=0; i<5; i++) {
        _pid = (ISBITSET(level, i) ? BITSET(_pid, ((i+3)&0x7)) : BITCLEAR(_pid, ((i+3)&0x7)));
    }
    return _pid;
}

int partition_t::init (int ax, int ay, int az, int level, int parent_pid, const struct bbox_t& abb)
{
    assert (level < NUM_LEVEL);
    ix = ax;
    iy = ay;
    iz = az;
    lev = level;
    ppid = parent_pid;
    encode ();
    bb = abb;
    printf ("partition_init pid=%d ppid=%d\n", pid, ppid);
    print_bits ( &pid, 16);
    printf("partition_init bbox min=(%f, %f, %f) max(%f, %f, %f)\n\n", bb.min.x, bb.min.y, bb.min.z, bb.max.x, bb.max.y, bb.max.z);

    return pid;
}

partition_t * partition_t::make_new (int ax, int ay, int az, int level, int parent_pid, const struct bbox_t& abb)
{
    partition_t * p = new partition_t;
    p->init(ax, ay, az, level, parent_pid, abb);
    return p;
}

bbox_t partition_t::gen_bbox_from_parent_box (int ax, int ay, int az, const struct bbox_t& bb)
{
    bbox_t abb;
    abb.min =  vec3_t(bb.min.x + bb.width() * 0.5f * ax, bb.min.y + bb.height() * 0.5f * ay, bb.min.z + bb.depth() * 0.5f * az);
    abb.max =  vec3_t(abb.min.x + bb.width() * 0.5f, abb.min.y + bb.height() * 0.5f, abb.min.z + bb.depth() * 0.5f);
    return abb;
}

size_t partition_t::childs_from (vector<partition_t *> & vp, const partitions_t& p)
{
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
    
    if (lev == MAX_LEVEL) {
        return 0;
    }
    
    for (int i=0; i<8; i++) {
        
        int val = partition_t::make_pid(ppid, indices[0+3*i], indices[1+3*i], indices[2+3*i], lev+1);
        partitions_t::const_iterator iter = p.find(val);
        vp.push_back(iter->second);
    }
    return vp.size();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void partition_generator_t::generate(const world_t &world, partitions_t &container)
{
    bbox_t merged;
    size_t count = world.bbounds.size();
    for (int i=0; i<count; i++) {
        merged = merged.merge(world.bbounds[i]);
    }
}

partition_t * partition_generator_t::gen_partition_at_level (int ax, int ay, int az, int level, const partition_t * parent)
{
    bbox_t abb;
    abb = partition_t::gen_bbox_from_parent_box (ax, ay, az, parent->bb);
    partition_t * partition = partition_t::make_new(ax, ay, az, level, parent->pid, abb);
    return partition;
}

void partition_generator_t::gen_partitions (int start_level, int max_level, partition_t * parent, partitions_t& container)
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

void partition_generator_t::gen_world_partitions (const struct bbox_t& wb, int max_level, partitions_t& container)
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

