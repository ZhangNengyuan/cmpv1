#include "koopatoriscv.hpp"
#include "ast.hpp"

using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);
string mode;

int main(int argc, const char *argv[]) {
    // 解析命令行参数
    assert(argc == 5);
    mode = argv[1];
    auto input = argv[2];
    auto output = argv[4];

    // 打开输入文件
    yyin = fopen(input, "r");
    assert(yyin);

    // 解析 AST
    unique_ptr<BaseAST> ast;
    auto ret = yyparse(ast);
    assert(!ret);

    if (mode == "-koopa") {
        // 直接输出到目标文件
        freopen(output, "w", stdout);
        ast->Dump();
        fclose(stdout);
    }
    else if (mode == "-riscv") {

        stringstream buffer;
        streambuf *old_buf = cout.rdbuf(buffer.rdbuf());
        
        ast->Dump();
        cout.rdbuf(old_buf);
        
        string koopa_ir = buffer.str();
        
        freopen(output, "w", stdout);
        koopa_dump(koopa_ir.c_str());
        fclose(stdout);
    }
    return 0;
}



