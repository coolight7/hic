#define _NameTagConcat_d(A, B) _NameTagConcat_1_d(A, B)
#define _NameTagConcat_1_d(A, B) A##B
#define _MacroArgToTag_d(...)                                                                      \
  _MacroArgToTag(__VA_ARGS__, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, 1)
#define _MacroArgToTag(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,  \
                       _17, _18, _19, TARGET, ...)                                                 \
  TARGET

#define _MacroArgSize_d(...)                                                                       \
  _MacroArgSize_1_d(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3,  \
                    2, 1)
#define _MacroArgSize_1_d(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15,    \
                          _16, _17, _18, _19, TARGET, ...)                                         \
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
#define _GENERATE_ENUM_ITEMN(a, ...) T##a, _MacroDefer_d(_GENERATE_ENUM_ITEM)()(__VA_ARGS__)

#define GENERATE_STRING_ITEM_d(...)                                                                \
  _NameTagConcat_d(_GENERATE_STRING_ITEM, _MacroArgToTag_d(__VA_ARGS__))(__VA_ARGS__)
#define _GENERATE_STRING_ITEM() GENERATE_STRING_ITEM_d
#define _GENERATE_STRING_ITEM1(a) #a
#define _GENERATE_STRING_ITEMN(a, ...) #a, _MacroDefer_d(_GENERATE_STRING_ITEM)()(__VA_ARGS__)

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
