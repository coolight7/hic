## 文法
program = {function} | {type} | {value} 
type = {enum} | {class};
limit = const | static;
value = <new>? <{limit}> <void | bool | char | int | long> <*|&> <ID> <= <*>?ID|{字面量}>?;
function = {value} ID (({value} (ID)?)*) {
	{code}
}

字面量 = number | string | true | false
enum = enum ID { (ID (=num)?)+ };
class = class ID { 
	ID() {}	// 构造函数
	~ID() {}	// 析构函数
	operator=({value}*) {} 		// 操作符函数

	((static)? {function})*  	// 成员函数
	((static)? {value};)* 		// 成员变量
	
};

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
template<typename T>
class Temp;

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

- InitKVList: 可自定义实现接收参数列表 [a, b, c]:
```c++
void fun(InitKVList<String, int> list) {}

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
