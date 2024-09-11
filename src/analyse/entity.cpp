#include "entity.h"

std::shared_ptr<SyntaxNode_value_define_c> SyntaxNode_function_call_c::returnType() const {
  return symbol->type->returnType();
}
