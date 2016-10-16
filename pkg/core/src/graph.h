/*
 * =====================================================================================
 *
 *       Filename:  graph.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/14/2016 03:51:20 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  xenia-cjen (xc), jonah0604@gmail.com
 *        Company:  LaDS(I), GIEE, NTU
 *
 * =====================================================================================
 */

#ifndef _CORE_GRAPH_H_ 
#define _CORE_GRAPH_H_ 

#include <vector> 
#include <set> 

struct Edge; 
struct VertexBase; 
typedef std::vector<Edge *> EdgeVec; 
typedef std::set<VertexBase *> VertexSet; 

struct VertexBase { 
    EdgeVec     es_; 

    size_t      GetNNeighbor() const; 
    VertexBase *GetNeighbor(size_t n) const; 
    void        GetNeighborSet(VertexSet &vs) const; 
};  

template <class T>
struct Vertex : public VertexBase {
          Vertex<T>() { }
		  Vertex<T>(T data) : data_(data) { }

    T     data_;
};

struct Edge { 
    VertexBase *v1_; 
    VertexBase *v2_; 
}; 

inline size_t VertexBase::GetNNeighbor() const { 
    return es_.size(); 
}

inline VertexBase *VertexBase::GetNeighbor(size_t n) const { 
    if (n>=es_.size()) return NULL; 

    VertexBase *v = es_[n]->v1_; 
    
    return (v==this)?es_[n]->v2_:v;
}

inline void VertexBase::GetNeighborSet(VertexSet &vs) const { 
    vs.clear(); 
    for (size_t n=0; n<es_.size(); n++) { 
        VertexBase *v = GetNeighbor(n); 
        vs.insert(v); 
    }
} 

#endif 
