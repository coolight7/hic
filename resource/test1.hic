int a = 10086;
int b = 0;
int c = 0xf7A0;
int d = 07650;
int* f = &d;
String str = "qiqi";

int test(int& a, int** &b) {
  return (a + b);
}

enum Hello_e {
  A1,
  B2,
  CC,
  // A1,
};

int main(char** args, int size) {
    d += b;
    c -= b;
    d /= b;
    c *= b;
    a ??= b ?? c;

    int ret = test(1, 2);
    test(3, 4);
    test(ret, 4, );  // 允许尾部多余的 ,
    // test(3,);     // 检查函数参数个数
    // test();
    // test_undef(); // 检查未定义符号
    if (a == b || (b == c && a == c)) {
        d = b;
    }

    // int ok = *c; // 检查 读址 操作数类型为指针
    char ch = 'b';
        // disable=124
    bool k = false;
    // disable = fdsa
    bool g = false;
    /*disable=uu*/
    /*ll
    disable=bbc*/

    String s = "sss";
    s = "adsf 123";
    return 0;
}