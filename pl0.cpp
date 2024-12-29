#define _CRT_SECURE_NO_WARNINGS

#include "pl0.h"

const char pl0_t::reserved_word[RESERVED_WORDS_NUMBER][IDENTIFER_LENGTH_MAX] = {
    "begin",
    "call",
    "const",
    "do",
    "end",
    "if",
    "odd",
    "procedure",
    "read",
    "then",
    "var",
    "while",
    "write"
};

const char pl0_t::instruction_name[INSTRUCTION_NUMBER][4] = {
    "lit",
    "opr",
    "lod",
    "sto",
    "cal",
    "int",
    "jmp",
    "jpc"
};

const char *pl0_t::error_msg[100] = {
    /* 0 */     "编译成功",
    /* 1 */     "未知的错误",
    /* 2 */     "PL/0 源程序不完整或缺少\".\"",
    /* 3 */     "数字串太长，最长为14个数字字符",
    /* 4 */     "数值太大，最大值为2047",
    /* 5 */     "目标代码表溢出",
    /* 6 */     "太长的标识符",
    /* 7 */     "未找到标识符",
    /* 8 */     "未找到\"=\"",
    /* 9 */     "未找到常量值，常量值必须是一个非负整数",
    /* 10 */    "未定义的标识符",
    /* 11 */    "不可以在此处使用过程标识符",
    /* 12 */    "此处未找到过程标识符",
    /* 13 */    "未找到\")\"",
    /* 14 */    "未找到关系运算符",
    /* 15 */    "此处必须是一个变量标识符",
    /* 16 */    "未找到\":=\"",
    /* 17 */    "未找到\"(\"",
    /* 18 */    "未找到\"then\"",
    /* 19 */    "未找到\"do\"",
    /* 20 */    "未找到\"end\"",
    /* 21 */    "太多的过程嵌套",
    /* 22 */    "未找到\";\"",
    /* 23 */    "标识符重复定义",
    /* 24 */    "符号表溢出",
    /* 25 */    "[运行时间错]运行栈溢出",
    /* 26 */    "[运行时间错]除0错",
    /* 27 */    "未找到表达式",
    /* 28 */    "无效的表达式",
    /* 29 */    "[运行时间错]未知的指令",
    /* 30 */    "过程说明的后面应该是过程说明符或语句开始符",
    /* 31 */    "应该是语句开始符",
    /* 32 */    "语句后的符号不正确",
    /* 33 */    "因子的开始符不正确",
    /* 34 */    "因子后有非法符号",
    /* 35 */    "分析过程失败",
    /* 36 */    "分析常量声明失败",
    /* 37 */    "分析过程失败",
    /* 38 */    "常量或变量声明后应该是逗号或分号",
    /* 39 */    "过程后面的符号不正确，主过程应该以句号结尾，非主过程应该以分号结尾",
    /* 40 */    "条件后面的符号不正确",
    /* 41 */    "表达式后面的符号不正确",
    /* 42 */    "表达式中的项后面的符号不正确",
    /* 43 */    ""
};


pl0_t::pl0_t(char *_f)
{
    f = _f;

    // 先预读一个字符到 ch 中
    ch = f[0];
    if( ch == '\0' )
    {
        fp = 0;
        line= 0;
    }
    else
    {
        fp = 1;
        line = 1;
    }

    num = 0;
    sym = sym_nul;
    ident[0] = '\0';

    cx = 0;
    tx = 0;

    // 分析语法单位 program
    //error.set(0, 0);
    succ = program();
}

void pl0_t::show_code()
{
    for (int i = 0; i < cx; i++)
    {
        printf("%d\t%s\t%d\t%d\n",
            i,
            instruction_name[code[i].instruction],
            code[i].level,
            code[i].address);
    }
}

void pl0_t::show_error()
{
    if (succ) return;
    printf("[Line:%d] Error %d : %s", error.line, error.error_num, error_msg[error.error_num]);
}

bool pl0_t::interpret(int *errorno, int *errorplace)
{
    // 目标代码执行前的初始化
    int s[STACKSIZE] = { 0, 0, 0 }; // 运行栈,s[0..2] 为用于第 0 层的 block ( 相当于 PASCAL 的 program ) 的三个联系单元  
    int t = 0;                      // 栈顶指针 , 实际指向栈顶的下一个单元 ( 空闲单元 )
    int p = 0;                      // 指令指针 , 总是指向 code 中正在执行的指令的下一条指令 , 模拟 ip 寄存器
    int b = 0;                      // 当前过程的栈基址
    int i = 0;                      // 当前指令指针 i 初始化为 0 ，i 总是指向当前正在执行的指令
    instruction_t I;                // 当前指令 , 模拟指令寄存器

    while (1)
    {
        I = code[p];                // I 得到当前即将要被执行的指令
        i = p;                      // i 指向这个将要被执行的指令
        p++;                        // p 指向下一条要执行的指令

        switch (I.instruction)      // 分析指令
        {
            case lit:               // lit 0 a 将常量值 a 压入栈顶
                s[t] = I.address;   // 将指令中 Address 字段的值压入到栈顶
                t++;                // 栈顶指针 t 加 1
                if (!test_t(t, errorno, errorplace, i))
                    return false;
                break;

            case lod:               // lod l, a 将地址为 (l,a) 的变量的值压入栈顶
            {
                int _b = base(I.level, s, b);   // 相对于上述外层过程栈基址偏移为 I.Address 的地址为 _a
                int _a = _b + I.address;
                s[t] = s[_a];                   // 将目标地址所指的栈单元的值压入到栈顶 ，栈顶指针加 1
                t++;
                if ( !test_t(t, errorno, errorplace, i) )
                    return false;
                break;
            }
            
            case sto:               // sto l,a 将栈顶的值弹出并存储到 地址为 (l,a) 的变量中 , 此指令与 lod 为逆操作
            {
                t--;                // 将栈顶单元的值弹出 ，栈顶指针减 1
                if (!test_t(t, errorno, errorplace, i))
                    return false;
                int _b = base(I.level, s, b);   // 当前过程的外 I.Level 层过程的栈基址为 _b
                int _a = _b + I.address;        // 相对于上述外层过程栈基址偏移为 I.Address 的地址为 _a
                s[_a] = s[t];                   // 将刚才从栈顶弹出的值赋值到此目标地址所指的栈单元中
                break;
            }

            case cal:               // cal l,a 调用入口地址为 (l,a) 的过程
                // 设当前过程为 C , C 调用 B ( 见课本 p29D2.10 )
                // C 的层次为 2 ,但是需要注意到 C 的过程体的层次数为 3 ,
                // B 的层次数为 1
                // 所以在 C 中调用 B 会生成 "cal 2 B.entry" 指令
                // 现在应该使得 ( 被调用的 ) B 的静态链 SL 为当前过程 C 的嵌套外 2 层 ( 在例中 C 的嵌套外 2 层为 A , B 直接被 A 嵌套 )的栈基址

                // 为被调用的过程在栈顶依次压入静态链指针 、动态链指针和返回地址 ，即三个联系单元
                s[t] = base(I.level, s, b); // 设置被调用的过程的活动单元中的静态链为当前过程的外 i.Level 层过程的栈基址 , 原因见上面的说明
                s[t + 1] = b;               // 设置被调用的过程的活动单元中的动态链为当前过程的栈基址
                s[t + 2] = p;               // 设置被调用的过程的活动单元中的返回地址为当前指令指针 p , 此时 p 指向 cal 的下一条指令

                b = t;                      // 被调用过程的栈基址 b 为当前栈顶指针 t
                p = I.address;              // 跳转到被调用的过程的入口地址
                break;
            
            case inte:              // int 0 a , 其中 a 为要分配的栈单元个数
                t = t + I.address;  // 在栈顶分配由 int 指令的 Address 字段指定个数的单元
                if (!test_t(t, errorno, errorplace, i))
                    return false;
                break;
        
            case jmp:               // 当前指令为 jmp
                p = I.address;      // 将运行指针 p 设置为由 jmp 指令的 Address 字段指定的值
                break;

            case jpc:
                t--;
                if (!test_t(t, errorno, errorplace, i))
                    return false;
                if ( s[t] == 0 )    // 若栈顶单元的值为假 ( 0 表示假 ，非 0 表示真 ) , 则跳转 , 否则顺序执行 ( 运行指针 p 不修改 ，使得程序顺序执行 )
                {
                    p = I.address;
                }
                break;

        
            case opr:
                switch (I.address)
                {
                    case 0: // return : 当前指令 opr 0 0 的功能为过程返回
                        t = b; // 用以释放将要结束的过程的栈空间
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        p = s[t + 2]; // 用以设置返回地址
                        b = s[t + 1]; // 用以设置返回后到达的过程的栈基址
                        break;
                    case 1: // 单目减 : 当前指令 opr lev 1 的功能为单目减
                        s[t - 1] = -s[t - 1];   // 将栈顶单元进行取反操作
                        break;
                    case 2: // + : 当前指令 opr lev 2 的功能为加法
                        // 将次栈顶单元 ( 左操作数 ) 与栈顶单元 ( 右操作数 ) 进行加法运算 ，运算结果置于次栈顶 ，然后栈顶指针减 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        s[t - 1] = s[t - 1] + s[t]; 
                        break;
                    case 3: // - : 当前指令 opr lev 3 的功能为减法
                        // 将次栈顶单元与栈顶单元进行减法运算 ，运算结果置于次栈顶，然后栈顶指针减 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        s[t - 1] = s[t - 1] - s[t];
                        break;
                    case 4: // * : 当前指令 opr lev 4 的功能为乘法
                        // 将次栈顶单元与栈顶单元进行乘法运算 ，运算结果置于次栈顶 ，然后栈顶指针减 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        s[t - 1] = s[t - 1] * s[t];
                        break;
                    case 5: // / : 当前指令 opr lev 5 的功能为除法
                        // 将次栈顶单元与栈顶单元进行除法运算 ，运算结果置于次栈顶 ，然后栈顶指针减 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        if (s[t] == 0)
                        {
                            *errorno = 26;  // 在当前指令中发现除数为 0 的错误
                            *errorplace = i;
                            return false;
                        }
                        s[t - 1] = s[t - 1] / s[t];
                        break;
                    case 6: // odd : 当前指令 opr lev 6 的功能为 odd
                        // 计算栈顶单元除以 2 的余数，运算结果仍置于栈顶
                        s[t - 1] = s[t - 1] % 2;
                        break;
                    case 8: // = : 当前指令 opr lev 8 的功能为关系运算 =
                        // 将次栈顶单元与栈顶单元进行 = 比较运算运算 ，运算结果置于次栈顶 ，然后栈顶指针减 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        s[t - 1] = (s[t - 1] == s[t]);
                        break;
                    case 9: // # : 当前指令 opr lev 9 的功能为关系运算 #
                        // 将次栈顶单元与栈顶单元进行 # 比较运算运算 ，运算结果置于次栈顶 ，然后栈顶指针减 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        s[t - 1] = (s[t - 1] != s[t]);
                        break;
                    case 10: // < : 当前指令 opr lev 10 的功能为关系运算 <
                        // 将次栈顶单元与栈顶单元进行 < 比较运算运算 ，运算结果置于次栈顶 ，然后栈顶指针减 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        s[t - 1] = (s[t - 1] < s[t]);
                        break;
                    case 11: // >= : 当前指令 opr lev 11 的功能为关系运算 >=
                        // 将次栈顶单元与栈顶单元进行>=比较运算运算 ，运算结果置于次栈顶，然后栈顶指针减 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        s[t - 1] = (s[t - 1] >= s[t]);
                        break;
                    case 12: // > : 当前指令 opr lev 12 的功能为关系运算 >
                        // 将次栈顶单元与栈顶单元进行 > 比较运算运算 ，运算结果置于次栈顶，然后栈顶指针减 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        s[t - 1] = (s[t - 1] > s[t]);
                        break;
                    case 13: // <= : 当前指令 opr lev 13 的功能为关系运算 <=
                        // 将次栈顶单元与栈顶单元进行 <= 比较运算运算 ，运算结果置于次栈顶 ，然后栈顶指针减 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        s[t - 1] = (s[t - 1] <= s[t]);
                        break;
                    case 14: // write : 当前指令 opr lev 14 的功能为 write
                        // 将栈顶单元的值打印输出 ，然后栈顶指针减 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        printf("%d", s[t]);
                        break;
                    case 15: // writeln : 当前指令 opr lev 15 的功能为 writeln
                        printf("\n");
                        break;
                    case 16: // read : 当前指令 opr lev 16 的功能为 read
                        // 输入一个数，压入到栈顶
                        scanf("%d", &s[t]);
                        t++;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        break;
                    default: // unknown function in opr instruction
                        *errorno = 29;
                        *errorplace = i;
                        return false;
                } // switch
                break;
            
            default: // unknown instruction
                *errorno = 29;
                *errorplace = i;
                return false;
        } // switch

        // 当主过程执行 "opr 0 0" 时 , 程序结束
        if (p == 0)
            break;
    };

    printf("\nPress any key\n");
    getchar();
    return true;
}


//=====================================================================================================

/* 通过过程基址 b 求出上 l 层过程的基址 */
/* b 为当前过程的栈基址
 s[b+0] 中为当前过程的静态链指针,指向嵌套当前过程的直接外层过程的栈基址
 s[b+1] 中为当前过程的动态链指针,指向调用当前过程的的过程的栈基址
 s[b+2] 中为当前过程的返回地址
 */
int pl0_t::base(int l, int *s, int b) {
    int b1 = b;
    while (l > 0) {
        b1 = s[b1];
        l--;
    }
    return b1;
}

bool pl0_t::gen(instruction_set_e instruction, int level, int address)
{
    if (cx >= CODE_ARRAY_SIZE_MAX)
    {
        error.set(5, line); // 目标代码过长
        return false; // 不可以再继续编译
    }

    // 填表
    code[cx].instruction = instruction;
    code[cx].level = level;
    code[cx].address = address;

    // 目标代码表表尾指针cx加1，成为 "+IntToStr(cx)+" ，指向下一可用空闲表项
    cx++;

    return true;
}

bool pl0_t::enter(ident_e k, int lev, int *ptx, int *pdx)
{
    (*ptx)++; // 符号表表尾指针 tx 指向下一行（指向一个空行）
    if (*ptx >= TABLE_LENGTH_MAX)
    {
        error.set(24, line); // 符号表空间不足
        return false;
    }

    // 开始填写符号表
    strcpy(sym_table[*ptx].name, ident);
    sym_table[*ptx].kind = k;
    sym_table[*ptx].level = lev; // 这是该标识所在的过程的层次数，需要强调说明的是：若一个过程标识符的层次数为n，则其中的各标识符的层次数为n+1）

    switch (k)
    {
        case id_const:
            sym_table[*ptx].value = num;
            // address 字段对于常量标识符无意义
            // size 字段对于常量标识符无意义
            break;
        case id_var:
            sym_table[*ptx].address = *pdx; // 这是该变量获得的内存空间的地址，此地址为该变量相对于该变量所在的过程在栈中的基地址的偏移地址）
            (*pdx)++; // 运行栈指针指向下一个可分配的栈单元
            // value 字段对于变量标识符无意义
            // size 字段对于变量标识符无意义
            break;
        case id_procedure:
            // value字段对于变量标识符无意义
            // address 字段的值应该是过程在目标代码表中的执行入口地址，但现在无法确定，有待于将来扫描到此过程的语句部分时回填
            // size 字段的值应该是过程所需的包括3个联系单元在内的栈空间大小，其大小为3+变量个数，但现在无法确定此字段的值，有待于将来扫描到此过程的语句部分时回填
            break;
    }

    // 填写符号表结束
    return true;
}

/* 从table[tx]到table[1]查找名字为id的标识符，
 若找到(>=1)，返回此标识符在table中的位置
 若未找到，返回0 */
int pl0_t::position(const char *id, int tx)
{
    int i = tx;
    for (; i >= 1; i--)
    {
        if( strcmp(sym_table[i].name, id) == 0)
            break;
    }
    return i;
}

/* 从table[tx]到table[1]查找名字为id且层次数为lev的标识符，
 若找到(>=1)，返回此标识符在table中的位置
 若未找到，返回0 */
bool pl0_t::redeclaration(const char *id, int tx, int lev)
{
    int i = tx;
    for (; i >= 1; i--)
    {
        if( strcmp( sym_table[i].name, id) == 0 && sym_table[i].level == lev )
            return true;
    }
    return false;
}

bool pl0_t::test_t(int _t, int *errorno, int *errorplace, int i)
{
    if (_t >= STACKSIZE || _t < 0)
    {
        *errorno = 25;
        *errorplace = i;
        return false;
    }
    return true;
}

//=====================================================================================================

/*
 从源程序中读取一个字符到全程变量 ch，
 getch 执行后：
 （1）全程变量 ch 得到当前的输入字符，其在 init() 中曾被初始成空格字符
 （2）流指针 fp 和 (line,col) 指向流中 ch 的下一个字符
 每次调用 getch 读取一个字符，会使得全程变量 col 加 1，例外的情况是字表符字符，
 一个制表符通常表示 n 个空格字符，n 的值现在无法确定，其值取决于编辑器,
 每次成功地执行了 getch() 后,全程变量 ch 为取得的字符，同时流的指针指向下一个字符
 getch 实际总是返回 true，若调用后 ch 为 '\0'，表明已经读到文件结束 。
 */
bool pl0_t::getch()
{
    ch = f[fp];
    if (ch != '\0')
    {
        fp++;
        if (ch == '\n')
        {
            line++;
        }
    }
    else;
    return true;
}

/*
 从源程序中读取一个词(符号)
 若读到的是保留字，全程变量word中为保留字字符串，sym为相应的符号类型，见TSymbol的定义
 若读到的是标识符，全程变量word中为标识符字符串，sym为sym_ident
 若读到的是数字，全程变量num为数字的值，sym为sym_number
 若读到的是":="，sym为sym_becomes
 若读到":",但其后不是"=",sym为sym_nul
 ...
 每次成功地取得一个单词后:
 (1)全程变量sym的得到单词的类型
 (2)全程变量得到单词的字符串
 (3)若取得的单词是数字，全程变量num得到数字的值
 _getsym返回true，而sym!=sym_nul，表明成功地取到一个符号
 _getsym返回true，而sym==sym_nul，表明读到一个不可识别的符号
 _getsym返回false，此时sym必定为sym_nul，表明文件已经结束
 */
bool pl0_t::getsym()
{
    sym = sym_nul;

    if (ch == '\0')
    {
        return false;
    }

    // 滤除各种空格，若在滤除各种空格过程中，读到文件结尾，失败返回
    // 滤除各种空格成功后，ch得到当前非空格字符
    // 且fp和(line,col)指向ch的下一个字符
    // 且start和(line0,col0)指向ch
    while (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r') // 忽略空格,换行和TAB等space字符
    {
        getch();
        if (ch == '\0')
            return false;
    }
    // while结束后,ch为读到的第一个非空格字符
    // fp指向ch的下一个字符
    // (line,col)指向ch的下一个字符

    if (ch >= 'a' && ch <= 'z') // 可能是保留字或标识符
    {
        char tmp[IDENTIFER_LENGTH_MAX + 1] = {0};

        for (int k = 0; ; k++)
        {
            if (k < IDENTIFER_LENGTH_MAX)
            {
                tmp[k] = ch;
            }
            else
            {
                // 发现编译错误（过长的符号串）,但编译程序并不结束，而是继续读，直到读到一个非字符数字字符或遇到 EOF
                // 跳过若干字符后，跳出外层循环
                error.set(6, line); // 过长的符号串
                return false;
            }

            getch();
            if (ch == '\0')
                break;
            else if ((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9'));
            else
                break;
        }

        strcpy(ident, tmp);

        // 确定符号的类型
        for (int k = 0; k < RESERVED_WORDS_NUMBER; k++ )
        {
            if( strcmp(ident, reserved_word[k]) == 0) /* word是否是第k个保留字 */
            {
                sym = reserved_word_type[k]; /* word是第k个保留字 */
                return true; /* 结束for搜索 */
            }
        }

        if( sym == sym_nul )
            sym = sym_ident; /* word是标识符 */
    }

    else if (ch >= '0' && ch <= '9')
    {
        int k = 0;
        num = 0;
        sym = sym_number;

        // 确定数字串的值
        for (int k = 0, sym = sym_nul; ; k++)
        {
            num = num * 10 + ch - '0';  // 此时ch为数字串的第k个字符
            if (num > NUMBER_VALUE_MAX)
            {
                error.set(4, line); // 过大的数字,数字不能超过 2047
                return false;
            }

            if (k > NUMBER_LENGTH_MAX)
            {
                error.set(3, line); // 过长的数字串,数字串的长度不能超过14个字符
                return false;
            }

            getch();
            if (ch == '\0')
                break;
            else if( ch >= '0' && ch <= '9' );
            else
                break;
        }
    }

    else if (ch == ':')
    {
        getch();
        if (ch == '=')
        {
            sym = sym_becomes;
            getch();
        }
        else
        {
            sym = sym_nul; /* 不可识别的符号，错误处理交给调用_getch的某外层函数处理 */
        }
    }

    else if (ch == '<')
    {
        getch();
        if (ch == '=')
        {
            sym = sym_leq;
            getch();

        }
        else
        {
            sym = sym_lss;
        }
    }

    else if (ch == '>')
{
        getch();
        if (ch == '=')
        {
            sym = sym_geq;
            getch();

        }
        else
        {
            sym = sym_gtr;
        }
    }

    else
    {
        switch (ch)
        {
            case '+':
                sym = sym_plus;
                break;
            case '-':
                sym = sym_minus;
                break;
            case '*':
                sym = sym_times;
                break;
            case '/':
                sym = sym_slash;
                break;
            case '=':
                sym = sym_eql;
                break;
            case '#':
                sym = sym_neq;
                break;
            case '(':
                sym = sym_lparen;
                break;
            case ')':
                sym = sym_rparen;
                break;
            case ',':
                sym = sym_comma;
                break;
            case ';':
                sym = sym_semicolon;
                break;
            case '.':
                sym = sym_period;
                break;
            default:
                sym = sym_nul;
                break;
        }

        getch();
    }

    return true;
}

//=====================================================================================================

bool pl0_t::program()
{
    // 先预读一个符号
    if (!getsym()) // sym=sym_nul表明已经读到文件尾
    {
        error.set(2, line); // PL/0 源程序不完整或缺少\".\"
        return false;
    }

    // 分析语法单位 block
    if( !block(0, 0) )
    {
        error.set(35, line); // 分析过程失败
        return false;
    }

    // 用以结束主过程的句号
    if( sym != sym_period )
    {
        error.set(2, line); // 未在程序中找到"."
        return false;
    }

    return true;
}

/*
 过程 (block) 处理
 lev 当前处理的过程所在的层 ,PL/0 主程序为第 0 层
 main 在首次调用 block时 , 指定的 lev 就是0
 tx  tx 为符号表 (table)的表尾指针
 main 在首次调用 block 时,指定的 tx 为 0,这意味着从 table[0] 开始登记标识符
 */
bool pl0_t::block(int lev, int tx)
{
    // 开始分析名字为 ProcedureName 的过程
    // 此过程的层次数 lev
    // 此过程中定义的标识符将从 tx 处开始填写（tx 指向符号表表尾，填表时，先将 tx 加 1，然后填至 table[tx]）处")
    //std::string ProcedureName = lev == 0 ? "program" : sym_table[tx].name;
    
    // 检查当前 block 的层次数是否合法，最大允许的层次数为 3
    if (lev > BLOCK_NESTING_DEPTH_MAX)
    {
        error.set(21, line); // 过多的过程嵌套
        return false; // 过多的过程嵌套
    }

    /* 本层的block中的变量被分配到的相对地址,0,1,2三个用于作为联系单元
       |            |
       |    ...     |
       +------------+
     3 |            | <-- dx = 3
       +------------+
     2 |  联系单元   |
       +------------+
     1 |  联系单元   |
       +------------+
     0 |  联系单元   | <-- 本层在栈中的基址
       +------------+
       |    ...     |
       |            |
       +------------+
     */

    // 在每个过程的分析开始时，将 dx 初始为 3，每个 block 的栈的前 3 个栈单元（0 ~ 2）有特殊用途：用于作为 3 个联系单元
    int dx = 3;

    // 暂存该过程在符号表中的入口地址到临时变量 tx0 中
    // 做这个工作的目的是为了将来在分析到该过程语句入口处时，能够回填此时 table[tx] 中的 Address 和 Size 字段，
    // Address 字段的值应该是该过程的语句部分在 code 中的起始地址
    // Size 字段的值应该是此过程所需的栈空间大小，大小为 3 加上此过程中定义的变量个数
    // 此时 table[0] 较为特殊，可以理解为其记载的是主过程，其相当于 PASCAL 中的 program
    int tx0 = tx;

    // 保存下面的 "jmp 0 x" 指令的地址 x ( x 应为 block 的语句部分的入口地址 ) , 用于将来回填 jmp 指令的地址字段
    // 每当编译器扫描到一个过程的开始处时，首先生成一个 jmp 0 x 指令
    // 其中 x 为此过程的语句部分的入口地址，显然此时 x 的值是未知的，
    // 需要等到将来编译程序扫描到此 block 的语句部分时，才可能知道 x 的值，那时将 x 的值设置（回填）为语句部分的入口地址 。为了便于将来回填x,现在需要保存指令 \" jmp 0 x \" 的地址，
    // 下面马上要产生的这个 jmp 指令的在 code 中的地址为现在的 cx 的值
    // jmp 0 x \" 指令总是一个过程要产生的第一条指令,这个指令的地址就是过程的入口地址,过程的入口地址会保存到该过程在符号表中的 Address 字段中
    int cx0 = cx;
    
    // 产生的 jmp 0 x 指令
    if (!gen(jmp, 0, -1))
        return false;

    while(1)
    {
        if( sym == sym_const )
        {
            // 开始过程 ProcedureName 的常量声明部分
            getsym();
            if( !const_declaration(lev, &tx, &dx) )
                return false;

            while( sym == sym_comma )
            {
                getsym();
                if( !const_declaration(lev, &tx, &dx) )
                    return false;
            }

            // 检查常量声明结尾处的分号
            if (sym != sym_semicolon)
            {
                error.set(22, line); // 常量定义部分结尾所需的分号未找到
                return false;
            }
            else
            {
                getsym();
            }
        }

        if( sym == sym_var)
        {
            // 开始过程 ProcedureName 的常量声明部分
            getsym();
            if( !var_declaration(lev, &tx, &dx) )
                return false;

            while( sym == sym_comma )
            {
                getsym();
                if( !var_declaration(lev, &tx, &dx) )
                    return false;
            }

            // 检查变量声明结尾处的分号
            if( sym != sym_semicolon )
            {
                error.set(22, line); // 变量定义部分结尾所需的分号未找到
                return false;
            }
            else
            {
                getsym();
            }
        }

        while( sym == sym_procedure)
        {
            // 开始过程 ProcedureName 的过程声明部分
            getsym(); // sym 得到下一个符号

            // 检查 ident 是否为标识符以及是否被重复定义
            if( sym != sym_ident ) // sym 是否是标识符
            {
                error.set(7, line); // 缺少标识符
                return false;
            }

            if( redeclaration(ident, tx, lev) ) // 此标识符是否在本过程中已经被定义（为常量标识符、变量标识符或过程标识符)
            {
                error.set(23, line); // 标识符重复定义错误
                return false; // 标识符重复定义错误
            }
            
            // 将过程标识符登记到符号表中
            if( !enter(id_procedure, lev, &tx, &dx) )
                return false;

            // 检查过程声明中过程标识符后面的分号
            getsym();
            if( sym != sym_semicolon )
            {
                error.set(22, line); // 未找到过程定义首部结束所需的分号
                return false;
            }

            // 分析语法单位 block
            getsym();
            if( !block(lev + 1, tx) )
                return false;

            // 检查过 block 后面的分号
            if( sym != sym_semicolon )
            {
                error.set(22, line); // 未找到定义过程结束所需的分号
                return false;
            }

            // 预读一个符号到sym中
            getsym();
        } // end while(sym == sym_procedure)

        if (sym == sym_const || sym == sym_var || sym == sym_procedure)
            continue;
        else
            break;
    };

    // 开始分析过程 ProcedureName 的语句部分的准备工作
    //（1) 回填目标代码表（code）：在开始扫描这个过程的时候，曾经生成了一条 jmp 0 ? 指令，这条指令的地址曾经保存到变量 cx0 中，就是为了便于现在回填。现在扫描到这个过程的语句部分，可以确定这个指令的地址的值了，应该设置这个地址值为这个过程的语句部分在目标代码表中的入口地址，也就是当前的 cx 的值
    // (2) 回填符号表（table）：在开始扫描这个过程的时候，曾经在符号表中登记了这个过程的过程标识符名，但是其中的地址和大小字段尚未确定，现在是确定并回填它们的时候了。注意到曾经将要回填的表项的指针保存到变量 tx0 中
    // (2.1) table[tx0].Address = cx ，如（1）所述 ，当前的cx的值就是此过程的执行入口地址，一个过程的执行入口地址就是这个过程的语句部分的入口地址
    // (2.2) table[tx0].Size = dx ，当前 dx 的值就是此过程所需的栈空间大小 ，其值总是 3 加上此过程中定义的变量个数，由于现在已经扫描到此过程的语句部分，变量定义部分已经结束，所以可以确定这个值了
    // (3) 产生一条指令 \"int 0 size\" ，每当进入到一个过程的语句部分就会产生这条指令，用于为这个过程分配栈空间，如(2.2)所述，指令中的 size 就是当前的 dx 的值，这条指令在目标代码表中的地址就是当前的 cx 的值

    // 回填 code[cx0] 中的 Address 字段，将其设置为当前的 cx 的值，因为此时 cx 为此过程的语句部分的入口地址，此时 cx 是此过程的入口地址
    // 回填位于 cx0 的 jmp 0 x 指令中的 x
    code[cx0].address = cx;

    // 回填 ：table[tx0].Address = cx 和 table[tx0].Size = dx
    // 回填位于 tx0 的用于描述本 block 的符号表表项的地址字段和大小字段
    sym_table[tx0].address = cx;
    sym_table[tx0].size = dx;

    // 语句部分需要生成的第一条指令是 int , 用于为 block 分配栈空间 ,分配的大小为 dx ,同时保存 int 指令的地址到 cx0
    if( !gen(inte, 0, dx) )
        return false;

    // 分析语法单位 statement
    if ( !statement(lev, tx) )
        return false;

    // 在每个过程的末尾会生成一条 opr 0 0 ，用于过程返回
    if( !gen(opr, 0, 0) )
        return false;

    return true;
}

bool pl0_t::const_declaration(int lev, int *ptx, int *pdx)
{
    // 分析常量声明 ident

    // 检查 ident 是否为标识符以及是否被重复定义
    if( sym != sym_ident )
    {
        error.set(7, line); // 常量声明中缺少常量标识符
        return false;
    }

    // if(此常量标识符在本过程中已经被定义为常量标识符、变量标识符或过程标识符)
    if (redeclaration(ident, *ptx, lev))
    {
        error.set(22, line); // 标识符重复定义
        return false;
    }

    // 检查常量声明中的 =
    getsym();
    if (sym != sym_eql)
    {
        error.set(8, line); // 缺少 =
        return false;
    }

    // 检查常量声明中的常数值
    getsym();
    if( sym != sym_number )
    {
        error.set(9, line); // 缺少常量值
        return false;
    }

    // 将常量标识符登记到符号表中
    if( !enter(id_const, lev, ptx, pdx) )
        return false;
    
    getsym();
    return true;
}

bool pl0_t::var_declaration(int lev, int *ptx, int *pdx)
{
    // 分析变量声明 ident

    // 检查 ident 是否为标识符以及是否被重复定义
    if( sym != sym_ident )
    {
        error.set(7, line); // 缺少标识符
        return false;
    }

    if (redeclaration(ident, *ptx, lev))
    {
        error.set(23, line); // 标识符重复定义
        return false;
    }
    
    // 将变量标识符登记到符号表中
    if ( !enter(id_var, lev, ptx, pdx) )
        return false;

    getsym();
    return true;
}

bool pl0_t::statement(int lev, int tx)
{
    if (sym == sym_ident) // 赋值语句
    {
        // 检查赋值语句的左边的标识符是否是一个已定义的标识符，并且是一个变量标识符
        int i = position(ident, tx);
        if( i == 0 )
        {
            error.set(10, line); // 未定义的标识符
            return false;
        }
        
        if( sym_table[i].kind != id_var)
        {
            error.set(15, line); // 赋值语句的左边是一个已定义的标识符 ，但不是一个变量标识符
            return false;
        }

        // 检查赋值语句中的赋值符
        getsym();
        if( sym != sym_becomes )
        {
            error.set(16, line); // 缺少 :=
            return false;
        }

        // 分析语法单位 expression
        getsym();
        if( !expression(lev, tx) )
            return false;

        // 产生赋值语句 sto 指令，将栈顶的值（表达式的值位于栈顶）复制到赋值语句左边的变量中，完成赋值
        if (!gen(sto, lev - sym_table[i].level, sym_table[i].address))
            return false;
    }

    else if( sym == sym_read ) // read 语句
    {
        // 检查 read 语句中的左括号
        getsym();
        if( sym != sym_lparen )
        {
            error.set(17, line); // 缺少左括号
            return false;
        }

        while(1)
        {
            getsym();

            // 检查 ident 是否为已定义的变量标识符
            if( sym != sym_ident )
            {
                error.set(7, line); // 缺少标识符
                return false;
            }
                
            // 是否是已经定义的标识符
            int i = position(ident, tx);
            if( i == 0 )
            {
                error.set(10, line); // 未定义的标识符
                return false;
            }
            
            // 是否是变量标识符
            if (sym_table[i].kind != id_var)
            {
                error.set(15, line); // 应该是变量标识符
                return false;
            }

            // 每个变量的 read 会产生两条指令
            // ● opr, 0, 16
            // ● sto, lev-table[i].Level, table[i].Address
            if( !gen(opr, 0, 16) )
                return false;
            if( !gen(sto, lev - sym_table[i].level, sym_table[i].address))
                return false;

            // 检查当前符号是否为逗号
            // 如果是逗号 ，则继续输入
            // 如果不是逗号，则表明输入变量列表结束
            getsym();
            if (sym == sym_comma)
                continue;
            else
                break;
        };

        // 检查 read 语句中的右括号
        if( sym != sym_rparen )
        {
            error.set(13, line); // 缺少右括号
            return false; //
        }
        else
        {
            getsym();
        }
    }

    else if( sym == sym_write ) // 此语句为 write 语句
    {
        // 检查 write 语句中的左括号
        getsym();
        if( sym != sym_lparen )
        {
            error.set(17, line); // 缺少左括号
            return false;
        }

        while(1)
        {
            // 分析语法单位 expression
            getsym();
            if( !expression(lev, tx) )
                return false;

            // 产生 write 语句的目标代码 opr, 0, 14 ，用于将位于栈顶的表达式的值打印输出
            if( !gen(opr, 0, 14) )
                return false;

            // 检查当前符号是否为逗号
            // 如果是逗号 ，则继续输出
            // 如果不是逗号，则表明输出表达式列表结束
            if (sym == sym_comma)
                continue;
            else
                break;
        };

        // 检查 write 语句中的右括号
        if( sym != sym_rparen )
        {
            error.set(13, line); // 缺少右括号
            return false; // 缺少)
        }

        // 在产生 write 语句中各个表达式的值的打印指令后，产生一个 opr 0 15 指令 ，其作用是打印一个回车换行符
        if( !gen(opr, 0, 15) )
            return false;

        getsym();
    }

    else if (sym == sym_call)   // 此语句为 call 语句
    {
        getsym();

        // 检查 ident 是否为已定义的过程标识符
        if (sym != sym_ident)   // 是否是标识符
        {
            error.set(17, line); // 缺少标识符
            return false;
        }

        int i = position( ident, tx );
        if (i == 0) // 是否是已经定义的标识符
        {
            error.set(10, line); // call 语句中的标识符未定义
            return false;
        }

        // 是否是过程标识符
        if (sym_table[i].kind != id_procedure)
        {
            error.set(12, line); // 应该是过程标识符
            return false;
        }

        // 产生 cal 指令
        if( !gen(cal, lev - sym_table[i].level, sym_table[i].address) )
            return false;
            
        getsym();
    }

    else if( sym == sym_if ) // 此语句为 if 语句
    {
        // 分析语法单位 condition
        getsym();
        if( !condition(lev, tx) )
            return false;

        // 检查 if 语句中的 then
        if (sym != sym_then)
        {
            error.set(18, line); // 缺少 then
            return false;
        }

        // 产生 jpc 指令
        // 并将此指令的地址暂存到 cx1 中 ，便于在分许完 if 语句 中的 statement 后 ，回填此指令中的地址字段
        int cx1 = cx;
        if( !gen(jpc, 0, -1) )
            return false;

        // 分析语法单位 statement
        getsym();
        if( !statement(lev, tx) )
            return false;

        // 回填在 statement 之前生成的 jpc 指令中的地址字段
        // 准备回填 jpc 指令中的地址字段，应该回填为当前的 cx 的值
        code[cx1].address = cx;
    }

    else if( sym == sym_while )  // 此语句为 while 语句
    {
        // 暂存 while 语句中 condition 的入口地址到 cx1
        int cx1 = cx;

        // 分许语法单位 condition
        getsym();
        if( !condition(lev, tx) )
            return false;

        // 产生 jpc 指令
        // 并将此指令的地址暂存到 cx2 中 ，便于在分许完 while 语句 中的 statement 后 ，回填此指令中的地址字段
        int cx2 = cx;
        if( !gen(jpc, 0, -1) )
            return false;

        // 检查 while 语句中的 do
        if (sym != sym_do)
        {
            error.set(19, line); // 缺少 do
            return false;
        }

        // 分许语法单位 statement
        getsym();
        if( !statement(lev, tx) )
            return false;

        // 产生 jmp, 0, cx1 指令
        // 其中 cx1 是 while 语句中的 condition 的入口地址
        if( !gen(jmp, 0, cx1) )
            return false;

        // 回填在 statement 之前生成的 jpc 指令中的地址字段
        code[cx2].address = cx;
    }

    else if (sym == sym_begin) // 复合语句
    {
        // 分许语法单位 statement
        getsym();
        if (!statement(lev, tx))
            return false;

        while (sym == sym_semicolon)
        {
            // 分许语法单位 statement
            getsym();
            if (!statement(lev, tx))
                return false;
        }

        // 检查复合语句中的 end
        if (sym != sym_end)
        {
            error.set(20, line); // 缺少 end
            return false;
        }

        getsym();
    }

    else    // 空语句
    {
    }

    return true;
}

bool pl0_t::condition(int lev, int tx)
{
    if( sym == sym_odd ) // 检查到 odd 运算符
    {
        // 分许语法单位 expression
        getsym();
        if( !expression(lev, tx) )
            return false;

        // 产生 opr, 0, 6 指令 ，对栈顶的值（表达式的值位于栈顶）进行奇偶判断，并将判断的结构放到栈顶
        if( !gen(opr, 0, 6) )
            return false;
    }
    else
    {
        // 分析语法单位 expression
        if( !expression(lev, tx) )
            return false;

        // 检查 condition 中的关系运算符
        if (sym != sym_eql && 
            sym != sym_neq && 
            sym != sym_lss && 
            sym != sym_leq && 
            sym != sym_gtr && 
            sym != sym_geq )
        {
            error.set(14, line); // 缺少关系运算符
            return false;
        }

        sym_e relop = sym;

        // 分析语法单位 expression
        getsym();
        if( !expression(lev, tx) )
            return false;        

        // 产生 opr, 0, n 指令 ，其中 n 表示关系运算符的类型：
        // ● n = 8  : sym_eql
        // ● n = 9  : sym_neq
        // ● n = 10 : sym_lss
        // ● n = 11 : sym_geq
        // ● n = 12 : sym_gtr
        // ● n = 13 : sym_leq
        switch( relop )
        {
            case sym_eql:
                if (!gen(opr, 0, 8))
                    return false;
                break;
            case sym_neq:
                if (!gen(opr, 0, 9))
                    return false;
                break;
            case sym_lss:
                if (!gen(opr, 0, 10))
                    return false;
                break;
            case sym_geq:
                if (!gen(opr, 0, 11))
                    return false;
                break;
            case sym_gtr:
                if (!gen(opr, 0, 12))
                    return false;
                break;
            case sym_leq:
                if (!gen(opr, 0, 13))
                    return false;
                break;
            default:
                break;
        }
    }

    return true;
}

bool pl0_t::expression(int lev, int tx)
{
    if (sym == sym_plus || sym == sym_minus)    // 检查到 + 或 - 单目运算符
    {
        // 将这个单目运算符暂存到 add_minus_op 中
        sym_e add_minus_op = sym;

        // 分析语法单位 term
        getsym();
        if( !term(lev, tx) )
            return false;

        if( add_minus_op == sym_minus)
        {
            // 如果 add_minus_op 为 sym_minus ，则产生 opr, 0, 1 指令
            // 如果 add_minus_op 为 sym_add ，则不必产生指令
            if (!gen(opr, 0, 1))
                return false;
        }
    }
    else
    {
        // 分析语法单位 term
        if( !term(lev, tx) )
            return false;
    }

    while( sym == sym_plus || sym == sym_minus )
    {
        // 检查到 + 或 - 双目运算符
        // 将这个双目运算符暂存到 add_minus_op 中
        sym_e add_minus_op = sym;

        // 分析语法单位 term
        getsym();
        if( !term(lev, tx) )
            return false;

        // 产生 opr, 0, n 指令 ，其中 n 表示双目算术运算符的类型
        // ● n = 2  : sym_plus
        // ● n = 3  : sym_minus
        if (add_minus_op == sym_plus)
        {
            if (!gen(opr, 0, 2))
                return false;
        }
        else
        {
            if (!gen(opr, 0, 3))
                return false;
        }
    }

    return true;
}

bool pl0_t::term(int lev, int tx)
{
    // 分析语法单位 factor
    if( !factor(lev, tx) )
        return false;

    while( sym == sym_times || sym == sym_slash )
    {
        // 检查到 * 或 / 双目运算符
        // 将这个双目运算符暂存到 times_slash_op 中
        sym_e times_slash_op = sym;

        // 分析语法单位 factor
        getsym();
        if( !factor(lev, tx) )
            return false;

        // 产生 opr, 0, n 指令 ，其中 n 表示双目算术运算符的类型
        // ● n = 4  : sym_times
        // ● n = 5  : sym_slash
        if( times_slash_op == sym_times)
        {
            if (!gen(opr, 0, 4))
                return false;
        }
        else // if (times_slash_op == sym_slash)
        {
            if (!gen(opr, 0, 5))
                return false;
        }
    }

    return true;
}

bool pl0_t::factor(int lev, int tx)
{
    if (sym == sym_ident)
    {
        // factor 的开始符是一个标识符
        // 检查这个标识符是否定义和类型
        int i = position(ident, tx);
        if( i == 0 )
        {
            error.set(10, line); // 未定义的标识符
            return false;
        }

        if( sym_table[i].kind == id_const )
        {
            // 产生 lit, 0, table[i].Value 指令
            if( !gen(lit, 0, sym_table[i].value) )
                return false;
        }
        else if( sym_table[i].kind == id_var )
        {
            // 产生 lod, lev-table[i].Level, table[i].Address 指令
            if( !gen(lod, lev - sym_table[i].level, sym_table[i].address) )
                return false;
        }
        else if (sym_table[i].kind == id_procedure )
        {
            error.set(11, line); // 不应该是过程标识符
            return false;
        }

        getsym();
    }

    else if (sym == sym_number)
    {
        // factor 的开始符是一个数字
        // 产生 产生 lit, 0, table[i].Value 指令
        if( !gen(lit, 0, num) )
            return false;

        getsym();
    }

    else if ( sym == sym_lparen )
    {
        // factor 的开始符是一个左圆括号

        // 分析语法单位 expression
        getsym();
        if ( !expression(lev, tx) )
            return false;

        // 检查 factor 中的右圆括号
        if (sym != sym_rparen)
        {
            error.set(13, line); // 缺少 )
            return false;
        }

        getsym();
    }

    return true;
}


