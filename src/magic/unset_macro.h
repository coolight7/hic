/**
 * ## 用于取消 `macro.h` 定义的宏，防止其扩散影响
 * - 不需要 `#pragma once`，否则可能导致编译异常
 */ 
#undef _NameTagConcat_d
#undef _NameTagConcat_1_d
#undef _MacroArgToTag_d
#undef _MacroArgToTag

#undef _MacroArgSize_d
#undef _MacroArgSize_1_d

#undef _MoreExpand_d
#undef _MoreExpand_1_d
#undef _MoreExpand_2_d
#undef _MoreExpand_3_d

#undef _MacroEmpty_d
#undef _MacroDefer_d

#undef GENERATE_ENUM_ITEM_d
#undef _GENERATE_ENUM_ITEM
#undef _GENERATE_ENUM_ITEM1
#undef _GENERATE_ENUM_ITEMN

#undef GENERATE_STRING_ITEM_d
#undef _GENERATE_STRING_ITEM
#undef _GENERATE_STRING_ITEM1
#undef _GENERATE_STRING_ITEMN

#undef GENERATE_ENUM
