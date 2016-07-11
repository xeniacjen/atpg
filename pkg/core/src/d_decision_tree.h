/*
 * =====================================================================================
 *
 *       Filename:  d_decision_tree.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/02/2016 06:05:11 PM
 *       Revision:  none
 *       Compiler:  g++ 
 *
 *         Author:  xenia-cjen (xc), jonah0604@gmail.com
 *        Company:  LaDS(I), GIEE, NTU
 *
 * =====================================================================================
 */

#include <stack>

#include "decision_tree.h" 
#include "gate.h"

namespace CoreNs { 

struct DDNode { 
                 DDNode(const GateVec &df, 
                   const unsigned &startPoint, 
                   const DecisionTree &tree); 
    unsigned     startPoint_; 

    bool         empty() const; 
    Gate        *top() const; 
    void         pop(); 
    void         getJTree(DecisionTree &tree) const; 
  protected: 
    GateVec      dfront_; 

    DecisionTree j_tree_; 
}; // DDNode 

class DDTree {
  public: 
        DDTree() { }; 
        ~DDTree(); 

    void clear(); 
    void push(const GateVec &df, 
           const unsigned &startPoint, 
           const DecisionTree &tree); 
    bool pop(); 
    void top(int &gid, unsigned &startPoint) const; 
    bool empty() const;  

    void GetPath(GateVec &path) const; // retuen path by reference  

  protected: 
    std::vector<DDNode*> trees_; 
}; // DDTree 

inline DDNode::DDNode(const GateVec &df, 
                      const unsigned &startPoint, 
                      const DecisionTree &tree) { 
    dfront_ = df; 
    startPoint_ = startPoint; 
    j_tree_ = tree; 
}

inline bool DDNode::empty() const { 
    return dfront_.empty(); 
}

inline Gate *DDNode::top() const { 
    return dfront_.back(); 
} 

inline void DDNode::pop() { 
    dfront_.pop_back(); 
}

inline void DDNode::getJTree(DecisionTree &tree) const { 
    tree = j_tree_; 
}

inline DDTree::~DDTree() { 
    clear(); 
}

inline void DDTree::clear() { 
    for (size_t i=0; i<trees_.size(); i++) 
        delete trees_[i]; 

    trees_.clear(); 
}

inline void DDTree::push(const GateVec &df, 
                         const unsigned &startPoint, 
                         const DecisionTree &tree) { 
    DDNode* n = new DDNode(df, startPoint, tree); 
    trees_.push_back(n); 
}

inline bool DDTree::pop() { 
    trees_.back()->pop(); 

    if (trees_.back()->empty()) { 
        trees_.pop_back(); 
        return true; 
    }

    return false; 
} 

inline void DDTree::top(int &gid, unsigned& startPoint) const { 
    gid = trees_.back()->top()->id_; 
    startPoint = trees_.back()->startPoint_; 
}

inline bool DDTree::empty() const { 
    return trees_.empty(); 
}

inline void DDTree::GetPath(GateVec &path) const { 
    path.clear(); 

    for (size_t i=0; i<trees_.size(); i++) 
        path.push_back(trees_[i]->top()); 
}

} // CoreNs 
