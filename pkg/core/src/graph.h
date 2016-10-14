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

struct Edge; 
typedef std::vector<Edge *> EdgeVec; 

struct VertexBase { 
    EdgeVec es_; 
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

#endif 
