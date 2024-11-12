#include <iostream>
#include <stack>
#include <string>
#include <map>
#include <vector>

using namespace std;

// 定义终结符和非终结符
enum Symbol { ID, PLUS, MULTIPLY, LPAREN, RPAREN, END, E, T, F, ERROR };

// 定义语法分析动作类型
enum ActionType { SHIFT, REDUCE, ACCEPT, ERROR_ACTION };

// 定义LR分析表的动作
struct Action {
    ActionType type;
    int state; // 移进时的状态或归约时的产生式编号
};

// 定义产生式
struct Production {
    Symbol lhs; // 左部
    int rhsLength; // 右部的长度，用于归约时的出栈操作
};

// LR分析表和GOTO表
map<pair<int, Symbol>, Action> actionTable;
map<pair<int, Symbol>, int> gotoTable;

// 定义产生式集合
vector<Production> productions;

// 初始化产生式
void initializeProductions() {
    // E -> E + T
    productions.push_back({ E, 3 });
    // E -> T
    productions.push_back({ E, 1 });
    // T -> T * F
    productions.push_back({ T, 3 });
    // T -> F
    productions.push_back({ T, 1 });
    // F -> ( E )
    productions.push_back({ F, 3 });
    // F -> id
    productions.push_back({ F, 1 });
}

// 初始化LR分析表和GOTO表
void initializeLRTable() {
    // 根据SLR分析表填入相应的动作
    actionTable[{0, ID}] = { SHIFT, 5 };
    actionTable[{0, LPAREN}] = { SHIFT, 4 };

    actionTable[{1, PLUS}] = { SHIFT, 6 };
    actionTable[{1, END}] = { ACCEPT, -1 };

    actionTable[{2, PLUS}] =  { REDUCE, 2 };
    actionTable[{2, MULTIPLY}] = { SHIFT, 7 };
    actionTable[{2, RPAREN}] = { REDUCE, 2 };
    actionTable[{2, END}] = { REDUCE, 2 };

    actionTable[{3, PLUS}] = { REDUCE, 4 };
    actionTable[{3, MULTIPLY}] = { REDUCE, 4 };
    actionTable[{3, RPAREN}] = { REDUCE, 4 };
    actionTable[{3, END}] = { REDUCE, 4 };

    actionTable[{4, ID}] = { SHIFT, 5 };
    actionTable[{4, LPAREN}] = { SHIFT, 4 };

    actionTable[{5, PLUS}] = { REDUCE, 6 };
    actionTable[{5, MULTIPLY}] = { REDUCE, 6 };
    actionTable[{5, RPAREN}] = { REDUCE, 6 };
    actionTable[{5, END}] = { REDUCE, 6 };

    actionTable[{6, ID}] = { SHIFT, 5 };
    actionTable[{6, LPAREN}] = { SHIFT, 4 };

    actionTable[{7, ID}] = { SHIFT, 5 };
    actionTable[{7, LPAREN}] = { SHIFT, 4 };

    actionTable[{8, PLUS}] = { SHIFT, 6 };
    actionTable[{8, RPAREN}] = { SHIFT, 11 };

    actionTable[{9, PLUS}] = { REDUCE, 1 };
    actionTable[{9, MULTIPLY}] = { SHIFT, 7 };
    actionTable[{9, RPAREN}] = { REDUCE, 1 };
    actionTable[{9, END}] = { REDUCE, 1 };

    actionTable[{10, PLUS}] = { REDUCE, 3 };
    actionTable[{10, MULTIPLY}] = { REDUCE, 3 };
    actionTable[{10, RPAREN}] = { REDUCE, 3 };
    actionTable[{10, END}] = { REDUCE, 3 };

    actionTable[{11, PLUS}] = { REDUCE, 5 };
    actionTable[{11, MULTIPLY}] = { REDUCE, 5 };
    actionTable[{11, RPAREN}] = { REDUCE, 5 };
    actionTable[{11, END}] = { REDUCE, 5 };

    // GOTO表部分示例
    gotoTable[{0, E}] = 1;
    gotoTable[{0, T}] = 2;
    gotoTable[{0, F}] = 3;

    gotoTable[{4, E}] = 8;
    gotoTable[{4, T}] = 2;
    gotoTable[{4, F}] = 3;

    gotoTable[{6, T}] = 9;
    gotoTable[{6, F}] = 3;

    gotoTable[{7, F}] = 10;
}

// 获取输入符号
Symbol getSymbol(char c) {
    switch (c) {
    case '+': return PLUS;
    case '*': return MULTIPLY;
    case '(': return LPAREN;
    case ')': return RPAREN;
    case 'i': return ID;  // 假设 'i' 代表 id
    case '$': return END;
    default: return ERROR;
    }
}

// 打印归约过程
void printReduction(int prodIndex) 
{
    switch (prodIndex) 
    {
    case 0: cout << "按 E' -> E  归约" << endl; break;
    case 1: cout << "按 E -> E + T 归约" << endl; break;
    case 2: cout << "按 E -> T 归约" << endl; break;
    case 3: cout << "按 T -> T * F 归约" << endl; break;
    case 4: cout << "按 T -> F 归约" << endl; break;
    case 5: cout << "按 F -> ( E ) 归约" << endl; break;
    case 6: cout << "按 F -> id 归约" << endl; break;
    }
}
// 移进-归约分析器
void LRParser(string input) {
    stack<int> stateStack;
    stack<Symbol> symbolStack;
    int pointer = 0;

    stateStack.push(0); // 初始状态

    while (true) 
    {
        int currentState = stateStack.top();
        Symbol currentSymbol = getSymbol(input[pointer]);
        /*错误有两种，第一种是输入了错误的字符，比如需要i,输入a,
        
        第二种是输入了终结符，但是当前状态没有对应的动作，
        比如输入了i,当前状态是1，但是1号状态的SLR表格单元空白，
        即没有i对应的动作*/
        if (currentSymbol == ERROR) 
        {
            cout << "Error" << endl;
            pointer++;
            continue; // 跳过错误符号，继续分析
        }

        Action action = actionTable[{currentState, currentSymbol}];

        if (action.type == SHIFT) 
        {
            cout << "移进: " << input[pointer] << endl;
            stateStack.push(action.state);
            symbolStack.push(currentSymbol);
            pointer++;  // 前进到下一个输入符号
        }
        else if (action.type == REDUCE) 
        {
            Production prod = productions[action.state-1];
            printReduction(action.state);

            // 根据产生式右部长度，进行出栈
            for (int i = 0; i < prod.rhsLength; i++) 
            {
                stateStack.pop();
                symbolStack.pop();
            }

            symbolStack.push(prod.lhs); // 将产生式左部符号压入符号栈
            currentState = stateStack.top();
            stateStack.push(gotoTable[{currentState, prod.lhs}]); // GOTO操作

        }
        else if (action.type == ACCEPT) 
        {
            cout <<"接受$，分析成功!" << endl;
            return;
        }
        else //即ERROR_ACTION
        {
            cout<<"Error"<<endl;
            // 处理错误还可详细弄一个函数，输出错误信息，
            //然后继续分析，这里就不写了

        }
    }
}

int main() {
    initializeLRTable();
    initializeProductions();

    string input = "i*i+i$";  // 输入的词法记号流
    LRParser(input);

    return 0;
}

/*
文法规则：
E -> E+T | T
T -> T*F | F
F -> (E) | id
*/
