#include "unit_list_controller.h"
#include "../app.h"
#include "../../shared/poincare_helpers.h"
#include <poincare/unit_convert.h>
#include <poincare/multiplication.h>
#include <poincare/power.h>
#include <poincare/undefined.h>
#include <poincare/unit.h>

using namespace Poincare;
using namespace Shared;

namespace Calculation {

void UnitListController::setExpression(Poincare::Expression e) {
  ExpressionsListController::setExpression(e);
  assert(!m_expression.isUninitialized());
  static_assert(k_maxNumberOfRows >= 3, "k_maxNumberOfRows must be greater than 3");

  Poincare::Expression expressions[k_maxNumberOfRows];
  // Initialize expressions
  for (size_t i = 0; i < k_maxNumberOfRows; i++) {
    expressions[i] = Expression();
  }

  /* 1. First rows: miscellaneous classic units for some dimensions, in both
   * metric and imperial units. */
  Expression copy = m_expression.clone();
  Expression units;
  // Reduce to be able to recognize units
  PoincareHelpers::Reduce(&copy, App::app()->localContext(), ExpressionNode::ReductionTarget::User);
  copy = copy.removeUnit(&units);
  double value = Shared::PoincareHelpers::ApproximateToScalar<double>(copy, App::app()->localContext());
  ExpressionNode::ReductionContext reductionContext(
      App::app()->localContext(),
      Preferences::sharedPreferences()->complexFormat(),
      Preferences::sharedPreferences()->angleUnit(),
      GlobalPreferences::sharedGlobalPreferences()->unitFormat(),
      ExpressionNode::ReductionTarget::User,
      ExpressionNode::SymbolicComputation::ReplaceAllSymbolsWithDefinitionsOrUndefined);
  int numberOfExpressions = Unit::SetAdditionalExpressions(units, value, expressions, k_maxNumberOfRows, reductionContext);

  // 2. SI units only
  assert(numberOfExpressions < k_maxNumberOfRows - 1);
  expressions[numberOfExpressions] = m_expression.clone();
  Shared::PoincareHelpers::Simplify(&expressions[numberOfExpressions], App::app()->localContext(), ExpressionNode::ReductionTarget::User, Poincare::ExpressionNode::SymbolicComputation::ReplaceAllDefinedSymbolsWithDefinition, Poincare::ExpressionNode::UnitConversion::InternationalSystem);
  numberOfExpressions++;

  /* 3. Get rid of duplicates
   * We find duplicates by comparing the serializations, to eliminate
   * expressions that only differ by the types of their number nodes. */
  Expression reduceExpression = m_expression.clone();
  // Make m_expression comparable to expressions (turn BasedInteger into Rational for instance)
  Shared::PoincareHelpers::Simplify(&reduceExpression, App::app()->localContext(), ExpressionNode::ReductionTarget::User, Poincare::ExpressionNode::SymbolicComputation::ReplaceAllDefinedSymbolsWithDefinition, Poincare::ExpressionNode::UnitConversion::None);
  int currentExpressionIndex = 0;
  while (currentExpressionIndex < numberOfExpressions) {
    bool duplicateFound = false;
    constexpr int buffersSize = Constant::MaxSerializedExpressionSize;
    char buffer1[buffersSize];
    int size1 = PoincareHelpers::Serialize(expressions[currentExpressionIndex], buffer1, buffersSize);
    for (int i = 0; i < currentExpressionIndex + 1; i++) {
      // Compare the currentExpression to all previous expressions and to m_expression
      Expression comparedExpression = i == currentExpressionIndex ? reduceExpression : expressions[i];
      assert(!comparedExpression.isUninitialized());
      char buffer2[buffersSize];
      int size2 = PoincareHelpers::Serialize(comparedExpression, buffer2, buffersSize);
      if (size1 == size2 && strcmp(buffer1, buffer2) == 0) {
        numberOfExpressions--;
        // Shift next expressions
        for (int j = currentExpressionIndex; j < numberOfExpressions; j++) {
          expressions[j] = expressions[j+1];
        }
        // Remove last expression
        expressions[numberOfExpressions] = Expression();
        // The current expression has been discarded, no need to increment the current index
        duplicateFound = true;
        break;
      }
    }
    if (!duplicateFound) {
      // The current expression is not a duplicate, check next expression
      currentExpressionIndex++;
    }
  }
  // Memoize layouts
  for (size_t i = 0; i < k_maxNumberOfRows; i++) {
    if (!expressions[i].isUninitialized()) {
      m_layouts[i] = Shared::PoincareHelpers::CreateLayout(expressions[i]);
    }
  }
}

I18n::Message UnitListController::messageAtIndex(int index) {
  return (I18n::Message)0;
}

}
