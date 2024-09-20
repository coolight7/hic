#include "entity.h"

size_t SymbolItem_value_c::size() {
  Assert_d(hasType() == true, "变量类型不能为空");
  return type->size();
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

void Type_c::debugPrint(const size_t tab, std::function<size_t()> onOutPrefix) const {
  if (nullptr != word) {
    _PRINT_WORD_PREFIX(false);
    word->printInfo();
  }
  _PRINT_WORD_PREFIX(true);
  if (0 == pointer && false == isReferer) {
    std::cout << "[值类型]";
  } else {
    for (int i = pointer; i > 0; --i) {
      std::cout << "*";
    }
    std::cout << " ";
    if (isReferer) {
      std::cout << "&";
    }
  }
  std::cout << std::endl;
}

size_t Type_c::size() const {
  if (pointer > 0) {
    // 指针
    return ValueTypeSize_e::Spointer;
  }
  switch (type) {
  case WordEnumType_e::Tbyte: {
    return ValueTypeSize_e::Sbyte;
  } break;
  case WordEnumType_e::Tbool: {
    return ValueTypeSize_e::Sbool;
  } break;
  case WordEnumType_e::Tenum:
  case WordEnumType_e::Tint: {
    return ValueTypeSize_e::Sint;
  } break;
  case WordEnumType_e::Tint64: {
    return ValueTypeSize_e::Sint64;
  } break;
  case WordEnumType_e::Tfloat: {
    return ValueTypeSize_e::Sfloat;
  } break;
  case WordEnumType_e::Tfloat64: {
    return ValueTypeSize_e::Sfloat64;
  } break;
  case WordEnumType_e::Tfunction: {
    return ValueTypeSize_e::Spointer;
  } break;
  case WordEnumType_e::TString: {
    switch (limit) {
    case TypeLimit_e::Constexpr: {
      Assert_d(nullptr != word, "常量表达式类型的值不应为空");
      if (nullptr != word) {
        return (ValueTypeSize_e::Sbyte * word->toString().value.size());
      }
    } break;
    default:
      break;
    }
    // TODO: 修正类型大小
    return ValueTypeSize_e::Spointer;
  } break;
  case WordEnumType_e::Tclass: {
  } break;
  default:
    break;
  }
  return ValueTypeSize_e::Spointer;
}

// 检查变量类型
bool Type_c::compare(std::shared_ptr<Type_c> left, std::shared_ptr<Type_c> right) {
  if (nullptr == left || nullptr == right) {
    UtilLog(Terror, "两个变量类型比较不应为空, {}, {}", (long long)(left.get()),
            (long long)(right.get()));
    return false;
  }
  if (left->type != right->type) {
    UtilLog(Terror, "{} 变量类型不同: token, 预期：{}, 但得到：{}", left->name(),
            WordEnumType_c::toName(left->type), WordEnumType_c::toName(right->type));
    return false;
  }
  if (left->pointer != right->pointer) {
    // 检查指针层级
    // 不用检查引用 type->isReferer != in_type.isReferer
    UtilLog(Terror, "{} 变量类型不同: 指针层级, 预期：{} 层, 但得到：{} 层", left->name(),
            left->pointer, right->pointer);
    return false;
  }
  switch (left->type) {
  case WordEnumType_e::Tenum:
  case WordEnumType_e::Tclass: {
    // 自定义类型名称相同
    if (left->word->toId().id == right->word->toId().id) {
      UtilLog(Terror, "{} 变量类型不同: 预期：{}, 但得到：{}", WordEnumType_c::toName(left->type),
              left->word->toId().id, right->word->toId().id);
      return false;
    }
  } break;
  case WordEnumType_e::Tfunction: {
    // TODO: 函数类型检查
  } break;
  default:
    break;
  }
  return true;
}