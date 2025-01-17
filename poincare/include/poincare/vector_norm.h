#ifndef POINCARE_VECTOR_NORM_H
#define POINCARE_VECTOR_NORM_H

#include <poincare/expression.h>

namespace Poincare {

class VectorNormNode final : public ExpressionNode {
public:

  // TreeNode
  size_t size() const override { return sizeof(VectorNormNode); }
  int numberOfChildren() const override;
#if POINCARE_TREE_LOG
  void logNodeName(std::ostream & stream) const override {
    stream << "VectorNorm";
  }
#endif

  // Properties
  Type type() const override { return Type::VectorNorm; }
private:
  // Layout
  Layout createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;
  int serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;
  // Simplification
  Expression shallowReduce(ReductionContext reductionContext) override;
  LayoutShape leftLayoutShape() const override { return LayoutShape::BoundaryPunctuation; }
  // Evaluation
  Evaluation<float> approximate(SinglePrecision p, Context * context, Preferences::ComplexFormat complexFormat, Preferences::AngleUnit angleUnit) const override { return templatedApproximate<float>(context, complexFormat, angleUnit); }
  Evaluation<double> approximate(DoublePrecision p, Context * context, Preferences::ComplexFormat complexFormat, Preferences::AngleUnit angleUnit) const override { return templatedApproximate<double>(context, complexFormat, angleUnit); }
  template<typename T> Evaluation<T> templatedApproximate(Context * context, Preferences::ComplexFormat complexFormat, Preferences::AngleUnit angleUnit) const;
};

class VectorNorm final : public Expression {
public:
  VectorNorm(const VectorNormNode * n) : Expression(n) {}
  static VectorNorm Builder(Expression child) { return TreeHandle::FixedArityBuilder<VectorNorm, VectorNormNode>({child}); }

  static constexpr Expression::FunctionHelper s_functionHelper = Expression::FunctionHelper("norm", 1, &UntypedBuilderOneChild<VectorNorm>);

  Expression shallowReduce(ExpressionNode::ReductionContext reductionContext);
};

}

#endif
