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
    /* 0 */     "����ɹ�",
    /* 1 */     "δ֪�Ĵ���",
    /* 2 */     "PL/0 Դ����������ȱ��\".\"",
    /* 3 */     "���ִ�̫�����Ϊ14�������ַ�",
    /* 4 */     "��ֵ̫�����ֵΪ2047",
    /* 5 */     "Ŀ���������",
    /* 6 */     "̫���ı�ʶ��",
    /* 7 */     "δ�ҵ���ʶ��",
    /* 8 */     "δ�ҵ�\"=\"",
    /* 9 */     "δ�ҵ�����ֵ������ֵ������һ���Ǹ�����",
    /* 10 */    "δ����ı�ʶ��",
    /* 11 */    "�������ڴ˴�ʹ�ù��̱�ʶ��",
    /* 12 */    "�˴�δ�ҵ����̱�ʶ��",
    /* 13 */    "δ�ҵ�\")\"",
    /* 14 */    "δ�ҵ���ϵ�����",
    /* 15 */    "�˴�������һ��������ʶ��",
    /* 16 */    "δ�ҵ�\":=\"",
    /* 17 */    "δ�ҵ�\"(\"",
    /* 18 */    "δ�ҵ�\"then\"",
    /* 19 */    "δ�ҵ�\"do\"",
    /* 20 */    "δ�ҵ�\"end\"",
    /* 21 */    "̫��Ĺ���Ƕ��",
    /* 22 */    "δ�ҵ�\";\"",
    /* 23 */    "��ʶ���ظ�����",
    /* 24 */    "���ű����",
    /* 25 */    "[����ʱ���]����ջ���",
    /* 26 */    "[����ʱ���]��0��",
    /* 27 */    "δ�ҵ����ʽ",
    /* 28 */    "��Ч�ı��ʽ",
    /* 29 */    "[����ʱ���]δ֪��ָ��",
    /* 30 */    "����˵���ĺ���Ӧ���ǹ���˵��������俪ʼ��",
    /* 31 */    "Ӧ������俪ʼ��",
    /* 32 */    "����ķ��Ų���ȷ",
    /* 33 */    "���ӵĿ�ʼ������ȷ",
    /* 34 */    "���Ӻ��зǷ�����",
    /* 35 */    "��������ʧ��",
    /* 36 */    "������������ʧ��",
    /* 37 */    "��������ʧ��",
    /* 38 */    "���������������Ӧ���Ƕ��Ż�ֺ�",
    /* 39 */    "���̺���ķ��Ų���ȷ��������Ӧ���Ծ�Ž�β����������Ӧ���ԷֺŽ�β",
    /* 40 */    "��������ķ��Ų���ȷ",
    /* 41 */    "���ʽ����ķ��Ų���ȷ",
    /* 42 */    "���ʽ�е������ķ��Ų���ȷ",
    /* 43 */    ""
};


pl0_t::pl0_t(char *_f)
{
    f = _f;

    // ��Ԥ��һ���ַ��� ch ��
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

    // �����﷨��λ program
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
    // Ŀ�����ִ��ǰ�ĳ�ʼ��
    int s[STACKSIZE] = { 0, 0, 0 }; // ����ջ,s[0..2] Ϊ���ڵ� 0 ��� block ( �൱�� PASCAL �� program ) ��������ϵ��Ԫ  
    int t = 0;                      // ջ��ָ�� , ʵ��ָ��ջ������һ����Ԫ ( ���е�Ԫ )
    int p = 0;                      // ָ��ָ�� , ����ָ�� code ������ִ�е�ָ�����һ��ָ�� , ģ�� ip �Ĵ���
    int b = 0;                      // ��ǰ���̵�ջ��ַ
    int i = 0;                      // ��ǰָ��ָ�� i ��ʼ��Ϊ 0 ��i ����ָ��ǰ����ִ�е�ָ��
    instruction_t I;                // ��ǰָ�� , ģ��ָ��Ĵ���

    while (1)
    {
        I = code[p];                // I �õ���ǰ����Ҫ��ִ�е�ָ��
        i = p;                      // i ָ�������Ҫ��ִ�е�ָ��
        p++;                        // p ָ����һ��Ҫִ�е�ָ��

        switch (I.instruction)      // ����ָ��
        {
            case lit:               // lit 0 a ������ֵ a ѹ��ջ��
                s[t] = I.address;   // ��ָ���� Address �ֶε�ֵѹ�뵽ջ��
                t++;                // ջ��ָ�� t �� 1
                if (!test_t(t, errorno, errorplace, i))
                    return false;
                break;

            case lod:               // lod l, a ����ַΪ (l,a) �ı�����ֵѹ��ջ��
            {
                int _b = base(I.level, s, b);   // ���������������ջ��ַƫ��Ϊ I.Address �ĵ�ַΪ _a
                int _a = _b + I.address;
                s[t] = s[_a];                   // ��Ŀ���ַ��ָ��ջ��Ԫ��ֵѹ�뵽ջ�� ��ջ��ָ��� 1
                t++;
                if ( !test_t(t, errorno, errorplace, i) )
                    return false;
                break;
            }
            
            case sto:               // sto l,a ��ջ����ֵ�������洢�� ��ַΪ (l,a) �ı����� , ��ָ���� lod Ϊ�����
            {
                t--;                // ��ջ����Ԫ��ֵ���� ��ջ��ָ��� 1
                if (!test_t(t, errorno, errorplace, i))
                    return false;
                int _b = base(I.level, s, b);   // ��ǰ���̵��� I.Level ����̵�ջ��ַΪ _b
                int _a = _b + I.address;        // ���������������ջ��ַƫ��Ϊ I.Address �ĵ�ַΪ _a
                s[_a] = s[t];                   // ���ղŴ�ջ��������ֵ��ֵ����Ŀ���ַ��ָ��ջ��Ԫ��
                break;
            }

            case cal:               // cal l,a ������ڵ�ַΪ (l,a) �Ĺ���
                // �赱ǰ����Ϊ C , C ���� B ( ���α� p29D2.10 )
                // C �Ĳ��Ϊ 2 ,������Ҫע�⵽ C �Ĺ�����Ĳ����Ϊ 3 ,
                // B �Ĳ����Ϊ 1
                // ������ C �е��� B ������ "cal 2 B.entry" ָ��
                // ����Ӧ��ʹ�� ( �����õ� ) B �ľ�̬�� SL Ϊ��ǰ���� C ��Ƕ���� 2 �� ( ������ C ��Ƕ���� 2 ��Ϊ A , B ֱ�ӱ� A Ƕ�� )��ջ��ַ

                // Ϊ�����õĹ�����ջ������ѹ�뾲̬��ָ�� ����̬��ָ��ͷ��ص�ַ ����������ϵ��Ԫ
                s[t] = base(I.level, s, b); // ���ñ����õĹ��̵Ļ��Ԫ�еľ�̬��Ϊ��ǰ���̵��� i.Level ����̵�ջ��ַ , ԭ��������˵��
                s[t + 1] = b;               // ���ñ����õĹ��̵Ļ��Ԫ�еĶ�̬��Ϊ��ǰ���̵�ջ��ַ
                s[t + 2] = p;               // ���ñ����õĹ��̵Ļ��Ԫ�еķ��ص�ַΪ��ǰָ��ָ�� p , ��ʱ p ָ�� cal ����һ��ָ��

                b = t;                      // �����ù��̵�ջ��ַ b Ϊ��ǰջ��ָ�� t
                p = I.address;              // ��ת�������õĹ��̵���ڵ�ַ
                break;
            
            case inte:              // int 0 a , ���� a ΪҪ�����ջ��Ԫ����
                t = t + I.address;  // ��ջ�������� int ָ��� Address �ֶ�ָ�������ĵ�Ԫ
                if (!test_t(t, errorno, errorplace, i))
                    return false;
                break;
        
            case jmp:               // ��ǰָ��Ϊ jmp
                p = I.address;      // ������ָ�� p ����Ϊ�� jmp ָ��� Address �ֶ�ָ����ֵ
                break;

            case jpc:
                t--;
                if (!test_t(t, errorno, errorplace, i))
                    return false;
                if ( s[t] == 0 )    // ��ջ����Ԫ��ֵΪ�� ( 0 ��ʾ�� ���� 0 ��ʾ�� ) , ����ת , ����˳��ִ�� ( ����ָ�� p ���޸� ��ʹ�ó���˳��ִ�� )
                {
                    p = I.address;
                }
                break;

        
            case opr:
                switch (I.address)
                {
                    case 0: // return : ��ǰָ�� opr 0 0 �Ĺ���Ϊ���̷���
                        t = b; // �����ͷŽ�Ҫ�����Ĺ��̵�ջ�ռ�
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        p = s[t + 2]; // �������÷��ص�ַ
                        b = s[t + 1]; // �������÷��غ󵽴�Ĺ��̵�ջ��ַ
                        break;
                    case 1: // ��Ŀ�� : ��ǰָ�� opr lev 1 �Ĺ���Ϊ��Ŀ��
                        s[t - 1] = -s[t - 1];   // ��ջ����Ԫ����ȡ������
                        break;
                    case 2: // + : ��ǰָ�� opr lev 2 �Ĺ���Ϊ�ӷ�
                        // ����ջ����Ԫ ( ������� ) ��ջ����Ԫ ( �Ҳ����� ) ���мӷ����� �����������ڴ�ջ�� ��Ȼ��ջ��ָ��� 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        s[t - 1] = s[t - 1] + s[t]; 
                        break;
                    case 3: // - : ��ǰָ�� opr lev 3 �Ĺ���Ϊ����
                        // ����ջ����Ԫ��ջ����Ԫ���м������� �����������ڴ�ջ����Ȼ��ջ��ָ��� 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        s[t - 1] = s[t - 1] - s[t];
                        break;
                    case 4: // * : ��ǰָ�� opr lev 4 �Ĺ���Ϊ�˷�
                        // ����ջ����Ԫ��ջ����Ԫ���г˷����� �����������ڴ�ջ�� ��Ȼ��ջ��ָ��� 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        s[t - 1] = s[t - 1] * s[t];
                        break;
                    case 5: // / : ��ǰָ�� opr lev 5 �Ĺ���Ϊ����
                        // ����ջ����Ԫ��ջ����Ԫ���г������� �����������ڴ�ջ�� ��Ȼ��ջ��ָ��� 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        if (s[t] == 0)
                        {
                            *errorno = 26;  // �ڵ�ǰָ���з��ֳ���Ϊ 0 �Ĵ���
                            *errorplace = i;
                            return false;
                        }
                        s[t - 1] = s[t - 1] / s[t];
                        break;
                    case 6: // odd : ��ǰָ�� opr lev 6 �Ĺ���Ϊ odd
                        // ����ջ����Ԫ���� 2 ��������������������ջ��
                        s[t - 1] = s[t - 1] % 2;
                        break;
                    case 8: // = : ��ǰָ�� opr lev 8 �Ĺ���Ϊ��ϵ���� =
                        // ����ջ����Ԫ��ջ����Ԫ���� = �Ƚ��������� �����������ڴ�ջ�� ��Ȼ��ջ��ָ��� 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        s[t - 1] = (s[t - 1] == s[t]);
                        break;
                    case 9: // # : ��ǰָ�� opr lev 9 �Ĺ���Ϊ��ϵ���� #
                        // ����ջ����Ԫ��ջ����Ԫ���� # �Ƚ��������� �����������ڴ�ջ�� ��Ȼ��ջ��ָ��� 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        s[t - 1] = (s[t - 1] != s[t]);
                        break;
                    case 10: // < : ��ǰָ�� opr lev 10 �Ĺ���Ϊ��ϵ���� <
                        // ����ջ����Ԫ��ջ����Ԫ���� < �Ƚ��������� �����������ڴ�ջ�� ��Ȼ��ջ��ָ��� 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        s[t - 1] = (s[t - 1] < s[t]);
                        break;
                    case 11: // >= : ��ǰָ�� opr lev 11 �Ĺ���Ϊ��ϵ���� >=
                        // ����ջ����Ԫ��ջ����Ԫ����>=�Ƚ��������� �����������ڴ�ջ����Ȼ��ջ��ָ��� 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        s[t - 1] = (s[t - 1] >= s[t]);
                        break;
                    case 12: // > : ��ǰָ�� opr lev 12 �Ĺ���Ϊ��ϵ���� >
                        // ����ջ����Ԫ��ջ����Ԫ���� > �Ƚ��������� �����������ڴ�ջ����Ȼ��ջ��ָ��� 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        s[t - 1] = (s[t - 1] > s[t]);
                        break;
                    case 13: // <= : ��ǰָ�� opr lev 13 �Ĺ���Ϊ��ϵ���� <=
                        // ����ջ����Ԫ��ջ����Ԫ���� <= �Ƚ��������� �����������ڴ�ջ�� ��Ȼ��ջ��ָ��� 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        s[t - 1] = (s[t - 1] <= s[t]);
                        break;
                    case 14: // write : ��ǰָ�� opr lev 14 �Ĺ���Ϊ write
                        // ��ջ����Ԫ��ֵ��ӡ��� ��Ȼ��ջ��ָ��� 1
                        t--;
                        if (!test_t(t, errorno, errorplace, i))
                            return false;
                        printf("%d", s[t]);
                        break;
                    case 15: // writeln : ��ǰָ�� opr lev 15 �Ĺ���Ϊ writeln
                        printf("\n");
                        break;
                    case 16: // read : ��ǰָ�� opr lev 16 �Ĺ���Ϊ read
                        // ����һ������ѹ�뵽ջ��
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

        // ��������ִ�� "opr 0 0" ʱ , �������
        if (p == 0)
            break;
    };

    printf("\nPress any key\n");
    getchar();
    return true;
}


//=====================================================================================================

/* ͨ�����̻�ַ b ����� l ����̵Ļ�ַ */
/* b Ϊ��ǰ���̵�ջ��ַ
 s[b+0] ��Ϊ��ǰ���̵ľ�̬��ָ��,ָ��Ƕ�׵�ǰ���̵�ֱ�������̵�ջ��ַ
 s[b+1] ��Ϊ��ǰ���̵Ķ�̬��ָ��,ָ����õ�ǰ���̵ĵĹ��̵�ջ��ַ
 s[b+2] ��Ϊ��ǰ���̵ķ��ص�ַ
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
        error.set(5, line); // Ŀ��������
        return false; // �������ټ�������
    }

    // ���
    code[cx].instruction = instruction;
    code[cx].level = level;
    code[cx].address = address;

    // Ŀ�������βָ��cx��1����Ϊ "+IntToStr(cx)+" ��ָ����һ���ÿ��б���
    cx++;

    return true;
}

bool pl0_t::enter(ident_e k, int lev, int *ptx, int *pdx)
{
    (*ptx)++; // ���ű��βָ�� tx ָ����һ�У�ָ��һ�����У�
    if (*ptx >= TABLE_LENGTH_MAX)
    {
        error.set(24, line); // ���ű�ռ䲻��
        return false;
    }

    // ��ʼ��д���ű�
    strcpy(sym_table[*ptx].name, ident);
    sym_table[*ptx].kind = k;
    sym_table[*ptx].level = lev; // ���Ǹñ�ʶ���ڵĹ��̵Ĳ��������Ҫǿ��˵�����ǣ���һ�����̱�ʶ���Ĳ����Ϊn�������еĸ���ʶ���Ĳ����Ϊn+1��

    switch (k)
    {
        case id_const:
            sym_table[*ptx].value = num;
            // address �ֶζ��ڳ�����ʶ��������
            // size �ֶζ��ڳ�����ʶ��������
            break;
        case id_var:
            sym_table[*ptx].address = *pdx; // ���Ǹñ�����õ��ڴ�ռ�ĵ�ַ���˵�ַΪ�ñ�������ڸñ������ڵĹ�����ջ�еĻ���ַ��ƫ�Ƶ�ַ��
            (*pdx)++; // ����ջָ��ָ����һ���ɷ����ջ��Ԫ
            // value �ֶζ��ڱ�����ʶ��������
            // size �ֶζ��ڱ�����ʶ��������
            break;
        case id_procedure:
            // value�ֶζ��ڱ�����ʶ��������
            // address �ֶε�ֵӦ���ǹ�����Ŀ�������е�ִ����ڵ�ַ���������޷�ȷ�����д��ڽ���ɨ�赽�˹��̵���䲿��ʱ����
            // size �ֶε�ֵӦ���ǹ�������İ���3����ϵ��Ԫ���ڵ�ջ�ռ��С�����СΪ3+�����������������޷�ȷ�����ֶε�ֵ���д��ڽ���ɨ�赽�˹��̵���䲿��ʱ����
            break;
    }

    // ��д���ű����
    return true;
}

/* ��table[tx]��table[1]��������Ϊid�ı�ʶ����
 ���ҵ�(>=1)�����ش˱�ʶ����table�е�λ��
 ��δ�ҵ�������0 */
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

/* ��table[tx]��table[1]��������Ϊid�Ҳ����Ϊlev�ı�ʶ����
 ���ҵ�(>=1)�����ش˱�ʶ����table�е�λ��
 ��δ�ҵ�������0 */
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
 ��Դ�����ж�ȡһ���ַ���ȫ�̱��� ch��
 getch ִ�к�
 ��1��ȫ�̱��� ch �õ���ǰ�������ַ������� init() ��������ʼ�ɿո��ַ�
 ��2����ָ�� fp �� (line,col) ָ������ ch ����һ���ַ�
 ÿ�ε��� getch ��ȡһ���ַ�����ʹ��ȫ�̱��� col �� 1�������������ֱ���ַ���
 һ���Ʊ��ͨ����ʾ n ���ո��ַ���n ��ֵ�����޷�ȷ������ֵȡ���ڱ༭��,
 ÿ�γɹ���ִ���� getch() ��,ȫ�̱��� ch Ϊȡ�õ��ַ���ͬʱ����ָ��ָ����һ���ַ�
 getch ʵ�����Ƿ��� true�������ú� ch Ϊ '\0'�������Ѿ������ļ����� ��
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
 ��Դ�����ж�ȡһ����(����)
 ���������Ǳ����֣�ȫ�̱���word��Ϊ�������ַ�����symΪ��Ӧ�ķ������ͣ���TSymbol�Ķ���
 ���������Ǳ�ʶ����ȫ�̱���word��Ϊ��ʶ���ַ�����symΪsym_ident
 �������������֣�ȫ�̱���numΪ���ֵ�ֵ��symΪsym_number
 ����������":="��symΪsym_becomes
 ������":",�������"=",symΪsym_nul
 ...
 ÿ�γɹ���ȡ��һ�����ʺ�:
 (1)ȫ�̱���sym�ĵõ����ʵ�����
 (2)ȫ�̱����õ����ʵ��ַ���
 (3)��ȡ�õĵ��������֣�ȫ�̱���num�õ����ֵ�ֵ
 _getsym����true����sym!=sym_nul�������ɹ���ȡ��һ������
 _getsym����true����sym==sym_nul����������һ������ʶ��ķ���
 _getsym����false����ʱsym�ض�Ϊsym_nul�������ļ��Ѿ�����
 */
bool pl0_t::getsym()
{
    sym = sym_nul;

    if (ch == '\0')
    {
        return false;
    }

    // �˳����ֿո������˳����ֿո�����У������ļ���β��ʧ�ܷ���
    // �˳����ֿո�ɹ���ch�õ���ǰ�ǿո��ַ�
    // ��fp��(line,col)ָ��ch����һ���ַ�
    // ��start��(line0,col0)ָ��ch
    while (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r') // ���Կո�,���к�TAB��space�ַ�
    {
        getch();
        if (ch == '\0')
            return false;
    }
    // while������,chΪ�����ĵ�һ���ǿո��ַ�
    // fpָ��ch����һ���ַ�
    // (line,col)ָ��ch����һ���ַ�

    if (ch >= 'a' && ch <= 'z') // �����Ǳ����ֻ��ʶ��
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
                // ���ֱ�����󣨹����ķ��Ŵ���,��������򲢲����������Ǽ�������ֱ������һ�����ַ������ַ������� EOF
                // ���������ַ����������ѭ��
                error.set(6, line); // �����ķ��Ŵ�
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

        // ȷ�����ŵ�����
        for (int k = 0; k < RESERVED_WORDS_NUMBER; k++ )
        {
            if( strcmp(ident, reserved_word[k]) == 0) /* word�Ƿ��ǵ�k�������� */
            {
                sym = reserved_word_type[k]; /* word�ǵ�k�������� */
                return true; /* ����for���� */
            }
        }

        if( sym == sym_nul )
            sym = sym_ident; /* word�Ǳ�ʶ�� */
    }

    else if (ch >= '0' && ch <= '9')
    {
        int k = 0;
        num = 0;
        sym = sym_number;

        // ȷ�����ִ���ֵ
        for (int k = 0, sym = sym_nul; ; k++)
        {
            num = num * 10 + ch - '0';  // ��ʱchΪ���ִ��ĵ�k���ַ�
            if (num > NUMBER_VALUE_MAX)
            {
                error.set(4, line); // ���������,���ֲ��ܳ��� 2047
                return false;
            }

            if (k > NUMBER_LENGTH_MAX)
            {
                error.set(3, line); // ���������ִ�,���ִ��ĳ��Ȳ��ܳ���14���ַ�
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
            sym = sym_nul; /* ����ʶ��ķ��ţ�������������_getch��ĳ��㺯������ */
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
    // ��Ԥ��һ������
    if (!getsym()) // sym=sym_nul�����Ѿ������ļ�β
    {
        error.set(2, line); // PL/0 Դ����������ȱ��\".\"
        return false;
    }

    // �����﷨��λ block
    if( !block(0, 0) )
    {
        error.set(35, line); // ��������ʧ��
        return false;
    }

    // ���Խ��������̵ľ��
    if( sym != sym_period )
    {
        error.set(2, line); // δ�ڳ������ҵ�"."
        return false;
    }

    return true;
}

/*
 ���� (block) ����
 lev ��ǰ����Ĺ������ڵĲ� ,PL/0 ������Ϊ�� 0 ��
 main ���״ε��� blockʱ , ָ���� lev ����0
 tx  tx Ϊ���ű� (table)�ı�βָ��
 main ���״ε��� block ʱ,ָ���� tx Ϊ 0,����ζ�Ŵ� table[0] ��ʼ�ǼǱ�ʶ��
 */
bool pl0_t::block(int lev, int tx)
{
    // ��ʼ��������Ϊ ProcedureName �Ĺ���
    // �˹��̵Ĳ���� lev
    // �˹����ж���ı�ʶ������ tx ����ʼ��д��tx ָ����ű��β�����ʱ���Ƚ� tx �� 1��Ȼ������ table[tx]����")
    //std::string ProcedureName = lev == 0 ? "program" : sym_table[tx].name;
    
    // ��鵱ǰ block �Ĳ�����Ƿ�Ϸ����������Ĳ����Ϊ 3
    if (lev > BLOCK_NESTING_DEPTH_MAX)
    {
        error.set(21, line); // ����Ĺ���Ƕ��
        return false; // ����Ĺ���Ƕ��
    }

    /* �����block�еı��������䵽����Ե�ַ,0,1,2����������Ϊ��ϵ��Ԫ
       |            |
       |    ...     |
       +------------+
     3 |            | <-- dx = 3
       +------------+
     2 |  ��ϵ��Ԫ   |
       +------------+
     1 |  ��ϵ��Ԫ   |
       +------------+
     0 |  ��ϵ��Ԫ   | <-- ������ջ�еĻ�ַ
       +------------+
       |    ...     |
       |            |
       +------------+
     */

    // ��ÿ�����̵ķ�����ʼʱ���� dx ��ʼΪ 3��ÿ�� block ��ջ��ǰ 3 ��ջ��Ԫ��0 ~ 2����������;��������Ϊ 3 ����ϵ��Ԫ
    int dx = 3;

    // �ݴ�ù����ڷ��ű��е���ڵ�ַ����ʱ���� tx0 ��
    // �����������Ŀ����Ϊ�˽����ڷ������ù��������ڴ�ʱ���ܹ������ʱ table[tx] �е� Address �� Size �ֶΣ�
    // Address �ֶε�ֵӦ���Ǹù��̵���䲿���� code �е���ʼ��ַ
    // Size �ֶε�ֵӦ���Ǵ˹��������ջ�ռ��С����СΪ 3 ���ϴ˹����ж���ı�������
    // ��ʱ table[0] ��Ϊ���⣬�������Ϊ����ص��������̣����൱�� PASCAL �е� program
    int tx0 = tx;

    // ��������� "jmp 0 x" ָ��ĵ�ַ x ( x ӦΪ block ����䲿�ֵ���ڵ�ַ ) , ���ڽ������� jmp ָ��ĵ�ַ�ֶ�
    // ÿ��������ɨ�赽һ�����̵Ŀ�ʼ��ʱ����������һ�� jmp 0 x ָ��
    // ���� x Ϊ�˹��̵���䲿�ֵ���ڵ�ַ����Ȼ��ʱ x ��ֵ��δ֪�ģ�
    // ��Ҫ�ȵ������������ɨ�赽�� block ����䲿��ʱ���ſ���֪�� x ��ֵ����ʱ�� x ��ֵ���ã����Ϊ��䲿�ֵ���ڵ�ַ ��Ϊ�˱��ڽ�������x,������Ҫ����ָ�� \" jmp 0 x \" �ĵ�ַ��
    // ��������Ҫ��������� jmp ָ����� code �еĵ�ַΪ���ڵ� cx ��ֵ
    // jmp 0 x \" ָ������һ������Ҫ�����ĵ�һ��ָ��,���ָ��ĵ�ַ���ǹ��̵���ڵ�ַ,���̵���ڵ�ַ�ᱣ�浽�ù����ڷ��ű��е� Address �ֶ���
    int cx0 = cx;
    
    // ������ jmp 0 x ָ��
    if (!gen(jmp, 0, -1))
        return false;

    while(1)
    {
        if( sym == sym_const )
        {
            // ��ʼ���� ProcedureName �ĳ�����������
            getsym();
            if( !const_declaration(lev, &tx, &dx) )
                return false;

            while( sym == sym_comma )
            {
                getsym();
                if( !const_declaration(lev, &tx, &dx) )
                    return false;
            }

            // ��鳣��������β���ķֺ�
            if (sym != sym_semicolon)
            {
                error.set(22, line); // �������岿�ֽ�β����ķֺ�δ�ҵ�
                return false;
            }
            else
            {
                getsym();
            }
        }

        if( sym == sym_var)
        {
            // ��ʼ���� ProcedureName �ĳ�����������
            getsym();
            if( !var_declaration(lev, &tx, &dx) )
                return false;

            while( sym == sym_comma )
            {
                getsym();
                if( !var_declaration(lev, &tx, &dx) )
                    return false;
            }

            // ������������β���ķֺ�
            if( sym != sym_semicolon )
            {
                error.set(22, line); // �������岿�ֽ�β����ķֺ�δ�ҵ�
                return false;
            }
            else
            {
                getsym();
            }
        }

        while( sym == sym_procedure)
        {
            // ��ʼ���� ProcedureName �Ĺ�����������
            getsym(); // sym �õ���һ������

            // ��� ident �Ƿ�Ϊ��ʶ���Լ��Ƿ��ظ�����
            if( sym != sym_ident ) // sym �Ƿ��Ǳ�ʶ��
            {
                error.set(7, line); // ȱ�ٱ�ʶ��
                return false;
            }

            if( redeclaration(ident, tx, lev) ) // �˱�ʶ���Ƿ��ڱ��������Ѿ������壨Ϊ������ʶ����������ʶ������̱�ʶ��)
            {
                error.set(23, line); // ��ʶ���ظ��������
                return false; // ��ʶ���ظ��������
            }
            
            // �����̱�ʶ���Ǽǵ����ű���
            if( !enter(id_procedure, lev, &tx, &dx) )
                return false;

            // �����������й��̱�ʶ������ķֺ�
            getsym();
            if( sym != sym_semicolon )
            {
                error.set(22, line); // δ�ҵ����̶����ײ���������ķֺ�
                return false;
            }

            // �����﷨��λ block
            getsym();
            if( !block(lev + 1, tx) )
                return false;

            // ���� block ����ķֺ�
            if( sym != sym_semicolon )
            {
                error.set(22, line); // δ�ҵ�������̽�������ķֺ�
                return false;
            }

            // Ԥ��һ�����ŵ�sym��
            getsym();
        } // end while(sym == sym_procedure)

        if (sym == sym_const || sym == sym_var || sym == sym_procedure)
            continue;
        else
            break;
    };

    // ��ʼ�������� ProcedureName ����䲿�ֵ�׼������
    //��1) ����Ŀ������code�����ڿ�ʼɨ��������̵�ʱ������������һ�� jmp 0 ? ָ�����ָ��ĵ�ַ�������浽���� cx0 �У�����Ϊ�˱������ڻ������ɨ�赽������̵���䲿�֣�����ȷ�����ָ��ĵ�ַ��ֵ�ˣ�Ӧ�����������ֵַΪ������̵���䲿����Ŀ�������е���ڵ�ַ��Ҳ���ǵ�ǰ�� cx ��ֵ
    // (2) ������ű�table�����ڿ�ʼɨ��������̵�ʱ�������ڷ��ű��еǼ���������̵Ĺ��̱�ʶ�������������еĵ�ַ�ʹ�С�ֶ���δȷ����������ȷ�����������ǵ�ʱ���ˡ�ע�⵽������Ҫ����ı����ָ�뱣�浽���� tx0 ��
    // (2.1) table[tx0].Address = cx ���磨1������ ����ǰ��cx��ֵ���Ǵ˹��̵�ִ����ڵ�ַ��һ�����̵�ִ����ڵ�ַ����������̵���䲿�ֵ���ڵ�ַ
    // (2.2) table[tx0].Size = dx ����ǰ dx ��ֵ���Ǵ˹��������ջ�ռ��С ����ֵ���� 3 ���ϴ˹����ж���ı������������������Ѿ�ɨ�赽�˹��̵���䲿�֣��������岿���Ѿ����������Կ���ȷ�����ֵ��
    // (3) ����һ��ָ�� \"int 0 size\" ��ÿ�����뵽һ�����̵���䲿�־ͻ��������ָ�����Ϊ������̷���ջ�ռ䣬��(2.2)������ָ���е� size ���ǵ�ǰ�� dx ��ֵ������ָ����Ŀ�������еĵ�ַ���ǵ�ǰ�� cx ��ֵ

    // ���� code[cx0] �е� Address �ֶΣ���������Ϊ��ǰ�� cx ��ֵ����Ϊ��ʱ cx Ϊ�˹��̵���䲿�ֵ���ڵ�ַ����ʱ cx �Ǵ˹��̵���ڵ�ַ
    // ����λ�� cx0 �� jmp 0 x ָ���е� x
    code[cx0].address = cx;

    // ���� ��table[tx0].Address = cx �� table[tx0].Size = dx
    // ����λ�� tx0 ������������ block �ķ��ű����ĵ�ַ�ֶκʹ�С�ֶ�
    sym_table[tx0].address = cx;
    sym_table[tx0].size = dx;

    // ��䲿����Ҫ���ɵĵ�һ��ָ���� int , ����Ϊ block ����ջ�ռ� ,����Ĵ�СΪ dx ,ͬʱ���� int ָ��ĵ�ַ�� cx0
    if( !gen(inte, 0, dx) )
        return false;

    // �����﷨��λ statement
    if ( !statement(lev, tx) )
        return false;

    // ��ÿ�����̵�ĩβ������һ�� opr 0 0 �����ڹ��̷���
    if( !gen(opr, 0, 0) )
        return false;

    return true;
}

bool pl0_t::const_declaration(int lev, int *ptx, int *pdx)
{
    // ������������ ident

    // ��� ident �Ƿ�Ϊ��ʶ���Լ��Ƿ��ظ�����
    if( sym != sym_ident )
    {
        error.set(7, line); // ����������ȱ�ٳ�����ʶ��
        return false;
    }

    // if(�˳�����ʶ���ڱ��������Ѿ�������Ϊ������ʶ����������ʶ������̱�ʶ��)
    if (redeclaration(ident, *ptx, lev))
    {
        error.set(22, line); // ��ʶ���ظ�����
        return false;
    }

    // ��鳣�������е� =
    getsym();
    if (sym != sym_eql)
    {
        error.set(8, line); // ȱ�� =
        return false;
    }

    // ��鳣�������еĳ���ֵ
    getsym();
    if( sym != sym_number )
    {
        error.set(9, line); // ȱ�ٳ���ֵ
        return false;
    }

    // ��������ʶ���Ǽǵ����ű���
    if( !enter(id_const, lev, ptx, pdx) )
        return false;
    
    getsym();
    return true;
}

bool pl0_t::var_declaration(int lev, int *ptx, int *pdx)
{
    // ������������ ident

    // ��� ident �Ƿ�Ϊ��ʶ���Լ��Ƿ��ظ�����
    if( sym != sym_ident )
    {
        error.set(7, line); // ȱ�ٱ�ʶ��
        return false;
    }

    if (redeclaration(ident, *ptx, lev))
    {
        error.set(23, line); // ��ʶ���ظ�����
        return false;
    }
    
    // ��������ʶ���Ǽǵ����ű���
    if ( !enter(id_var, lev, ptx, pdx) )
        return false;

    getsym();
    return true;
}

bool pl0_t::statement(int lev, int tx)
{
    if (sym == sym_ident) // ��ֵ���
    {
        // ��鸳ֵ������ߵı�ʶ���Ƿ���һ���Ѷ���ı�ʶ����������һ��������ʶ��
        int i = position(ident, tx);
        if( i == 0 )
        {
            error.set(10, line); // δ����ı�ʶ��
            return false;
        }
        
        if( sym_table[i].kind != id_var)
        {
            error.set(15, line); // ��ֵ���������һ���Ѷ���ı�ʶ�� ��������һ��������ʶ��
            return false;
        }

        // ��鸳ֵ����еĸ�ֵ��
        getsym();
        if( sym != sym_becomes )
        {
            error.set(16, line); // ȱ�� :=
            return false;
        }

        // �����﷨��λ expression
        getsym();
        if( !expression(lev, tx) )
            return false;

        // ������ֵ��� sto ָ���ջ����ֵ�����ʽ��ֵλ��ջ�������Ƶ���ֵ�����ߵı����У���ɸ�ֵ
        if (!gen(sto, lev - sym_table[i].level, sym_table[i].address))
            return false;
    }

    else if( sym == sym_read ) // read ���
    {
        // ��� read ����е�������
        getsym();
        if( sym != sym_lparen )
        {
            error.set(17, line); // ȱ��������
            return false;
        }

        while(1)
        {
            getsym();

            // ��� ident �Ƿ�Ϊ�Ѷ���ı�����ʶ��
            if( sym != sym_ident )
            {
                error.set(7, line); // ȱ�ٱ�ʶ��
                return false;
            }
                
            // �Ƿ����Ѿ�����ı�ʶ��
            int i = position(ident, tx);
            if( i == 0 )
            {
                error.set(10, line); // δ����ı�ʶ��
                return false;
            }
            
            // �Ƿ��Ǳ�����ʶ��
            if (sym_table[i].kind != id_var)
            {
                error.set(15, line); // Ӧ���Ǳ�����ʶ��
                return false;
            }

            // ÿ�������� read ���������ָ��
            // �� opr, 0, 16
            // �� sto, lev-table[i].Level, table[i].Address
            if( !gen(opr, 0, 16) )
                return false;
            if( !gen(sto, lev - sym_table[i].level, sym_table[i].address))
                return false;

            // ��鵱ǰ�����Ƿ�Ϊ����
            // ����Ƕ��� �����������
            // ������Ƕ��ţ��������������б����
            getsym();
            if (sym == sym_comma)
                continue;
            else
                break;
        };

        // ��� read ����е�������
        if( sym != sym_rparen )
        {
            error.set(13, line); // ȱ��������
            return false; //
        }
        else
        {
            getsym();
        }
    }

    else if( sym == sym_write ) // �����Ϊ write ���
    {
        // ��� write ����е�������
        getsym();
        if( sym != sym_lparen )
        {
            error.set(17, line); // ȱ��������
            return false;
        }

        while(1)
        {
            // �����﷨��λ expression
            getsym();
            if( !expression(lev, tx) )
                return false;

            // ���� write ����Ŀ����� opr, 0, 14 �����ڽ�λ��ջ���ı��ʽ��ֵ��ӡ���
            if( !gen(opr, 0, 14) )
                return false;

            // ��鵱ǰ�����Ƿ�Ϊ����
            // ����Ƕ��� ����������
            // ������Ƕ��ţ������������ʽ�б����
            if (sym == sym_comma)
                continue;
            else
                break;
        };

        // ��� write ����е�������
        if( sym != sym_rparen )
        {
            error.set(13, line); // ȱ��������
            return false; // ȱ��)
        }

        // �ڲ��� write ����и������ʽ��ֵ�Ĵ�ӡָ��󣬲���һ�� opr 0 15 ָ�� ���������Ǵ�ӡһ���س����з�
        if( !gen(opr, 0, 15) )
            return false;

        getsym();
    }

    else if (sym == sym_call)   // �����Ϊ call ���
    {
        getsym();

        // ��� ident �Ƿ�Ϊ�Ѷ���Ĺ��̱�ʶ��
        if (sym != sym_ident)   // �Ƿ��Ǳ�ʶ��
        {
            error.set(17, line); // ȱ�ٱ�ʶ��
            return false;
        }

        int i = position( ident, tx );
        if (i == 0) // �Ƿ����Ѿ�����ı�ʶ��
        {
            error.set(10, line); // call ����еı�ʶ��δ����
            return false;
        }

        // �Ƿ��ǹ��̱�ʶ��
        if (sym_table[i].kind != id_procedure)
        {
            error.set(12, line); // Ӧ���ǹ��̱�ʶ��
            return false;
        }

        // ���� cal ָ��
        if( !gen(cal, lev - sym_table[i].level, sym_table[i].address) )
            return false;
            
        getsym();
    }

    else if( sym == sym_if ) // �����Ϊ if ���
    {
        // �����﷨��λ condition
        getsym();
        if( !condition(lev, tx) )
            return false;

        // ��� if ����е� then
        if (sym != sym_then)
        {
            error.set(18, line); // ȱ�� then
            return false;
        }

        // ���� jpc ָ��
        // ������ָ��ĵ�ַ�ݴ浽 cx1 �� �������ڷ����� if ��� �е� statement �� �������ָ���еĵ�ַ�ֶ�
        int cx1 = cx;
        if( !gen(jpc, 0, -1) )
            return false;

        // �����﷨��λ statement
        getsym();
        if( !statement(lev, tx) )
            return false;

        // ������ statement ֮ǰ���ɵ� jpc ָ���еĵ�ַ�ֶ�
        // ׼������ jpc ָ���еĵ�ַ�ֶΣ�Ӧ�û���Ϊ��ǰ�� cx ��ֵ
        code[cx1].address = cx;
    }

    else if( sym == sym_while )  // �����Ϊ while ���
    {
        // �ݴ� while ����� condition ����ڵ�ַ�� cx1
        int cx1 = cx;

        // �����﷨��λ condition
        getsym();
        if( !condition(lev, tx) )
            return false;

        // ���� jpc ָ��
        // ������ָ��ĵ�ַ�ݴ浽 cx2 �� �������ڷ����� while ��� �е� statement �� �������ָ���еĵ�ַ�ֶ�
        int cx2 = cx;
        if( !gen(jpc, 0, -1) )
            return false;

        // ��� while ����е� do
        if (sym != sym_do)
        {
            error.set(19, line); // ȱ�� do
            return false;
        }

        // �����﷨��λ statement
        getsym();
        if( !statement(lev, tx) )
            return false;

        // ���� jmp, 0, cx1 ָ��
        // ���� cx1 �� while ����е� condition ����ڵ�ַ
        if( !gen(jmp, 0, cx1) )
            return false;

        // ������ statement ֮ǰ���ɵ� jpc ָ���еĵ�ַ�ֶ�
        code[cx2].address = cx;
    }

    else if (sym == sym_begin) // �������
    {
        // �����﷨��λ statement
        getsym();
        if (!statement(lev, tx))
            return false;

        while (sym == sym_semicolon)
        {
            // �����﷨��λ statement
            getsym();
            if (!statement(lev, tx))
                return false;
        }

        // ��鸴������е� end
        if (sym != sym_end)
        {
            error.set(20, line); // ȱ�� end
            return false;
        }

        getsym();
    }

    else    // �����
    {
    }

    return true;
}

bool pl0_t::condition(int lev, int tx)
{
    if( sym == sym_odd ) // ��鵽 odd �����
    {
        // �����﷨��λ expression
        getsym();
        if( !expression(lev, tx) )
            return false;

        // ���� opr, 0, 6 ָ�� ����ջ����ֵ�����ʽ��ֵλ��ջ����������ż�жϣ������жϵĽṹ�ŵ�ջ��
        if( !gen(opr, 0, 6) )
            return false;
    }
    else
    {
        // �����﷨��λ expression
        if( !expression(lev, tx) )
            return false;

        // ��� condition �еĹ�ϵ�����
        if (sym != sym_eql && 
            sym != sym_neq && 
            sym != sym_lss && 
            sym != sym_leq && 
            sym != sym_gtr && 
            sym != sym_geq )
        {
            error.set(14, line); // ȱ�ٹ�ϵ�����
            return false;
        }

        sym_e relop = sym;

        // �����﷨��λ expression
        getsym();
        if( !expression(lev, tx) )
            return false;        

        // ���� opr, 0, n ָ�� ������ n ��ʾ��ϵ����������ͣ�
        // �� n = 8  : sym_eql
        // �� n = 9  : sym_neq
        // �� n = 10 : sym_lss
        // �� n = 11 : sym_geq
        // �� n = 12 : sym_gtr
        // �� n = 13 : sym_leq
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
    if (sym == sym_plus || sym == sym_minus)    // ��鵽 + �� - ��Ŀ�����
    {
        // �������Ŀ������ݴ浽 add_minus_op ��
        sym_e add_minus_op = sym;

        // �����﷨��λ term
        getsym();
        if( !term(lev, tx) )
            return false;

        if( add_minus_op == sym_minus)
        {
            // ��� add_minus_op Ϊ sym_minus ������� opr, 0, 1 ָ��
            // ��� add_minus_op Ϊ sym_add ���򲻱ز���ָ��
            if (!gen(opr, 0, 1))
                return false;
        }
    }
    else
    {
        // �����﷨��λ term
        if( !term(lev, tx) )
            return false;
    }

    while( sym == sym_plus || sym == sym_minus )
    {
        // ��鵽 + �� - ˫Ŀ�����
        // �����˫Ŀ������ݴ浽 add_minus_op ��
        sym_e add_minus_op = sym;

        // �����﷨��λ term
        getsym();
        if( !term(lev, tx) )
            return false;

        // ���� opr, 0, n ָ�� ������ n ��ʾ˫Ŀ���������������
        // �� n = 2  : sym_plus
        // �� n = 3  : sym_minus
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
    // �����﷨��λ factor
    if( !factor(lev, tx) )
        return false;

    while( sym == sym_times || sym == sym_slash )
    {
        // ��鵽 * �� / ˫Ŀ�����
        // �����˫Ŀ������ݴ浽 times_slash_op ��
        sym_e times_slash_op = sym;

        // �����﷨��λ factor
        getsym();
        if( !factor(lev, tx) )
            return false;

        // ���� opr, 0, n ָ�� ������ n ��ʾ˫Ŀ���������������
        // �� n = 4  : sym_times
        // �� n = 5  : sym_slash
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
        // factor �Ŀ�ʼ����һ����ʶ��
        // ��������ʶ���Ƿ��������
        int i = position(ident, tx);
        if( i == 0 )
        {
            error.set(10, line); // δ����ı�ʶ��
            return false;
        }

        if( sym_table[i].kind == id_const )
        {
            // ���� lit, 0, table[i].Value ָ��
            if( !gen(lit, 0, sym_table[i].value) )
                return false;
        }
        else if( sym_table[i].kind == id_var )
        {
            // ���� lod, lev-table[i].Level, table[i].Address ָ��
            if( !gen(lod, lev - sym_table[i].level, sym_table[i].address) )
                return false;
        }
        else if (sym_table[i].kind == id_procedure )
        {
            error.set(11, line); // ��Ӧ���ǹ��̱�ʶ��
            return false;
        }

        getsym();
    }

    else if (sym == sym_number)
    {
        // factor �Ŀ�ʼ����һ������
        // ���� ���� lit, 0, table[i].Value ָ��
        if( !gen(lit, 0, num) )
            return false;

        getsym();
    }

    else if ( sym == sym_lparen )
    {
        // factor �Ŀ�ʼ����һ����Բ����

        // �����﷨��λ expression
        getsym();
        if ( !expression(lev, tx) )
            return false;

        // ��� factor �е���Բ����
        if (sym != sym_rparen)
        {
            error.set(13, line); // ȱ�� )
            return false;
        }

        getsym();
    }

    return true;
}


