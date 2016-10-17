#ifndef _CLIQUE_PART_H_ 
#define _CLIQUE_PART_H_

#include <cstdio> 
#include <cstdlib>
#include <cassert>

/****************************************************************************
 *  This is a C implementation of the Tseng and Seiworick's Clique 
 *  partitioning heuristic, as reported in the following publication:  
 *       C. Tseng and D. P. Siewiorek, ``Facet: A Procedure for the 
 *      Automated Synthesis of Digital Systems,'' DAC, 1983, pp. 490-496.
 *
 *  Author: Srinivas Katkoori, 
 *          University of South Florida,
 *          katkoori@csee.usf.edu.
 * 
 *  Date: June 2001
 *  
 *  Implementation Details:
 *   
 *   Input: Two dimensional symmetric compatibility array 
 *   where compat[i][j] = 1  if nodes i and j are compatible 
 *                     = 0  otherwise  
 *
 *   Output: Set of cliques
 *   A global array "clique_set" stores the results.
 *   Each clique has a (1) list of members, a integer array of nodeid's 
 *   (2) the number of members information.
 *   All the entries are initialized to UNKNOWN values.
 *
 *   o The maximum number of cliques is 200.  This can be changes by 
 *     changing the hash define value of MAXCLIQUES.
 *   o Call clique_partition(compatibility array, nodesize) 
 *   o The output can be printed using print_clique_set() function.
 *   o Compile this code without DEBUG option
 *
 *       unix% gcc -g clique_partition.c
 *
 *   o To enable DEBUG option:
 *       
 *       unix% gcc -g -DDEBUG clique_partition.c 
 *    
 *  Modification History:
 *   
 *  Acknowledgment:
 *--  This code is developed as part of the AUDI (AUtomatic 	    -- 	
 *--  Design Instantiation) system, a behavioral synthesis system,  -- 	
 *--  developed at the University of South Florida.  The AUDI project 
 *--  is supported by the National Science Foundation (NSF) under   -- 	
 *--  the (CAREER) grant number 0093301.  If you have any questions,-- 
 *--  contact: 
 *--     Dr. Srinivas Katkoori,                                     -- 	
 *--     4202 East Fowler Avenue, ENB 118,                          --
 *--     Computer Science & Engineering Department,                 -- 	
 *--     University of South Florida,                               -- 	
 *--     Tampa, FL 33647.					    -- 	
 *--     Email: katkoori@csee.usf.edu				    --	
 *-- 								    -- 	
 ****************************************************************************
 *  Psuedo code of the Clique Partitioning heuristic
 ****************************************************************************
 *
 *  Clique Partition G(V,E)
 *  begin
 *  N <- V  / make a local copy of V
 *  for n in N 
 *    priority(n) <- 0
 *  end for
 *  while (N is not empty) do
 *    x <- select_node() / if a node with priority, then pick that node 
 *                       / else a node with highest degree
 *                       /        if multiple nodes then pick a node
 *                       /           with highest neighbor wt
 *                       /           if multiple, pick one randomly.   
 *    Y <- { y | (x, y) is in E}
 *    if | Y | = 0 
 *    then
 *       N <- N - {x}  / x represents a clique
 *    else
 *      for y in Y do 
 *         I_y <- { z | (y,z) is not in E}
 *      end for
 *
 *      min_val  <- min{ | intersection(I_y, Y) |  y in Y}
 *      Y1 <- { y | y in Y and | intersection (I_y, Y) | = min_val}
 *       
 *      max_val  <- max{ |I_y| for y in Y1} 
 *      Y2 <- { y | y in Y1 and |I_y| = max_val} 
 *        
 *      y <- a node arbitrarily selected from Y2
 *      
 *      / merge nodes x and y into z 
 *      Create a new node z
 *      priority (z) <-  1
 *      For every node n which is connected to both x and y, 
 *        create an edge connecting n and z 
 *      Delete nodes x and y and all edges connected to them 
 *    end if
 *   end while 
 *  end
 *****************************************************************************
 */

#define UNKNOWN -12345
#define MAXCLIQUES 1024 

#define CLIQUE_UNKNOWN -12345  
#define CLIQUE_TRUE 100
#define CLIQUE_FALSE 110 

#include <vector> 
#include <set> 

typedef std::set<std::pair<size_t, size_t> > UGraph; 
typedef UGraph::iterator UGraphIter; 

struct CompGraph {
    bool get(size_t i, size_t j) const; 
    void add_edge(size_t i, size_t j); 
    
    void copy(const CompGraph& g); 
private: 
    UGraph graph_; 
}; 

struct clique
{
    std::vector<int> members;          /* members of the clique */
    int size;                          /* number of members in the clique */
};

class CliquePartition { 
public:  
    int clique_partition(const CompGraph& compat, int nodesize); 
    std::vector<clique> clique_set;   /* stores the clique partitioning results */

private: 
    // int input_sanity_check(CompGraph& compat, int array_dimension); 
    // int output_sanity_check(int array_dimension, CompGraph& local_compat, CompGraph& compat); 
    // void make_a_local_copy(CompGraph& local_compat, const CompGraph& compat, int nodesize); 
    int get_degree_of_a_node(int x, int nodesize, const CompGraph& local_compat, int* node_set); 
    int select_new_node(const CompGraph& local_compat, int nodesize, int* node_set); 
    int form_setY(int* setY, int* current_clique, const CompGraph& local_compat, int nodesize, int* node_set); 
    void print_setY(int* setY); 
    void form_set_Y1(int nodesize, int* set_Y, int* set_Y1, int** sets_I_y, int* node_set); 
    void form_set_Y2(int nodesize, int* set_Y2, int* set_Y1, int* sizes_of_sets_I_y); 
    int pick_a_node_to_merge(int* setY, const CompGraph& local_compat, int* node_set, int nodesize); 
    void print_clique_set(); 
}; 

inline bool CompGraph::get(size_t i, size_t j) const { 
    if (i==j) return true; 

    UGraphIter it = (j>i)?graph_.find(std::make_pair(i, j))
                         :graph_.find(std::make_pair(j, i)); // i>j 
    return (it!=graph_.end()); 
} 

inline void CompGraph::add_edge(size_t i, size_t j) { 
    assert(i<j); 
    graph_.insert(std::make_pair(i, j)); 
}

inline void CompGraph::copy(const CompGraph& g) { 
    graph_ = g.graph_; 
}


#endif // _CLIQUE_PART_H_ 
