#include <torch/csrc/jit/fuser/common/fusion.h>
#include <torch/csrc/jit/fuser/common/ir.h>
#include <torch/csrc/jit/fuser/common/iter_visitor.h>
#include <torch/csrc/jit/fuser/common/type.h>
#include <deque>
#include <iostream>

#include <torch/csrc/jit/fuser/common/iriostream.h>

namespace torch {
namespace jit {
namespace fuser {

std::vector<const Statement*> IterVisitor::next(
    const Statement* const statement) {
  if (statement->isVal())
    return next(static_cast<const Val*>(statement));
  else if (statement->isExpr())
    return next(static_cast<const Expr*>(statement));
  else
    throw std::runtime_error("Could not detect type in next_dispatch.");
}

std::vector<const Statement*> IterVisitor::next(const Val* const v) {
  if (FusionGuard::getCurFusion()->origin(v) != nullptr)
    return {FusionGuard::getCurFusion()->origin(v)};
  return {};
}

std::vector<const Statement*> IterVisitor::next(const Expr* const expr) {
  return {expr->inputs().begin(), expr->inputs().end()};
}

void IterVisitor::handle(const Statement* const stmt) {
  stmt->dispatch(this);
}

void IterVisitor::handle(const Expr* const expr) {
<<<<<<< HEAD
  expr->dispatch(this);
=======
   switch (*(expr->getExprType())) {
    case ExprType::UnaryOp:
      return handle(static_cast<const UnaryOp*>(expr));
    case ExprType::BinaryOp:
      return handle(static_cast<const BinaryOp*>(expr));
    default:
      throw std::runtime_error("Unknown ExprType in handle(Expr).");
    }
>>>>>>> Create BinaryOp and UnaryOp Exprs.
}

void IterVisitor::handle(const Val* const val) {
  val->dispatch(this);
}

void IterVisitor::handle(const TensorDomain* const t) {}

void IterVisitor::handle(const TensorView* const t) {}

void IterVisitor::handle(const IterDomain* const t) {}

void IterVisitor::handle(const Tensor* const t) {}

void IterVisitor::handle(const Float* const f) {}

void IterVisitor::handle(const Int* const i) {}

void IterVisitor::handle(const UnaryOp* const uop) {}

void IterVisitor::handle(const BinaryOp* const bop) {}

void IterVisitor::handle(const Split* const split) {}

void IterVisitor::handle(const Merge* const merge) {}

void IterVisitor::handle(const Reorder* const reoder) {}

void IterVisitor::traverse(
    const Fusion* const fusion,
    std::vector<const Val*> from) {
  std::set<const Statement*> visited;
  std::deque<const Statement*> to_visit;

  if (FusionGuard::getCurFusion() != fusion)
    throw std::runtime_error("fusion is not active.");
  std::queue<const Val*> outputs_to_visit;
  for (const Val* entry : from)
    outputs_to_visit.emplace(entry);

  while (!outputs_to_visit.empty()) {
    to_visit.push_front(outputs_to_visit.front());
    outputs_to_visit.pop();

    while (!to_visit.empty()) {
      const Statement* stmt = to_visit.front();
      std::vector<const Statement*> inps = next(stmt);
      for (auto it = inps.rbegin(); it != inps.rend(); it++){
        const Statement* inp = *it;
        if (visited.find(inp) == visited.end()) {
          to_visit.emplace_front(inp);
        }
      }

      if (to_visit.front() != stmt) {
        continue;
      }

      to_visit.pop_front();
      if (visited.find(stmt) == visited.end()) {
        handle(stmt);
        visited.emplace(stmt);
      }
    }
  }
}

void IterVisitor::traverse(
    const Fusion* const fusion,
    bool from_outputs_only,
    bool breadth_first) {
  if (breadth_first)
    throw std::runtime_error("Not implemented yet.");
  std::set<const Statement*> visited;
  std::deque<const Statement*> to_visit;

  if (FusionGuard::getCurFusion() != fusion)
    throw std::runtime_error("fusion is not active.");

  std::vector<const Val*> outputs_to_visit;

  if (from_outputs_only) {
    for (const Val* out : fusion->outputs())
      outputs_to_visit.push_back(out);
  } else
    for (const Val* it : fusion->vals()) {
      const std::set<const Expr*>& uses = fusion->uses(it);
      if (uses.empty())
        outputs_to_visit.push_back(it);
    }

  traverse(fusion, outputs_to_visit);
}



void DependencyCheck::handle(const Val* val){
  IterVisitor::handle(val);
  if(val->same_as(dependency_))
    is_dependency = true;
}

bool DependencyCheck::check(){
  is_dependency = false;
  IterVisitor::traverse(FusionGuard::getCurFusion(), {of_});
  return is_dependency;
}

} // namespace fuser
} // namespace jit
} // namespace torch
