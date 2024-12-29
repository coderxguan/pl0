#pragma once

#include<string>

class pl0_t
{
    // ===========================================================================
    // 与词法分析有关的定义
    // ===========================================================================

    // 标识符的最大字符个数
    #define IDENTIFER_LENGTH_MAX 10

    // 数字的最大位数，PL/0 仅支持 10 进制整数，其位数不可超过 14 位，其值不可超过 2047，数字允许以 0 开始
    #define NUMBER_LENGTH_MAX	14
    #define NUMBER_VALUE_MAX	2047

    // 单词共 32 种类型
    #define SYMBOL_TYPE_MAX 32

    // 单词（symbol）的类型共 32 种，具体包括：
    // 空类型（用于初始化）
    // 标识符（常量标识符，变量标识符，过程标识符统称为标识符，在符号表中会登记所有的标识符，查找符号表可以确定是何种标识符）
    // 常数（仅支持整型常数）
    // 系统符号（ 16 个，每个作为一种单词类型，具体包括：+,-,*,/,=,#,<,<=,>,>=,(,),,,;,.,:=）
    // 保留字（ 13 个，每个作为一种单词类型，具体包括：begin,end,if,then,while,write,read,do,call,const,var,procedure,odd）
    enum  sym_e {
        sym_nul, sym_ident, sym_number, sym_plus, sym_minus, sym_times, sym_slash,
        sym_eql, sym_neq, sym_lss, sym_leq, sym_gtr, sym_geq, sym_lparen,
        sym_rparen, sym_comma, sym_semicolon, sym_period, sym_becomes, sym_begin,
        sym_end, sym_if, sym_then, sym_while, sym_write, sym_read, sym_do, sym_call,
        sym_const, sym_var, sym_procedure, sym_odd
    };  

    // 保留字个数，在 PL/0 中有 begin 、end 、if 、then 等 13 个保留字
    #define RESERVED_WORDS_NUMBER 13

    /* 保留字列表 */
    static const char reserved_word[RESERVED_WORDS_NUMBER][IDENTIFER_LENGTH_MAX];

    /* 保留字对应的单词类型 */
    enum sym_e reserved_word_type[RESERVED_WORDS_NUMBER] = 
	{
        sym_begin, sym_call, sym_const, sym_do, sym_end, sym_if, sym_odd,
        sym_procedure, sym_read, sym_then, sym_var, sym_while, sym_write
    };

    bool is_reserved_word(char *word)
    {
        for (int i = 0; i < RESERVED_WORDS_NUMBER; i++)
        {
            if (strcmp(reserved_word[i], word) == 0)
                return true;
        }
        return false;
    }

    // ===========================================================================
    // 与符号表有关的定义
    // ===========================================================================

    // 常量标识符，变量标识符，过程标识符统称为标识符；只有标识符才会登记到符号表中
    enum ident_e {
        id_const, id_var, id_procedure
    };

    // 符号表表项结构
    struct sym_table_entry_t
    {
        char name[IDENTIFER_LENGTH_MAX];
        ident_e kind;
        int value;      /* 此字段仅用于常量标识符，用于存储其值，常量标识符的值直接存储在符号表中 */
        int level;      /* 此字段仅用于变量标识符和过程标识符，用于存储其所在的块的层次,最外层的块（即主程序）的层次数为0 */
                        /* 说明：按照PL/0的语法，在任何块中均可以有常量标识符的定义，但是，在不同的块中的常量标识符不可同名，因为
                            常量标识符不存在块层次，这种做法是没有道理的，在后面的实现中，认为常量标识符与其它类型的标识符一样，存在层次的问题，
                            在不同的分程序中可以定义同名的常量标识符。 */
        int address;    /* 此字段仅用于变量标识符和过程标识符，
                            对于变量标识符，用于存储其在栈中相对于其所属的block基址的相对地址
                            对于过程标识符，用于存储其在code中的起始地址 */
        int size;       /* 仅用于过程标识符，用于存储此过程所需的存储空间大小,此大小为在过程中定义的变量个数加上3,
                            每个过程在栈中需要3个单元作为联系单元 */
    };
    
    // 符号表
    #define TABLE_LENGTH_MAX 100    // 标识符表（即符号表）的最大行数，每一行登记一个标识符
    sym_table_entry_t sym_table[TABLE_LENGTH_MAX];
    int tx;

    // 将当前的 ident ( 由最近一次调用 getsym 得到 ) 填写到符号中
    bool enter(ident_e k, int lev, int *ptx, int *pdx);

    // 从 tx 到 0 查找名字为 id 的表项 ，返回名字为 id 的表项在符号表中的索引 , 若查找失败 ， 返回 0 
    // 注意 : 实际上符号表索引 0 总是用于无名的主过程
    int position(const char *id, int tx);

    // ===========================================================================
    // 与目标指令表有关的定义
    // ===========================================================================
    /*
    ---------------------------------------------------------------------------------------------------
    指令   层次差     地址                             指令功能
    ---------------------------------------------------------------------------------------------------
    lit    0(总是为0) 常量值                           将常量值压入到栈顶
    lod    层次差     相对地址                         将栈顶值弹出到由(层次差,相对地址)指定的栈单元
    sto    层次差     相对地址                         将由(层次差,相对地址)指定的栈单元压入到栈顶
    cal    层次差     被调用的过程在目标程序中中的地址 调用过程
    int    0(总是为0) 开辟的单元数(=3+局部变量个数)    为主过程或被调用的过程在栈中开辟数据区
    jmp    0(总是为0) 转向地址                         无条件转移指令
    jpc    0(总是为0) 转向地址                         条件转移指令(当栈顶的值为假,转到指定的地址,否则顺序执行)
    opr    0(总是为0) 运算符                           说明
    ---------------------------------------------------------------------------------
                    0(return)                        过程调用结束后返回到调用点，并释放被调用过程的栈空间
                    1(单目减)                        将栈顶的值取反
                    2(+)                             将栈顶的值与次栈顶的值做算术运算，运算的结果放到次栈顶
                    3(-)
                    4(*)
                    5(/)
                    6(odd)                           将栈顶的值作奇偶判定，奇数为真，偶数为假，判定的结果放到栈顶
                    7(无)                            null
                    8(=)                             将栈顶的值与次栈顶的值做关系运算，运算的结果放到次栈顶
                    9(#)
                    10(<)
                    11(>=)
                    12(>)
                    13(<=)
                    14(write)                        将栈顶的值输出到屏幕
                    15(writeln)                      将换行输出到屏幕
                    16(readln)                       从键盘输入一个整数到栈顶
    ---------------------------------------------------------------------------------------------------
    */

    // 目标指令共 8 种，形成目标指令集
    #define INSTRUCTION_NUMBER 8

    // 目标指令（ int 为 C/C++ 保留字, 故写成 inte ）
    enum instruction_set_e { lit = 0, opr, lod, sto, cal, inte, jmp, jpc };

    // 每个串需要 4 字节
    static const char instruction_name[INSTRUCTION_NUMBER][4];

    struct instruction_t
    {
        instruction_set_e instruction;  // 指令
        int level;                      // 引用层与定义层的层次差
        int address;                    // 此字段的含义取决于 instruction

        const std::string to_string()
        {
            return std::string(instruction_name[(int)instruction]) + std::string(" ") +
                std::to_string(level) + " " +
                std::to_string(address);
        }
    };

    // 目标代码数组的长度
    #define CODE_ARRAY_SIZE_MAX 200
    instruction_t code[CODE_ARRAY_SIZE_MAX];

    // Code indeX , 用于操作目标代码数组的指针 ，总是指向最后一个有效行的下一行（空行）, 其在 init 中被初始为 0
    int cx;    

    // ===========================================================================
    // 与编译错误列表有关的定义
    // ===========================================================================

    static const char *error_msg[100];

    struct error_t
    {
        int error_num;
        int line;

        error_t()
        {
            error_num = 0;
            line = 0;
        }
        void set(int en, int l)
        {
            if (error_num) return;
            error_num = en;
            line = l;
        }
    };

    error_t error;
    
    // ===========================================================================
    // 其它
    // ===========================================================================

    // 运行栈大小
    #define STACKSIZE 500

    // 块嵌套的最大深度为 3 ，最外层的块为第 0 层 ，因此，层次数可为 0 ~ 3
    // 这个限制实际属于语义分析
    #define BLOCK_NESTING_DEPTH_MAX 3

    // ===========================================================================
    // 一些全程变量
    // ===========================================================================

    // 代表源程序文本的字符流,以'\0'结束,代表EOF// f与ErrorList为cpl0的输入接口,
    char *f;// = NULL;

    // 操作上述字符流的指针
    int fp;// = 0;

    // 当前字符在源程序文件中的行号和列号
    int line;// = 1;
        
    // 当前字符（ getch 用到的全程变量）
    char ch;

    // 当前单词的类型（ getsym 用到的全程变量）
    sym_e sym;

    // 最近读到的标识符（ getsym 用到的全程变量）
    char ident[IDENTIFER_LENGTH_MAX + 1];

    // 当前数字的值（ getsym 用到的全程变量）
    int num;

    // ===========================================================================
    // 以下是一些函数的原型说明
    // ===========================================================================
    bool getch();
    bool getsym();
    bool gen(instruction_set_e Instruction, int Level, int Address);    
    bool redeclaration(const char *id, int tx, int lev);
    int base(int l, int *s, int b);
    bool test_t(int _t, int *errorno, int *errorplace, int i);

    bool const_declaration(int lev, int *ptx, int *pdx);
    bool var_declaration(int lev, int *ptx, int *pdx);

    bool factor(int lev, int tx);
    bool term(int lev, int tx);
    bool expression(int lev, int tx);
    bool condition(int lev, int tx);
    bool statement(int lev, int tx);
    
    bool program();
    bool block(int lev, int tx);

public:
    pl0_t(char *f);
    bool succ;
    void show_code();
    void show_error();
    bool interpret(int *errorno, int *errorplace);
};

