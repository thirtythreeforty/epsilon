#ifndef POINCARE_EXPRESSION_REFERENCE_H
#define POINCARE_EXPRESSION_REFERENCE_H

#include <poincare/tree_by_reference.h>
#include <poincare/preferences.h>
#include <poincare/print_float.h>
#include <poincare/expression_node.h>

#include <stdio.h>

namespace Poincare {

class Context;

class Expression : public TreeByReference {
  // TODO clean friends
  friend class AbsoluteValue;
  friend class Addition;
  friend class ArcCosine;
  friend class ArcSine;
  friend class ArcTangent;
  friend class Arithmetic;
  friend class BinomialCoefficient;
  friend class Ceiling;
  friend class ComplexArgument;
  friend class Conjugate;
  friend class Cosine;
  friend class Derivative;
  friend class Determinant;
  friend class Division;
  friend class DivisionQuotient;
  friend class Sine;
  friend class Tangent;

  friend class ExpressionNode;
  friend class EqualNode;
  friend class SymbolNode;
  friend class ConfidenceInterval;
  friend class Multiplication;
  friend class Subtraction;
  friend class Addition;
  friend class AdditionNode;
  friend class IntegralNode;
  friend class Opposite;
  friend class Number;
  friend class Symbol;
  friend class Decimal;
  friend class DerivativeNode;
  friend class NAryExpressionNode;
  friend class Parenthesis;
  friend class Power;
  friend class Matrix;
  friend class Trigonometry;
template<int T>
  friend class LogarithmNode;
  friend class NaperianLogarithmNode;
template<typename T>
  friend class ExceptionExpressionNode;
public:
  static bool isExpression() { return true; }
  /* Constructor & Destructor */
  Expression();
  virtual ~Expression() = default;
  Expression clone() const { TreeByReference c = TreeByReference::clone(); return static_cast<Expression&>(c); }
  static Expression parse(char const * string);

  /* Circuit breaker */
  typedef bool (*CircuitBreaker)();
  static void setCircuitBreaker(CircuitBreaker cb);
  static bool shouldStopProcessing();

  /* Properties */
  ExpressionNode::Type type() const { return node()->type(); }
  ExpressionNode::Sign sign() const { return node()->sign(); }
  bool isUndefinedOrAllocationFailure() const { return node()->type() == ExpressionNode::Type::Undefined || node()->type() == ExpressionNode::Type::AllocationFailure; }
  bool isNumber() const { return node()->isNumber(); }
  bool isRationalZero() const;
  typedef bool (*ExpressionTest)(const Expression e, Context & context);
  bool recursivelyMatches(ExpressionTest test, Context & context) const;
  bool isApproximate(Context & context) const;
  static bool IsMatrix(const Expression e, Context & context);
  /* 'characteristicXRange' tries to assess the range on x where the expression
   * (considered as a function on x) has an interesting evolution. For example,
   * the period of the function on 'x' if it is periodic. If
   * the function is x-independent, the return value is 0.0f (because any
   * x-range is equivalent). If the function does not have an interesting range,
   * the return value is NAN.
   * NB: so far, we consider that the only way of building a periodic function
   * is to use sin/tan/cos(f(x)) with f a linear function. */
  float characteristicXRange(Context & context, Preferences::AngleUnit angleUnit) const { return this->node()->characteristicXRange(context, angleUnit); }
  /* polynomialDegree returns:
   * - (-1) if the expression is not a polynome
   * - the degree of the polynome otherwise */
  int polynomialDegree(char symbolName) const { return this->node()->polynomialDegree(symbolName); }
  /* getVariables fills the table variables with the variable present in the
   * expression and returns the number of entries in filled in variables.
   * For instance getVariables('x+y+2*w/cos(4)') would result in
   * variables = « xyw »  and would return 3. If the final number of
   * variables would overflow the maxNumberOfVariables, getVariables return -1 */
  static constexpr int k_maxNumberOfVariables = 6;
  int getVariables(ExpressionNode::isVariableTest isVariable, char * variables) const { return this->node()->getVariables(isVariable, variables); }
  /* getLinearCoefficients return false if the expression is not linear with
   * the variables hold in 'variables'. Otherwise, it fills 'coefficients' with
   * the coefficients of the variables hold in 'variables' (following the same
   * order) and 'constant' with the constant of the expression. */
  bool getLinearCoefficients(char * variables, Expression coefficients[], Expression constant[], Context & context, Preferences::AngleUnit angleUnit) const;
  /* getPolynomialCoefficients fills the table coefficients with the expressions
   * of the first 3 polynomial coefficients and return polynomialDegree.
   * coefficients has up to 3 entries. It supposed to be called on Reduced
   * expression. */
  static constexpr int k_maxPolynomialDegree = 2;
  static constexpr int k_maxNumberOfPolynomialCoefficients = k_maxPolynomialDegree+1;
  int getPolynomialReducedCoefficients(char symbolName, Expression coefficients[], Context & context, Preferences::AngleUnit angleUnit) const;

  /* Comparison */
  /* isIdenticalTo is the "easy" equality, it returns true if both trees have
   * same structures and all their nodes have same types and values (ie,
   * sqrt(pi^2) is NOT identical to pi). */
  bool isIdenticalTo(const Expression e) const {
    /* We use the simplification order only because it is a already-coded total
     * order on expresssions. */
    return ExpressionNode::SimplificationOrder(this->node(), e.node(), true) == 0;
  }
  bool isEqualToItsApproximationLayout(Expression approximation, int bufferSize, Preferences::AngleUnit angleUnit, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits, Context & context);

  /* Layout Helper */
  LayoutRef createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const { return node()->createLayout(floatDisplayMode, numberOfSignificantDigits); }
  int serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode = Preferences::PrintFloatMode::Decimal, int numberOfSignificantDigits = PrintFloat::k_numberOfStoredSignificantDigits) const { return this->node()->serialize(buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits); }

  /* Simplification */
  static Expression ParseAndSimplify(const char * text, Context & context, Preferences::AngleUnit angleUnit);
  Expression simplify(Context & context, Preferences::AngleUnit angleUnit) const;

  /* Approximation Helper */
  template<typename U> static U epsilon();
  template<typename U> Expression approximate(Context& context, Preferences::AngleUnit angleUnit, Preferences::ComplexFormat complexFormat) const;
  template<typename U> U approximateToScalar(Context& context, Preferences::AngleUnit angleUnit) const;
  template<typename U> static U approximateToScalar(const char * text, Context& context, Preferences::AngleUnit angleUnit);
  template<typename U> U approximateWithValueForSymbol(char symbol, U x, Context & context, Preferences::AngleUnit angleUnit) const;
  /* Expression roots/extrema solver*/
  struct Coordinate2D {
    double abscissa;
    double value;
  };
  Coordinate2D nextMinimum(char symbol, double start, double step, double max, Context & context, Preferences::AngleUnit angleUnit) const;
  Coordinate2D nextMaximum(char symbol, double start, double step, double max, Context & context, Preferences::AngleUnit angleUnit) const;
  double nextRoot(char symbol, double start, double step, double max, Context & context, Preferences::AngleUnit angleUnit) const;
  Coordinate2D nextIntersection(char symbol, double start, double step, double max, Context & context, Preferences::AngleUnit angleUnit, const Expression expression) const;

protected:
  Expression(const ExpressionNode * n) : TreeByReference(n) {}

  template<class T> explicit operator T() const {

    /* This operator allows static_casts from Expression to derived Expressions.
     * The asserts ensure that the Expression can only be casted to another
     * Expression, but this does not prevent Expression types mismatches (cast
     * Float to Symbol for instance).
     *
     * The explicit keyword prevents implicit casts. This solves the following
     * problem:
     * Expression e;
     * Symbol s = static_cast<Symbol>(e);
     * e could be casted in const char * to use the Symbol(const char *)
     * constructor, or e could be casted directly to S: it is ambiguous.
     *
     * static_cast operator copy the Expression. To avoid copy, we call
     * static_cast<Symbol &>(e) instead of static_cast<Symbol>(e). */

    assert(T::isExpression());
    static_assert(sizeof(T) == sizeof(Expression), "Size mismatch");
    return *reinterpret_cast<T *>(const_cast<Expression *>(this));
  }

  /* Reference */
  ExpressionNode * node() const {
    assert(TreeByReference::node() == nullptr || !TreeByReference::node()->isGhost());
    return static_cast<ExpressionNode *>(TreeByReference::node());
  }

  /* Hierarchy */
  Expression parent() const {
    return Expression(static_cast<ExpressionNode *>(TreeByReference::parent().node()));
  }
  Expression childAtIndex(int i) const {
    return Expression(static_cast<ExpressionNode *>(TreeByReference::childAtIndex(i).node()));
  }
  void setChildrenInPlace(Expression other) { node()->setChildrenInPlace(other); }
  Expression(int nodeIdentifier) : TreeByReference(nodeIdentifier) {}
  void defaultSetChildrenInPlace(Expression other);

  /* Properties */
  int getPolynomialCoefficients(char symbolName, Expression coefficients[]) const { return node()->getPolynomialCoefficients(symbolName, coefficients); }

  /* Simplification */
  Expression denominator(Context & context, Preferences::AngleUnit angleUnit) const { return node()->denominator(context, angleUnit); }
  Expression shallowReduce(Context & context, Preferences::AngleUnit angleUnit) { return node()->shallowReduce(context, angleUnit); }
  Expression shallowBeautify(Context & context, Preferences::AngleUnit angleUnit) { return node()->shallowBeautify(context, angleUnit); }
  Expression deepBeautify(Context & context, Preferences::AngleUnit angleUnit);
  Expression deepReduce(Context & context, Preferences::AngleUnit angleUnit);
  Expression setSign(ExpressionNode::Sign s, Context & context, Preferences::AngleUnit angleUnit) { return node()->setSign(s, context, angleUnit); }
private:
  /* Simplification */
  Expression defaultShallowReduce(Context & context, Preferences::AngleUnit angleUnit);
  Expression defaultShallowBeautify(Context & context, Preferences::AngleUnit angleUnit);

  /* Properties */
  Expression defaultReplaceSymbolWithExpression(char symbol, Expression expression) const;
  int defaultGetPolynomialCoefficients(char symbol, Expression expression[]) const;

  /* Expression roots/extrema solver*/
  constexpr static double k_solverPrecision = 1.0E-5;
  constexpr static double k_sqrtEps = 1.4901161193847656E-8; // sqrt(DBL_EPSILON)
  constexpr static double k_goldenRatio = 0.381966011250105151795413165634361882279690820194237137864; // (3-sqrt(5))/2
  constexpr static double k_maxFloat = 1e100;
  typedef double (*EvaluationAtAbscissa)(char symbol, double abscissa, Context & context, Preferences::AngleUnit angleUnit, const Expression expression0, const Expression expression1);
  Coordinate2D nextMinimumOfExpression(char symbol, double start, double step, double max, EvaluationAtAbscissa evaluation, Context & context, Preferences::AngleUnit angleUnit, const Expression expression = Expression(), bool lookForRootMinimum = false) const;
  void bracketMinimum(char symbol, double start, double step, double max, double result[3], EvaluationAtAbscissa evaluation, Context & context, Preferences::AngleUnit angleUnit, const Expression expression = Expression()) const;
  Coordinate2D brentMinimum(char symbol, double ax, double bx, EvaluationAtAbscissa evaluation, Context & context, Preferences::AngleUnit angleUnit, const Expression expression = Expression()) const;
  double nextIntersectionWithExpression(char symbol, double start, double step, double max, EvaluationAtAbscissa evaluation, Context & context, Preferences::AngleUnit angleUnit, const Expression expression) const;
  void bracketRoot(char symbol, double start, double step, double max, double result[2], EvaluationAtAbscissa evaluation, Context & context, Preferences::AngleUnit angleUnit, const Expression expression) const;
  double brentRoot(char symbol, double ax, double bx, double precision, EvaluationAtAbscissa evaluation, Context & context, Preferences::AngleUnit angleUnit, const Expression expression) const;
};

}

#endif
