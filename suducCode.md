# 语法分析伪代码

## 一、PL/0非终结符FOLLOW集合与FIRST集合

表1- PL/0文法非终结符的开始符号集与后继符号集

| 非终结符 | FIRST(S)                       | FOLLOW(S)                 |
| -------- | ------------------------------ | ------------------------- |
| 分程序   | const var ident if begin while | . ; 空集                  |
| 语句     | ident begin if while           | . ; end 空集              |
| 条件     | + - ( ident number             | then do                   |
| 表达式   | = + - ( ident number           | . ; R end then do         |
| 项       | ident number (                 | . ; R + - end then do     |
| 因子     | ident number (                 | . ; R + - * / end then do |

## 二、递归下降子程序*8 + 2

### program程序递归下降

> 由于程序开始必为一个“program”保留字和标识符作为程序首部，因此可以直接线性判断这两个标识符。判断合法后再进入“分程序”的推导

```c++
...类似下面就行了
```



### block分程序递归下降

> 分程序可以推到为“”

```c++
void block(unsigned long fsys)
{
    long tx0;
    long cx0;
    long tx1;
    long dx1;

    dx=3;
    // 地址寄存器给出每层局部量当前已分配到的相对位置
    // 置初始值为 3 的原因是：每一层最开始的位置有三个空间用于存放
    // 静态链 SL、动态链 DL 和 返回地址 RA
    tx0=tx;                        // 记录本层开始时符号表的位置
    table[tx].addr=cx;             // 符号表记下当前层代码的开始地址
    gen(jmp,0,0);                  // block开始时首先写下一句跳转指令，地址到后面再补？？

    if(lev>levmax)                 // 嵌套层数太大？？？我们应该没有嵌套层数吧
    {
        error(32);
    }

    do                             // 开始处理声明部分
    {
        if(sym==constsym)          // 常量
        {
            getsym();

            do
            {
                constdeclaration();// 常量声明
                while(sym==comma)  // 逗号
                {
                    getsym();
                    constdeclaration();// 逗号后面继续常量声明
                }

                if(sym==semicolon) // 分号
                {
                    getsym();
                }
                else
                {
                	// 常量声明没有以分号结尾错误
                    error(5);
                }
            } //while(sym==ident); 这里貌似也有错误，上面的分号后就应该结束常量声明了
        }

        if(sym==varsym)            // 变量
        {
            getsym();
            do
            {
                vardeclaration();  // 变量声明
                while(sym==comma)  // 逗号
                {
                    getsym();
                    vardeclaration();// 逗号后面继续声明变量
                }

                if(sym==semicolon) // 分号
                {
                    getsym();
                }
                else
                {
                	// 变量声明没有以分号结尾错误
                    error(5);
                }
            } //while(sym==ident);	此处貌似有问题，分号后应该结束
        }

		// 感觉这里我们不需要，产生式直接线性进行“语句”递归下降程序即可
        test(statbegsys|ident,declbegsys,7);
        
    } // while(sym&declbegsys); // 声明必须符合线性，即先常量后变量，因此不存在循环

    code[table[tx0].addr].a=cx;    // 把block开头写下的跳转指令的地址补上（这里不太明白）
    table[tx0].addr=cx;            // tx0的符号表存的是当前block的参数？？？
    cx0=cx;
    gen(Int,0,dx);
    statement(fsys|semicolon|endsym);	// 进入“语句”递归下降
    gen(opr,0,0); // return？？？
    test(fsys,0,8);	// ？？？好像也不需要检查
    //listcode(cx0);
}

```



### constDeclaration常量定义递归下降

> 由于这个表达式的FIRST中只有`const`，故在block中读取，此递归下降子程序中<u>无需处理该保留字</u>
>
> 本程序处理单个常量定义，并进行符号表的相关操作

```c++
void constdeclaration()
{
    // 以无符号整数开头
    if(sym == ident)
    {
        getsym();

        if(sym == becomes) // 出现赋值号
        {
            if(sym == becomes)     // 赋值号报错
            {
                error(1);
            }

            getsym();

            if(sym == number)      // 将数字登录到符号表
            {
                enter(constant);
                getsym();
            }
            else
            {
                error(2);
            }
        }
        else
        {
            error(3);
        }
    }
    else
    {
        error(4);
    }
}
```



### varDeclaration变量定义递归下降

> 由于这个表达式的FIRST中只有`var`，故在block中读取，此递归下降子程序中无需处理该保留字
>
> 处理单个变量定义

```c++
void vardeclaration()
{
    if(sym == ident)
    {
        enter(variable);           // 将标识符登录到符号表中
        getsym();
    }
    else // 不是标识符则直接报错
    {
        error(4);
    }
}
```



### statement语句递归下降

> 可以推为<赋值语句>|<条件语句>|<循环语句>|<复合语句>|空（即可检查FOLLOW来直接退出）
>
> 而由于各种语句的“推导”均为线性，因此各语句的递归下降均可以在此处直接实现

```c++
void statement(unsigned long fsys)
{
    long i,cx1,cx2;

    if(sym==ident)                 // 赋值语句的FIRST为“标识符”
    {
        i=position(id);            // 查符号表
        if(i==0)
        {
            error(11);             // 变量未定义
        }
        else if(table[i].kind!=variable)
        {
            error(12);             // 非变量，报错
            i=0;
        }

        getsym();

        if(sym==becomes)           // 赋值语句中，变量后无赋值符号则错误
        {
            getsym();
        }
        else
        {
            error(13);
        }

        expression(fsys);          // 进入“表达式”递归下降函数

        if(i!=0)                   // 若未出错，则产生一个sto代码？？？
        {
            gen(sto,lev-table[i].level,table[i].addr);
        }
    }
    else if(sym==ifsym)            // 条件语句的FIRST为“if”
    {
        getsym();
        condition(fsys|thensym|dosym);// 进入“条件”的递归下降函数

        if(sym==thensym)	// 判断语句后若不是“then”保留字则报错
        {
            getsym();
        }
        else
        {
            error(16);
        }
        cx1=cx;		// 提前预留“为假出口”，并再之后回填
        gen(jpc,0,0);
        statement(fsys);           // 进入“语句”递归下降函数（递归）
        code[cx1].a=cx;
    }
    else if(sym==beginsym)         // 复合语句的FIRST只有“begin”
    {
        getsym();
        statement(fsys|semicolon|endsym); // 后面是stmt
        // 处理分号和语句（这里或许可以改成 while(sym==semicolon)，为“语句”的FIRST则可直接报错）
        while(sym==semicolon||(sym&statbegsys))
        {
            if(sym==semicolon)     // 复合语句第一条语句后若不是分号，则报错
            {
                getsym();
            }
            else
            {
                error(10);
            }
            statement(fsys|semicolon|endsym);
        }
        if(sym==endsym)	// 复合语句最后若不是“end”保留字，则报错
        {
            getsym();
        }
        else
        {
            error(17);
        }
    }
    else if(sym==whilesym)         // 循环语句的FIRST只有WHILE保留字
    {
        cx1=cx;                    // 记录中间代码起始指针
        getsym();
        condition(fsys|dosym);     // 后面是一个cond
        cx2=cx;                    // 记录中间代码位置，要放退出地址
        gen(jpc,0,0);
        if(sym==dosym)             // do语句
        {
            getsym();
        }
        else
        {
            error(18);
        }

        statement(fsys);           // 后面是stmt
        gen(jmp,0,cx1);            // 循环跳转

        code[cx2].a=cx;            // 把退出地址补上
    }

    // 语句可以为空，故进行FOLLOW判断？？？
    test(fsys,0,19);
}
```



### expression表达式递归下降

> 表达式可以认为是一个可以由“加法运算符”和一个“项”的闭包，为线性结构

```c++
void expression(unsigned long fsys)
{
    unsigned long addop;

    if(sym==plus || sym==minus)    // 处理正负号（可选）
    {
        addop=sym;                 // 保存正负号
        getsym();

        term(fsys|plus|minus);     // 正负号后面是一个term

        if(addop==minus)
        {
            gen(opr,0,1);          // 负号，取反运算
        }
    }
    else	// 若开头无加法运算符，则直接进入“项”递归下降函数
    {
        term(fsys|plus|minus);
    }

    // 若后面仍有“加法运算符”连接的表达式
    while(sym==plus || sym==minus) // 处理加减
    {
        addop=sym;                 // 保存运算符
        getsym();

        term(fsys|plus|minus);     // 运算符后是一个term

        if(addop==plus)
        {
            gen(opr,0,2);          // 加
        }
        else
        {
            gen(opr,0,3);          // 减
        }
    }
}
```



### term项递归下降

> “项”可以认为是以“因子”开头的，跟着“乘法运算符+因子”闭包的线性正则表达式

```c++
void term(unsigned long fsys)
{
    unsigned long mulop;

    factor(fsys|times|slash);      // 每个term都应该从factor开始

    while(sym==times || sym==slash)// 处理乘除
    {
        mulop = sym;               // 保存当前运算符
        getsym();

        factor(fsys|times|slash);  // 运算符之后是一个factor

        if(mulop == times)
        {
            gen(opr,0,4);          // 乘法
        }
        else{
            gen(opr,0,5);          // 除法
        }
    }
}
```



### factor因子递归下降

> 因子可以推导为“标识符/无符号正式/（表达式）”

```c++
void factor(unsigned long fsys)
{
    long i;

    // 判断符号是否处在“因子”的首符集中
    test(facbegsys, fsys, 24);     // 开始因子处理前，先检查当前 token 是否在 facbegsys 集合中
                                   // 如果不是合法的 token，抛 24 号错误，并通过 fsys 集恢复使语法处理可以继续进行

    while(sym & facbegsys)
    {
        if(sym == ident)           // 遇到标识符
        {
            i = position(id);      // 查符号表

            if(i==0)               // 标识符未定义
            {
                error(11);
            }
            else
            {
                switch(table[i].kind)
                {
                    case constant: //常量
                        gen(lit, 0, table[i].val);
                        break;

                    case variable: //变量
                        gen(lod, lev-table[i].level, table[i].addr);
                        break;
                }
            }

            getsym();
        }
        else if(sym == number)     // 遇到数字
        {
            if(num>amax)	// 数字过大
            {
                error(31); num=0;
            }

            gen(lit,0,num);
            getsym();
        }
        else if(sym == lparen)     // 遇到左括号
        {
            getsym();
            expression(rparen|fsys); // 后面是一个exp

            if(sym==rparen)        // 子表达式结束，应该遇到右括号
            {
                getsym();
            }
            else
            {
                error(22);
            }
        }

		// ？？？
        test(fsys,lparen,23);      // 一个因子处理完毕，遇到的 token 应在 fsys 集合中
                                   // 如果不是，抛 23 号错，并找到下一个因子的开始，使语法分析可以继续运行下去
    }
}
```



### condition条件分析递归下降

```c++
void condition(unsigned long fsys)
{
    unsigned long relop;

    expression(fsys|eql|neq|lss|gtr|leq|geq); // 后面是一个exp

    if(!(sym&(eql|neq|lss|gtr|leq|geq)))	// 不是逻辑运算则报错
    {
		error(20);
	}
    else
    {
    	relop=sym;             // 保存当前运算符
        getsym();
		
        expression(fsys);      // 处理表达式右边
		switch(relop)
        {
        	case eql:
            	gen(opr, 0, 8);
            break;

            case neq:
            	gen(opr, 0, 9);
			break;

            case lss:
            	gen(opr, 0, 10);
            break;

            case geq:
            	gen(opr, 0, 11);
            break;

            case gtr:
            	gen(opr, 0, 12);
            break;

            case leq:
            	gen(opr, 0, 13);
            break;
		}
     }
}
```

