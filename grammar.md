## 文法
- program = {function_define} | {type_define} | {value_define_init} | ~

- constexpr = {number} | {string} | true | false | nullptr
- type_define = {enum_define} | {class_define};

- value_type = void | bool | char | int | long | {ID_enum_type} | {ID_class_type} // 变量类型
- value_define = <{value_type}> <*|&>?                        // 变量声明
- value_define_id = <{value_define}> <ID>                     // 变量声明
- _value_set_right = <= <<*|&>?ID>|{constexpr}>               // 变量赋值的右半部分
- value_set = ID {_value_set_right}                       // 变量赋值
- value_define_init = {value_define_id} {_value_set_right};      // 变量声明并初始化

- function_call = {ID_function}(<<*/&>?ID_value|{constexpr}>*);

- expr = {function_call} | ~

- ctrl_break = break;
- ctrl_continue = continue;
- ctrl_return = return <{ID_value} | {constexpr}>;
- ctrl_if = <if (expr) { {code} }> <else if (expr) { {code} }>* <else { {code} }>?
- ctrl_while = while({expr_bool}) { {code} | {ctrl_break} | {ctrl_continue} }
- ctrl_for = for(<{value_define_init} | {expr}>?;<{expr_bool}>?;<{expr}>?) { {code} | {ctrl_break} | {ctrl_continue} }

- code = {value_define_init} | {value_set} | {ctrl_if} | {ctrl_while} | {ctrl_for} | {ctrl_return} | {expr}

- function_define = {value_define} ID ({value_define_id}*) {
	{code}
}

- enum_define = enum ID { (ID (=number)?,)+ };
- class_define = class ID { 
	ID() { {code} }	                    // 构造函数
	~ID() { {code} }	                // 析构函数
	operator=({value}*) { {code} } 		// 操作符函数

	((static)? {function_define})*  	// 成员函数
	((static)? <{value_define_id}|{value_define_init}>;)* 		// 成员变量
};

### First 集
- First(value_type) = {value_type} | ID
- First(value_define) = First(value_type)
- First(function_define) = First(value_define)
                         = First(value_type)
                         = {value_type} | ID
- First(enum_define) = enum
- First(class_define) = class
- First(type_define) = First(enum_define) || First(class_define) 
                     = {enum, class}
- First(value_define_init) = First(value_define)
                           = First(value_type)
                           = {value_type} | ID
- First(program) = First(function_define) || First(type_define) || First(value_define_init)

### tree
- program
    - function_define/value_define_init = {value_type} | ID
        - ID -> function_define
        - First(_value_set_right) = = -> value_define_init
    - type_define = enum | class

## 变量声明
### 普通
```c++
int i = 0;
String str = "";
```
### 类型初始化
```c++
class Temp;
Temp temp1 = Temp();
auto temp2 = Temp();
Temp* temp3 = new Temp();
```

### 模板
```c++
class Temp<T>;

Temp<int> temp1 = Temp<int>();
auto temp2 = Temp<int>();
```

### 特殊
- InitList: 可自定义实现接收参数列表 [a, b, c]:
```c++
void fun(InitList<String> list) {}

fun(["a", "b", "c"]);
```

- Array: 
```c++
Array<String> arr1 = <String>["aaa", "bbb"];
Array<String> arr2 = Array<String>["aaa", "bbb"];
```
- List: 
```c++
List<String> list1 = <String>["a", "b"];
List<String> list2 = List<String>["a", "b"];
```

- InitMap: 可自定义实现接收键值对参数:
```c++
void fun(InitMap<String, int> list) {}

fun({
    "a" : 1,
    "b" : 2,
});
```

- Map:
```c++
auto value = <String, int>{
    "abc" : 123,
    "aaa" : 77,
};

auto value2 = Map<String, int>{
    "abc" : 123,
    "aaa" : 77,
};
```
