/**
 * ## 用于定义生成 `enum` 的定义和对应的字符串名称，以及相应的转换函数
 * - 不需要 `#pragma once`，否则可能导致编译异常
 * - 用法详见 `GENERATE_ENUM`
 */
#define _NameTagConcat_d(A, B) _NameTagConcat_1_d(A, B)
#define _NameTagConcat_1_d(A, B) A##B
#define _MacroArgToTag_d(...)                                                                      \
  _MacroArgToTag(__VA_ARGS__, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, \
                 N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, 1)
#define _MacroArgToTag(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,  \
                       _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31,  \
                       _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46,  \
                       _47, _48, _49, TARGET, ...)                                                 \
  TARGET

#define _MacroArgSize_d(...)                                                                       \
  _MacroArgSize_1_d(__VA_ARGS__, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35,   \
                    34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,    \
                    15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define _MacroArgSize_1_d(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15,    \
                          _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29,    \
                          _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43,    \
                          _44, _45, _46, _47, _48, _49, TARGET, ...)                               \
  TARGET

#define _MoreExpand_d(...) _MoreExpand_1_d(_MoreExpand_1_d(_MoreExpand_1_d(__VA_ARGS__)))
#define _MoreExpand_1_d(...) _MoreExpand_2_d(_MoreExpand_2_d(_MoreExpand_2_d(__VA_ARGS__)))
#define _MoreExpand_2_d(...) _MoreExpand_3_d(_MoreExpand_3_d(_MoreExpand_3_d(__VA_ARGS__)))
#define _MoreExpand_3_d(...) __VA_ARGS__

#define _MacroEmpty_d()
#define _MacroDefer_d(ID) ID _MacroEmpty_d()

#define GENERATE_ENUM_ITEM_d(...)                                                                  \
  _NameTagConcat_d(_GENERATE_ENUM_ITEM, _MacroArgToTag_d(__VA_ARGS__))(__VA_ARGS__)
#define _GENERATE_ENUM_ITEM() GENERATE_ENUM_ITEM_d
#define _GENERATE_ENUM_ITEM1(a) T##a
#define _GENERATE_ENUM_ITEMN(a, ...)                                                               \
  _GENERATE_ENUM_ITEM1(a), _MacroDefer_d(_GENERATE_ENUM_ITEM)()(__VA_ARGS__)

#define GENERATE_STRING_ITEM_d(...)                                                                \
  _NameTagConcat_d(_GENERATE_STRING_ITEM, _MacroArgToTag_d(__VA_ARGS__))(__VA_ARGS__)
#define _GENERATE_STRING_ITEM() GENERATE_STRING_ITEM_d
#define _GENERATE_STRING_ITEM1(a) #a
#define _GENERATE_STRING_ITEMN(a, ...)                                                             \
  _GENERATE_STRING_ITEM1(a), _MacroDefer_d(_GENERATE_STRING_ITEM)()(__VA_ARGS__)

/**
 * ## 定义枚举和对应的字符串名称和转换函数
 * - `name`: 枚举类型名，用于生成：
 *    - `enum name_e`; 枚举定义；名称由 `name` 后追加 `_e`
 *    - `class name_c`;
 * 放置枚举的字符串名称和相关转换函数，都是静态变量和静态函数，名称由 `name`
 * 后追加 `_c`
 */
#define GENERATE_ENUM(name, ...)                                                                   \
  enum name##_e{_MoreExpand_d(GENERATE_ENUM_ITEM_d(__VA_ARGS__))};                                 \
                                                                                                   \
  class name##_c {                                                                                 \
  public:                                                                                          \
    inline static const std::array<const std::string, _MacroArgSize_d(__VA_ARGS__)> namelist = {   \
        _MoreExpand_d(GENERATE_STRING_ITEM_d(__VA_ARGS__))};                                       \
                                                                                                   \
    inline static int toInt(name##_e index) { return static_cast<int>(index); }                    \
                                                                                                   \
    inline static name##_e toEnum(int index) { return static_cast<name##_e>(index); }              \
                                                                                                   \
    inline static const std::string& toName(name##_e index) { return namelist[toInt(index)]; }     \
  };
