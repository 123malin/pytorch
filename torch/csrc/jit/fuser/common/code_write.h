#pragma once

#include <torch/csrc/jit/fuser/common/arith.h>
#include <torch/csrc/jit/fuser/common/index_compute.h>
#include <torch/csrc/jit/fuser/common/iriostream.h>
#include <torch/csrc/jit/fuser/common/iter_visitor.h>
#include <torch/csrc/jit/fuser/common/predicate_compute.h>
#include <torch/csrc/jit/fuser/common/transform_iter.h>

#include <ostream>
#include <stack>
#include <map>

namespace torch {
namespace jit {
namespace fuser {

/*
std::ostream& operator<<(std::ostream& os, std::vector<Int*> vec) {
  os << "<";
  for (int i = 0; i < vec.size(); i++) {
    IRPrinter(os).print_inline(vec[i]);
    if (i == vec.size() - 1)
      os << ">";
    else
      os << ",";
  }
  return os;
}
*/

//Run through and grab all values that ar eused in this fusion based on
//the registered outputs.
struct FindUsedVals : public IterVisitor {
  std::set<Val*> used_vals;

  void handle(Val* v) {
    used_vals.emplace(v);
  }

 public:
  static std::set<Val*> find() {
    FindUsedVals finder;
    finder.traverse(FusionGuard::getCurFusion(), true);
    return finder.used_vals;
  }

};

struct TORCH_API CodeWrite : public IRPrinter {
 private:
  //Check if expr is a TensorView operation we can print
  bool isTVOp(const Expr* expr);


  /*****PRINTING FUNCTIONS****/
  //Print the indexing into a TensorView
  void printIndexInto(std::vector<Int*> indices, const TensorView* const);
  //Compute and print the predicate based on accessing a specific TensorView
  bool print_predicate(const TensorView* const);

  //Print the allocation of a register space
  void printAlloc(TensorView*);
  // Print lhs of uop/bop, returns if predicate was needed
  bool printConsumer(TensorView*);
  //Printing functions for TensorView ops
  void handle(const TensorView* const);
  //Check overrides before printing a value
  void handle(const Val* const);
  void handle(const UnaryOp* const);
  void handle(const BinaryOp* const);

  //Ignore split/merge/reorder operations,
  //we don't want to print them directly
  void handle(const Split* const) {}
  void handle(const Merge* const) {}
  void handle(const Reorder* const) {}

  //Indent the generated code
  void indent();

  //Update the for loop structure based on provided TensorView
  void updateView(TensorView*);

  //Grab all the indices used in the current for loop structure
  std::vector<Int*> getLoopIndices();
  //Open a new inner most for loop
  void openFor(IterDomain*);
  //Close the inner most for loop
  void closeFor();
  //Close all for loops
  void resetFors();
  //Clear out the last recorded computeAtView
  void clearActiveView();

  //Mark if the TensorView I'm printing is a producer
  bool producer = false;
  //Track the TensorView that is consuming the current producer
  TensorView* consumer = nullptr;

  //Track the for loops
  std::vector<const ForLoop*> fors;
  //Track the indentation size for pretty printing
  int indent_size = 0;

  //Track the last computeAt TensorView and axis
  const TensorView* active_view = nullptr;
  int active_view_axis = 0;

  //Mark if I want to reset all fors next time I call updateView
  bool reset_fors = false;

  //Track all 
  std::set<IterDomain*> bound_iters;

  //Print std::string instead of Val
  std::map<const Val* const, std::string> overrides;

  //Set override for thread/block axes
  void bind(IterDomain* id, Val* iterator);
  
  // Grab all values that are used. Look for Tensors
  // to set Int* -> Tensor->size(i)
  // Grab all IterDoms that are used. Look for any
  // mappings to threadIdx.x / blockIdx.x
  // Add them to bound_iters
  void setupOverrides();

  //wrapper for overrides.find on non-const vals
  std::map<const Val* const, std::string>::iterator
    overrides_find(Val* val){
      return overrides.find(const_cast<const Val* const>(val));
    }
  //wrapper for override.emplace on non-const vals
  void overrides_emplace(Val* val, std::string str){
    overrides[const_cast<const Val* const>(val)] = str;
  }

  //Print the header of the kernel
  void header();

 public:
  //Init printer on ostream
  CodeWrite(std::ostream& _os) : IRPrinter(_os) {}

  //print generated code to ostream
  void traverse(Fusion* fusion);
};

} // namespace fuser
} // namespace jit
} // namespace torch
