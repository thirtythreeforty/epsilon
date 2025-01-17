#include <poincare/sum_and_product.h>
#include <poincare/decimal.h>
#include <poincare/undefined.h>
#include <poincare/variable_context.h>
extern "C" {
#include <assert.h>
#include <stdlib.h>
}
#include <cmath>

namespace Poincare {

Layout SumAndProductNode::createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return createSumAndProductLayout(
    childAtIndex(0)->createLayout(floatDisplayMode, numberOfSignificantDigits),
    childAtIndex(1)->createLayout(floatDisplayMode, numberOfSignificantDigits),
    childAtIndex(2)->createLayout(floatDisplayMode, numberOfSignificantDigits),
    childAtIndex(3)->createLayout(floatDisplayMode, numberOfSignificantDigits)
  );
}

Expression SumAndProductNode::shallowReduce(ReductionContext reductionContext) {
  return SumAndProduct(this).shallowReduce(reductionContext.context());
}

template<typename T>
Evaluation<T> SumAndProductNode::templatedApproximate(Context * context, Preferences::ComplexFormat complexFormat, Preferences::AngleUnit angleUnit) const {
  Evaluation<T> aInput = childAtIndex(2)->approximate(T(), context, complexFormat, angleUnit);
  Evaluation<T> bInput = childAtIndex(3)->approximate(T(), context, complexFormat, angleUnit);
  T start = aInput.toScalar();
  T end = bInput.toScalar();
  if (std::isnan(start) || std::isnan(end) || start != (int)start || end != (int)end || end - start > k_maxNumberOfSteps) {
    return Complex<T>::Undefined();
  }
  SymbolNode * symbol = static_cast<SymbolNode *>(childAtIndex(1));
  VariableContext nContext = VariableContext(symbol->name(), context);
  Evaluation<T> result = Complex<T>::Builder((T)emptySumAndProductValue());
  for (int i = (int)start; i <= (int)end; i++) {
    if (Expression::ShouldStopProcessing()) {
      return Complex<T>::Undefined();
    }
    nContext.setApproximationForVariable<T>((T)i);
    Expression child = Expression(childAtIndex(0)).clone();
    if (child.type() == ExpressionNode::Type::Sequence) {
      /* Since we cannot get the expression of a sequence term like we would for
      * a function, we replace its potential abstract rank by the value it should
      * have. We can then evaluate its value */
      child.childAtIndex(0).replaceSymbolWithExpression(symbol, Float<T>::Builder(i));
    }
    result = evaluateWithNextTerm(T(), result, child.node()->approximate(T(), &nContext, complexFormat, angleUnit), complexFormat);
    if (result.isUndefined()) {
      return Complex<T>::Undefined();
    }
  }
  return result;
}

Expression SumAndProduct::shallowReduce(Context * context) {
  {
    Expression e = Expression::defaultShallowReduce();
    e = e.defaultHandleUnitsInChildren();
    if (e.isUndefined()) {
      return e;
    }
  }
  assert(!childAtIndex(1).deepIsMatrix(context));
  if (childAtIndex(2).deepIsMatrix(context) || childAtIndex(3).deepIsMatrix(context)) {
    return replaceWithUndefinedInPlace();
  }
  return *this;
}

template Evaluation<float> SumAndProductNode::templatedApproximate(Context * context, Preferences::ComplexFormat complexFormat, Preferences::AngleUnit angleUnit) const;
template Evaluation<double> SumAndProductNode::templatedApproximate(Context * context, Preferences::ComplexFormat complexFormat, Preferences::AngleUnit angleUnit) const;

}