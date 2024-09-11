#include "entity.h"

size_t SymbolItem_value_c::size() {
  Assert_d(hasType() == true, "变量类型不能为空");
  return type->size();
}

bool SymbolItem_value_c::checkValueType(const std::shared_ptr<SyntaxNode_value_define_c>& in_type) {
  if (nullptr == in_type) {
    UtilLog(Terror, "{} 变量待检查类型不应为空", name);
    return false;
  }
  return SyntaxNode_value_define_c::compare(type, in_type);
  return checkValueType(*in_type);
}

// 检查变量类型
// - 对象自身为已定义符号
// - [in_type] 为引用符号类型需求
bool SymbolItem_value_c::checkValueType(const SyntaxNode_value_define_c& in_type) {
  if (type->value_type->token != in_type.value_type->token) {
    UtilLog(Terror, "{} 变量类型不同: token, 预期：{}, 但得到：{}", name,
            WordEnumToken_c::toName(type->value_type->token),
            WordEnumToken_c::toName(in_type.value_type->token));
    return false;
  }
  if (type->pointer != in_type.pointer) {
    // 检查指针层级
    // 不用检查引用 type->isReferer != in_type.isReferer
    UtilLog(Terror, "{} 变量类型不同: 指针层级, 预期：{} 层, 但得到：{} 层", name, type->pointer,
            in_type.pointer);
    return false;
  }
  switch (type->value_type->token) {
  case WordEnumToken_e::Tid: {
    // 自定义类型
    // 类型名称
    if (type->value_type->toId().id != in_type.value_type->toId().id) {
      UtilLog(Terror, "{} 变量类型不同: ID, 预期：{}, 但得到：{}", name,
              type->value_type->toId().id, in_type.value_type->toId().id);
      return false;
    }
  } break;
  case WordEnumToken_e::Ttype: {
    // 内置类型
    if (type->value_type->toType().value != in_type.value_type->toType().value) {
      UtilLog(Terror, "{} 变量类型不同: type, 预期：{}, 但得到：{}", name,
              WordEnumType_c::toName(type->value_type->toType().value),
              WordEnumType_c::toName(in_type.value_type->toType().value));
      return false;
    }
  } break;
  default: {
    Assert_d(true == false, "{} 非法的变量类型: {}", name,
             WordEnumToken_c::toName(type->value_type->token));
    return false;
  }
  }
  return true;
}

// TODO: 检查传入参数是否匹配
bool SymbolItem_function_c::checkFunArgs(std::list<std::shared_ptr<ListNode_c>>& args) {
  if (args.size() != type->args.size()) {
    UtilLog(Terror, "函数参数个数不匹配: {} (预期 {} 个，但给定了 {} 个)", name, type->args.size(),
            args.size());
    return false;
  }
  return true;
}