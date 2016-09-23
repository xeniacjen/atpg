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

#include <set>
#include <map>

#include "decision_tree.h" 
#include "fault.h"

namespace CoreNs { 

typedef std::set<int> GateSet; 
typedef std::set<Fault *> FaultSet; 
typedef std::map<Gate *, GateSet> GateSetMap; 
typedef std::map<Gate *, FaultSet> FaultSetMap; 
typedef FaultSetMap::iterator FaultSetMapIter; 
typedef GateSetMap::iterator GateSetMapIter; 

struct DDNode { 
                 DDNode(const GateVec &df, 
                   const unsigned &startPoint, 
                   const DecisionTree &tree); 

                 ~DDNode(); 

    unsigned     startPoint_; 

    bool         empty() const; 
    Gate        *top() const; 
    void         top(GateVec &gids) const;        
    void         top(GateSet &gs) const; 
    void         pop(); 
    void         getJTree(DecisionTree &tree) const; 

    void         set_mask_(Value *mask); 
    Value       *get_mask_(size_t& n) const; 

    void         get_pred(GateSet &gs); 
    // void         get_fs(FaultSet &fs); 

    // void         set_f2p(FaultSetMap &f2p);

    // FaultSetMap  fault_to_prop_; 
    // FaultSetMap  fault_proped_; 
    GateSetMap   predecessor_; 

    GateVec      dfront_; 
  protected: 
    Value       *d_mask_; // indicate which gates to d-drive 

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
    bool pop(DecisionTree &tree); 
    void pop_hard(DecisionTree &tree); 
    unsigned top(int &gid) const; 
    unsigned top(GateVec &gids) const; 
    DDNode *top(); 
    DDNode *sub_top(); 
    bool empty() const;  

    void GetPath(GateVec &path) const; // retuen path by reference  
    void GetMultiPath(GateVec &path) const; // retuen path by reference  

  protected: 
    std::vector<DDNode*> trees_; 
}; // DDTree 

inline DDNode::DDNode(const GateVec &df, 
                      const unsigned &startPoint, 
                      const DecisionTree &tree) { 
    dfront_ = df; 
    startPoint_ = startPoint; 
    j_tree_ = tree; 

    d_mask_ = NULL; 
}

inline DDNode::~DDNode() { 
    if (d_mask_) delete [] d_mask_; 
}

inline bool DDNode::empty() const { 
    return dfront_.empty(); 
}

inline Gate *DDNode::top() const { 
    return dfront_.back(); 
} 

inline void DDNode::top(GateVec &gids) const { 
    gids.clear(); 
    for (size_t i=0; i<dfront_.size(); i++) 
        if (d_mask_[i]==H || d_mask_[i]==X)
            gids.push_back(dfront_[i]); 
} 

inline void DDNode::top(GateSet &gs) const {
    gs.clear(); 
    for (size_t i=0; i<dfront_.size(); i++) 
        if (d_mask_[i]==H || d_mask_[i]==X)
            gs.insert(dfront_[i]->id_); 
} 

inline void DDNode::pop() { 
    dfront_.pop_back(); 
}

inline void DDNode::getJTree(DecisionTree &tree) const { 
    tree = j_tree_; 
}

inline void DDNode::set_mask_(Value *mask) { 
    d_mask_ = mask; 
}

inline Value *DDNode::get_mask_(size_t& n) const { 
    n = dfront_.size(); 

    return d_mask_; 
}

/** 
inline void DDNode::set_f2p(FaultSetMap &f2p) { 
    fault_to_prop_.clear(); 
    
    FaultSetMapIter it; 
    for (size_t i=0; i<dfront_.size(); i++) { 
        Gate *g = dfront_[i]; 
        it = f2p.find(g); 
        if (it!=f2p.end()) { 
            fault_to_prop_.insert(std::pair<Gate *, FaultSet>(
              it->first, it->second)); 
        } 
        else assert(0); 
    }
    for (it=f2p.begin(); it!=f2p.end(); ++it) { 
        Gate *g = it->first; 
        if (g->type_==Gate::PO || g->type_==Gate::PPO) { 
            FaultSetMapIter itg = fault_proped_.find(g); 
            if (itg!=fault_proped_.end()) { 
                itg->second.insert(it->second.begin(), it->second.end()); 
            } 
            else { 
                fault_proped_.insert(std::pair<Gate *, FaultSet>(
                  it->first, it->second)); 
            }
        }
    }
}
*/ 

/**
inline void DDNode::get_fs(FaultSet &fs) { 
    fs.clear(); 

    FaultSetMapIter it; 
    for (it=fault_proped_.begin(); it!=fault_proped_.end(); ++it) { 
        fs.insert(it->second.begin(), it->second.end()); 
    }
}
*/ 

inline void DDNode::get_pred(GateSet &gs) { 
    gs.clear(); 

    GateSetMapIter it; 
    for (size_t i=0; i<dfront_.size(); i++) { 
        if (d_mask_[i]==H || d_mask_[i]==X) { 
            it = predecessor_.find(dfront_[i]); 
            if (it!=predecessor_.end()) { 
                gs.insert(it->second.begin(), it->second.end()); 
            }
            else assert(0); 
        }
    } 

    for (it=predecessor_.begin(); it!=predecessor_.end(); ++it) { 
        Gate *g = it->first; 
        if (g->type_==Gate::PO || g->type_==Gate::PPO) { 
            gs.insert(it->second.begin(), it->second.end()); 
        }
    }
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

inline bool DDTree::pop(DecisionTree &tree) { 
    trees_.back()->getJTree(tree); 
    trees_.back()->pop(); 

    if (trees_.back()->empty()) { 
        delete trees_.back(); 
        trees_.pop_back(); 
        return true; 
    }

    return false; 
} 

inline void DDTree::pop_hard(DecisionTree &tree) { 
    trees_.back()->getJTree(tree); 
    
    delete trees_.back(); 
    trees_.pop_back(); 
}

inline unsigned DDTree::top(int &gid) const { 
    gid = trees_.back()->top()->id_; 
    
    return trees_.back()->startPoint_; 
}

inline unsigned DDTree::top(GateVec &gids) const {
    trees_.back()->top(gids); 

    return trees_.back()->startPoint_; 
}

inline DDNode *DDTree::top() { 
    return trees_.back(); 
}

inline DDNode *DDTree::sub_top() { 
    return trees_[trees_.size()-2]; 
}

inline bool DDTree::empty() const { 
    return trees_.empty(); 
}

inline void DDTree::GetPath(GateVec &path) const { 
    path.clear(); 

    for (size_t i=0; i<trees_.size()-1; i++) 
        path.push_back(trees_[i]->top()); 
}

inline void DDTree::GetMultiPath(GateVec &path) const { 
    path.clear(); 

    for (size_t i=0; i<trees_.size()-1; i++) { 
        GateVec gids; 
        trees_[i]->top(gids); 
        path.insert(path.end(), gids.begin(), gids.end()); 
    }
}

} // CoreNs 
