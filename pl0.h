#pragma once

#include<string>

class pl0_t
{
    // ===========================================================================
    // ��ʷ������йصĶ���
    // ===========================================================================

    // ��ʶ��������ַ�����
    #define IDENTIFER_LENGTH_MAX 10

    // ���ֵ����λ����PL/0 ��֧�� 10 ������������λ�����ɳ��� 14 λ����ֵ���ɳ��� 2047������������ 0 ��ʼ
    #define NUMBER_LENGTH_MAX	14
    #define NUMBER_VALUE_MAX	2047

    // ���ʹ� 32 ������
    #define SYMBOL_TYPE_MAX 32

    // ���ʣ�symbol�������͹� 32 �֣����������
    // �����ͣ����ڳ�ʼ����
    // ��ʶ����������ʶ����������ʶ�������̱�ʶ��ͳ��Ϊ��ʶ�����ڷ��ű��л�Ǽ����еı�ʶ�������ҷ��ű����ȷ���Ǻ��ֱ�ʶ����
    // ��������֧�����ͳ�����
    // ϵͳ���ţ� 16 ����ÿ����Ϊһ�ֵ������ͣ����������+,-,*,/,=,#,<,<=,>,>=,(,),,,;,.,:=��
    // �����֣� 13 ����ÿ����Ϊһ�ֵ������ͣ����������begin,end,if,then,while,write,read,do,call,const,var,procedure,odd��
    enum  sym_e {
        sym_nul, sym_ident, sym_number, sym_plus, sym_minus, sym_times, sym_slash,
        sym_eql, sym_neq, sym_lss, sym_leq, sym_gtr, sym_geq, sym_lparen,
        sym_rparen, sym_comma, sym_semicolon, sym_period, sym_becomes, sym_begin,
        sym_end, sym_if, sym_then, sym_while, sym_write, sym_read, sym_do, sym_call,
        sym_const, sym_var, sym_procedure, sym_odd
    };  

    // �����ָ������� PL/0 ���� begin ��end ��if ��then �� 13 ��������
    #define RESERVED_WORDS_NUMBER 13

    /* �������б� */
    static const char reserved_word[RESERVED_WORDS_NUMBER][IDENTIFER_LENGTH_MAX];

    /* �����ֶ�Ӧ�ĵ������� */
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
    // ����ű��йصĶ���
    // ===========================================================================

    // ������ʶ����������ʶ�������̱�ʶ��ͳ��Ϊ��ʶ����ֻ�б�ʶ���Ż�Ǽǵ����ű���
    enum ident_e {
        id_const, id_var, id_procedure
    };

    // ���ű����ṹ
    struct sym_table_entry_t
    {
        char name[IDENTIFER_LENGTH_MAX];
        ident_e kind;
        int value;      /* ���ֶν����ڳ�����ʶ�������ڴ洢��ֵ��������ʶ����ֱֵ�Ӵ洢�ڷ��ű��� */
        int level;      /* ���ֶν����ڱ�����ʶ���͹��̱�ʶ�������ڴ洢�����ڵĿ�Ĳ��,�����Ŀ飨�������򣩵Ĳ����Ϊ0 */
                        /* ˵��������PL/0���﷨�����κο��о������г�����ʶ���Ķ��壬���ǣ��ڲ�ͬ�Ŀ��еĳ�����ʶ������ͬ������Ϊ
                            ������ʶ�������ڿ��Σ�����������û�е���ģ��ں����ʵ���У���Ϊ������ʶ�����������͵ı�ʶ��һ�������ڲ�ε����⣬
                            �ڲ�ͬ�ķֳ����п��Զ���ͬ���ĳ�����ʶ���� */
        int address;    /* ���ֶν����ڱ�����ʶ���͹��̱�ʶ����
                            ���ڱ�����ʶ�������ڴ洢����ջ���������������block��ַ����Ե�ַ
                            ���ڹ��̱�ʶ�������ڴ洢����code�е���ʼ��ַ */
        int size;       /* �����ڹ��̱�ʶ�������ڴ洢�˹�������Ĵ洢�ռ��С,�˴�СΪ�ڹ����ж���ı�����������3,
                            ÿ��������ջ����Ҫ3����Ԫ��Ϊ��ϵ��Ԫ */
    };
    
    // ���ű�
    #define TABLE_LENGTH_MAX 100    // ��ʶ���������ű������������ÿһ�еǼ�һ����ʶ��
    sym_table_entry_t sym_table[TABLE_LENGTH_MAX];
    int tx;

    // ����ǰ�� ident ( �����һ�ε��� getsym �õ� ) ��д��������
    bool enter(ident_e k, int lev, int *ptx, int *pdx);

    // �� tx �� 0 ��������Ϊ id �ı��� ����������Ϊ id �ı����ڷ��ű��е����� , ������ʧ�� �� ���� 0 
    // ע�� : ʵ���Ϸ��ű����� 0 ��������������������
    int position(const char *id, int tx);

    // ===========================================================================
    // ��Ŀ��ָ����йصĶ���
    // ===========================================================================
    /*
    ---------------------------------------------------------------------------------------------------
    ָ��   ��β�     ��ַ                             ָ���
    ---------------------------------------------------------------------------------------------------
    lit    0(����Ϊ0) ����ֵ                           ������ֵѹ�뵽ջ��
    lod    ��β�     ��Ե�ַ                         ��ջ��ֵ��������(��β�,��Ե�ַ)ָ����ջ��Ԫ
    sto    ��β�     ��Ե�ַ                         ����(��β�,��Ե�ַ)ָ����ջ��Ԫѹ�뵽ջ��
    cal    ��β�     �����õĹ�����Ŀ��������еĵ�ַ ���ù���
    int    0(����Ϊ0) ���ٵĵ�Ԫ��(=3+�ֲ���������)    Ϊ�����̻򱻵��õĹ�����ջ�п���������
    jmp    0(����Ϊ0) ת���ַ                         ������ת��ָ��
    jpc    0(����Ϊ0) ת���ַ                         ����ת��ָ��(��ջ����ֵΪ��,ת��ָ���ĵ�ַ,����˳��ִ��)
    opr    0(����Ϊ0) �����                           ˵��
    ---------------------------------------------------------------------------------
                    0(return)                        ���̵��ý����󷵻ص����õ㣬���ͷű����ù��̵�ջ�ռ�
                    1(��Ŀ��)                        ��ջ����ֵȡ��
                    2(+)                             ��ջ����ֵ���ջ����ֵ���������㣬����Ľ���ŵ���ջ��
                    3(-)
                    4(*)
                    5(/)
                    6(odd)                           ��ջ����ֵ����ż�ж�������Ϊ�棬ż��Ϊ�٣��ж��Ľ���ŵ�ջ��
                    7(��)                            null
                    8(=)                             ��ջ����ֵ���ջ����ֵ����ϵ���㣬����Ľ���ŵ���ջ��
                    9(#)
                    10(<)
                    11(>=)
                    12(>)
                    13(<=)
                    14(write)                        ��ջ����ֵ�������Ļ
                    15(writeln)                      �������������Ļ
                    16(readln)                       �Ӽ�������һ��������ջ��
    ---------------------------------------------------------------------------------------------------
    */

    // Ŀ��ָ� 8 �֣��γ�Ŀ��ָ�
    #define INSTRUCTION_NUMBER 8

    // Ŀ��ָ� int Ϊ C/C++ ������, ��д�� inte ��
    enum instruction_set_e { lit = 0, opr, lod, sto, cal, inte, jmp, jpc };

    // ÿ������Ҫ 4 �ֽ�
    static const char instruction_name[INSTRUCTION_NUMBER][4];

    struct instruction_t
    {
        instruction_set_e instruction;  // ָ��
        int level;                      // ���ò��붨���Ĳ�β�
        int address;                    // ���ֶεĺ���ȡ���� instruction

        const std::string to_string()
        {
            return std::string(instruction_name[(int)instruction]) + std::string(" ") +
                std::to_string(level) + " " +
                std::to_string(address);
        }
    };

    // Ŀ���������ĳ���
    #define CODE_ARRAY_SIZE_MAX 200
    instruction_t code[CODE_ARRAY_SIZE_MAX];

    // Code indeX , ���ڲ���Ŀ����������ָ�� ������ָ�����һ����Ч�е���һ�У����У�, ���� init �б���ʼΪ 0
    int cx;    

    // ===========================================================================
    // ���������б��йصĶ���
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
    // ����
    // ===========================================================================

    // ����ջ��С
    #define STACKSIZE 500

    // ��Ƕ�׵�������Ϊ 3 �������Ŀ�Ϊ�� 0 �� ����ˣ��������Ϊ 0 ~ 3
    // �������ʵ�������������
    #define BLOCK_NESTING_DEPTH_MAX 3

    // ===========================================================================
    // һЩȫ�̱���
    // ===========================================================================

    // ����Դ�����ı����ַ���,��'\0'����,����EOF// f��ErrorListΪcpl0������ӿ�,
    char *f;// = NULL;

    // ���������ַ�����ָ��
    int fp;// = 0;

    // ��ǰ�ַ���Դ�����ļ��е��кź��к�
    int line;// = 1;
        
    // ��ǰ�ַ��� getch �õ���ȫ�̱�����
    char ch;

    // ��ǰ���ʵ����ͣ� getsym �õ���ȫ�̱�����
    sym_e sym;

    // ��������ı�ʶ���� getsym �õ���ȫ�̱�����
    char ident[IDENTIFER_LENGTH_MAX + 1];

    // ��ǰ���ֵ�ֵ�� getsym �õ���ȫ�̱�����
    int num;

    // ===========================================================================
    // ������һЩ������ԭ��˵��
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

