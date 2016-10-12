/*
 * =====================================================================================
 *
 *       Filename:  b_decision_tree.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/12/2016 02:59:07 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  xenia-cjen (xc), jonah0604@gmail.com
 *        Company:  LaDS(I), GIEE, NTU
 *
 * =====================================================================================
 */

#include "learn.h" 

namespace CoreNs { 

typedef std::vector<Objective> ObjVec; 

struct BDNode { 
        BDNode(const ObjVec &candi); 
        ~BDNode() { } 

    bool empty() const; 

    void top(Objective& obj); 
    void pop(); 
    
  protected: 
    ObjVec   candidates_;  
}; // BDNode 

class BDTree { 
  public: 
    BDTree() { } 
    ~BDTree(); 

    bool empty() const; 
    void clear(); 
    
    void push(const ObjVec& objs); 
    bool pop(); 
    void top(Objective& obj); 

  protected: 
    std::vector<BDNode *> tree_; 
}; // BDTree

inline BDTree::~BDTree() { 
    for (size_t n=0; n<tree_.size(); n++) 
        delete tree_[n]; 

    clear(); 
}

inline BDNode::BDNode(const ObjVec &candi) { 
    candidates_ = candi; 
}

inline bool BDNode::empty() const { return candidates_.empty(); }

inline void BDNode::top(Objective& obj) { obj = candidates_.back(); } 

inline void BDNode::pop() { candidates_.pop_back(); }

inline bool BDTree::empty() const { return tree_.empty(); } 

inline void BDTree::clear() { tree_.clear(); } 

inline void BDTree::push(const ObjVec& objs) { 
    BDNode *n = new BDNode(objs); 
    tree_.push_back(n); 
}

inline bool BDTree::pop() { 
    tree_.back()->pop(); 

    if (tree_.back()->empty()) { 
        delete tree_.back(); 
        tree_.pop_back(); 

        return true; 
    }

    return false; 
}

inline void BDTree::top(Objective& obj) { tree_.back()->top(obj); }

}; // CoreNs 
