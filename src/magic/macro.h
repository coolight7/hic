/**
 * ## 用于定义生成 `enum` 的定义和对应的字符串名称，以及相应的转换函数
 * - 不需要 `#pragma once`，否则可能导致编译异常
 * - 用法详见 `GENERATE_ENUM`
 */
#define _NameTagConcat_d(A, B) _NameTagConcat_1_d(A, B)
#define _NameTagConcat_1_d(A, B) A##B
#define _MacroArgToTag_d(...)                                                                      \
  _MacroArgToTag(__VA_ARGS__, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, \
                 N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N,  \
                 N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N,  \
                 N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, 1)
#define _MacroArgToTag(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,  \
                       _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31,  \
                       _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46,  \
                       _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61,  \
                       _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76,  \
                       _77, _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91,  \
                       _92, _93, _94, _95, _96, _97, _98, _99, TARGET, ...)                        \
  TARGET

/**
 * ## 获得可变参数的个数
 * - 100, ##__VA_ARGS__ 可在无参数时移除 100 之后的第一个,逗号，使得 _MacroArgSize_d() 正常返回 0
 */
#define _MacroArgSize_d(...)                                                                       \
  _MacroArgSize_1_d(100, ##__VA_ARGS__, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86,    \
                    85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67,    \
                    66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,    \
                    47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29,    \
                    28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, \
                    8, 7, 6, 5, 4, 3, 2, 1, 0, 0)
#define _MacroArgSize_1_d(                                                                         \
    _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, \
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, \
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77, \
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96, \
    _97, _98, _99, TARGET, ...)                                                                    \
  TARGET

#define _MoreExpand_d(...) _MoreExpand_1_d(_MoreExpand_1_d(_MoreExpand_1_d(__VA_ARGS__)))
#define _MoreExpand_1_d(...) _MoreExpand_2_d(_MoreExpand_2_d(_MoreExpand_2_d(__VA_ARGS__)))
#define _MoreExpand_2_d(...) _MoreExpand_3_d(_MoreExpand_3_d(_MoreExpand_3_d(__VA_ARGS__)))
#define _MoreExpand_3_d(...) _MoreExpand_4_d(_MoreExpand_4_d(_MoreExpand_4_d(__VA_ARGS__)))
#define _MoreExpand_4_d(...) _MoreExpand_5_d(_MoreExpand_5_d(_MoreExpand_5_d(__VA_ARGS__)))
#define _MoreExpand_5_d(...) __VA_ARGS__

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
 * - 枚举值由 名称拼接前置字母 `T`，防止枚举名和保留关键字重叠
 * - 放置枚举的字符串名称和相关转换函数，都是静态变量和静态函数，名称由 `name`
 * 后追加 `_c`
 */
#define GENERATE_ENUM(name, ...)                                                                   \
  enum name##_e{_MoreExpand_d(GENERATE_ENUM_ITEM_d(__VA_ARGS__))};                                 \
                                                                                                   \
  class name##_c {                                                                                 \
  public:                                                                                          \
    name##_c() = delete;                                                                           \
                                                                                                   \
    inline static const std::array<const std::string, _MacroArgSize_d(__VA_ARGS__)> namelist = {   \
        _MoreExpand_d(GENERATE_STRING_ITEM_d(__VA_ARGS__))};                                       \
                                                                                                   \
    inline static constexpr int toInt(name##_e index) { return static_cast<int>(index); }          \
                                                                                                   \
    inline static constexpr name##_e toEnum(int index) { return static_cast<name##_e>(index); }    \
                                                                                                   \
    inline static constexpr const std::string& toName(name##_e index) {                            \
      return namelist[toInt(index)];                                                               \
    }                                                                                              \
                                                                                                   \
    inline static constexpr bool existInt(int index) {                                             \
      if (index >= 0 && index < namelist.size()) {                                                 \
        return true;                                                                               \
      }                                                                                            \
      return false;                                                                                \
    }                                                                                              \
                                                                                                   \
    inline static constexpr bool existName(std::string_view in_name) {                             \
      for (const auto& item : namelist) {                                                          \
        if (item == in_name) {                                                                     \
          return true;                                                                             \
        }                                                                                          \
      }                                                                                            \
      return false;                                                                                \
    }                                                                                              \
  };
