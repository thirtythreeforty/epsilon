#include <poincare/parenthesis.h>
#include <poincare/char_layout_node.h>

extern "C" {
#include <assert.h>
#include <stdlib.h>
}

namespace Poincare {


Expression::Type Parenthesis::type() const {
  return Type::Parenthesis;
}

Expression * Parenthesis::clone() const {
  Parenthesis * o = new Parenthesis(m_operands, true);
  return o;
}

int Parenthesis::polynomialDegree(char symbolName) const {
  return operand(0)->polynomialDegree(symbolName);
}

LayoutRef Parenthesis::privateCreateLayout(PrintFloat::Mode floatDisplayMode, ComplexFormat complexFormat) const {
  return CharLayoutRef('('); //TODO
/*
  assert(floatDisplayMode != PrintFloat::Mode::Default);
  assert(complexFormat != ComplexFormat::Default);
  return LayoutEngine::createParenthesedLayout(operand(0)->createLayout(floatDisplayMode, complexFormat), false);*/
}

Expression * Parenthesis::shallowReduce(Context& context, AngleUnit angleUnit) {
  Expression * e = Expression::shallowReduce(context, angleUnit);
  if (e != this) {
    return e;
  }
  return replaceWith(editableOperand(0), true);
}

template<typename T>
Expression * Parenthesis::templatedApproximate(Context& context, AngleUnit angleUnit) const {
  return operand(0)->approximate<T>(context, angleUnit);
}

}
