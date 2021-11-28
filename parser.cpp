#include <iostream>
#include <algorithm>

#include "base.h"
#include "parser.h"

namespace solver {

void Parser::GetNextToken() {
    cur_token_ = peek_token_;
    peek_token_ = lexer_->GetNextToken();
}

void Parser::ParseProgram() {
    std::cout << "Start parse program..." << std::endl;
    while (!CurTokenIs(FEOF)) {
        // 处理注释
        SkipComment();
        // 处理合约
#ifdef NOHANDLE
        patch.push_back(cur_token_);
#endif

#ifdef HANDEL
        if (CurTokenIs(CONTRACT)) {
            // 解析全局变量 局部变量等
            Clear();
            ParseContract();
            GetCodePatch();
            continue;
        } else if (CurTokenIs(LIBRARY)) {
            ParseLibrary();
            continue;
        }
#endif
        GetNextToken();
    }
    std::cout << "Finish parse program. Contract num = "<< total_contract_num_ 
              << ". Function num = " << total_function_num_ << std::endl;
}

void Parser::ParseContract() {
    total_contract_num_++;
    // 解析合约的名字
    GetNextToken();
    std::string contract_name = cur_token_->GetLiteral();
    struct_enum_names_.insert(contract_name);
    std::cout << "Start parse contract " << contract_name << "..." << std::endl;
    while (!CurTokenIs(LBRACE)) {
        GetNextToken();
    }
    std::stack<Token*> st;
    st.push(cur_token_);
    GetNextToken();
    while (!st.empty() && !CurTokenIs(FEOF)) {
        // 处理注释
        SkipComment();
#ifdef DEBUG
        std::cout << "CurTokenIs: " << cur_token_->GetLiteral() << std::endl;
#endif
        if (CurTokenIs(LBRACE)) {
            st.push(cur_token_);
        } else if (CurTokenIs(RBRACE)) {
            st.pop();
        } else if (Util::IsVirableDefine(cur_token_->GetLiteral()) ||
                   struct_enum_names_.find(cur_token_->GetLiteral()) != struct_enum_names_.end()) {
            // 处理全局变量
            ParseGlobalVar();
            continue;
        } else if (CurTokenIs(FUNCTION)) {
            // 处理函数
            ParseFunction();
            continue;
        } else if (CurTokenIs(EVENT)) {
            // 处理事件
            ParseEvent();
            continue;
        } else if (CurTokenIs(MODIFIER)) {
            // 处理 modifier
            ParseModifier();
            continue;
        } else if (CurTokenIs(CONSTRUCTOR)) {
            // 处理 constructor
            ParseConstructor();
            continue;
        } else if (CurTokenIs(USING)) {
            while (!CurTokenIs(SEMICOLON)) {
                GetNextToken();
            }
        }
        GetNextToken();
    }
    std::cout << "Finish parse contract." << std::endl;
}

void Parser::ParseLibrary() {
    // 暂时不处理 特殊处理了 SafeMath 库
    while (!CurTokenIs(LBRACE) && !CurTokenIs(SEMICOLON)) {
        GetNextToken();
    }
    if (CurTokenIs(SEMICOLON)) {
        GetNextToken();
        return;
    }
    int st = 1;
    GetNextToken();
    while (st != 0) {
        SkipComment();
        if (CurTokenIs(LBRACE)) st++;
        if (CurTokenIs(RBRACE)) st--;
        GetNextToken();
    }
    return;
}
void Parser::ParseGlobalVar() {
    std::vector<Token*> t;
    // 特殊处理 mapping
    if (CurTokenIs(MAPPING)) {
        while (!PeekTokenIs(SEMICOLON)) {
            t.push_back(cur_token_);
            GetNextToken();
        }
        std::string var_name = cur_token_->GetLiteral();
        t.push_back(cur_token_);
        GetNextToken();
        t.push_back(cur_token_);
        GetNextToken();
        global_var_tokens_[var_name] = t;
        return;
    }
    bool flag = false;
    if (CurTokenIs(STRUCT) || CurTokenIs(VENUM)) {
        flag = true;
    }
    // 跳过第一个变量定义的 token
    t.push_back(cur_token_);
    GetNextToken();
    while (!CurTokenIs(IDENT)) {
        t.push_back(cur_token_);
        GetNextToken();
    }
    std::string var_name = cur_token_->GetLiteral();
#ifdef DEBUG
    std::cout<<var_name << std::endl;
#endif
    if (flag) {
        struct_enum_names_.insert(var_name);
    }
    int st = 0;
    while (true) {
        // 处理注释
        SkipComment();
        t.push_back(cur_token_);
        if (CurTokenIs(LBRACE)) st ++;
        if (CurTokenIs(RBRACE)) {
            st --;
            if (st == 0) break; 
        }
        if (CurTokenIs(SQUO)) {
            t.push_back(new Token(STRINGVAL, "string_val", cur_token_->GetLine()));
            GetNextToken();
            while (!CurTokenIs(SQUO)) {
                GetNextToken();
            }
            t.push_back(cur_token_);
        }
        if (CurTokenIs(DQUO)) {
            t.push_back(new Token(STRINGVAL, "string_val", cur_token_->GetLine()));
            GetNextToken();
            while (!CurTokenIs(DQUO)) {
                GetNextToken();
            }
            t.push_back(cur_token_);
        }
        if (st == 0 && CurTokenIs(SEMICOLON)) break;
        GetNextToken();
    }
    GetNextToken();
    global_var_tokens_[var_name] = t;
    // std::cout<<var_name << " end. Cur token is " << cur_token_.GetLiteral() << std::endl;
}

void Parser::ParseFunction() {
    total_function_num_++;
    // 函数名可能会和api一样 比如 transfer
    std::vector<Token*> t;
    while (!CurTokenIs(IDENT) && !Util::IsAPI(cur_token_->GetType()) && !CurTokenIs(LBRACE)) {
        SkipComment();
        t.push_back(cur_token_);
        GetNextToken();
    }
    // 加入函数池
    std::string func_name = "func";
    // 函数名为空？
    if (!CurTokenIs(LBRACE)){
        func_name = cur_token_->GetLiteral();
    }
#ifdef DEBUG
    std::cout << "FUNC:" << func_name << std::endl;
#endif
    while (!CurTokenIs(LBRACE) && !CurTokenIs(SEMICOLON)) {
        SkipComment();
        if (CurTokenIs(LBRACE) || CurTokenIs(SEMICOLON)) break;
        t.push_back(cur_token_);
#ifdef DEBUG
        std::cout << cur_token_->GetLiteral() << std::endl;
#endif
        if (CurTokenIs(SQUO)) {
            t.push_back(new Token(STRINGVAL, "string_val", cur_token_->GetLine()));
            GetNextToken();
            while (!CurTokenIs(SQUO)) {
                GetNextToken();
            }
            t.push_back(cur_token_);
        } else if (CurTokenIs(DQUO)) {
            t.push_back(new Token(STRINGVAL, "string_val", cur_token_->GetLine()));
            GetNextToken();
            while (!CurTokenIs(DQUO)) {
                GetNextToken();
            }
            t.push_back(cur_token_);
        } 
        GetNextToken();
    }
    t.push_back(cur_token_);
    if (CurTokenIs(SEMICOLON)) return;
    std::stack<Token*> st;
    st.push(cur_token_);
    GetNextToken();
    while (!st.empty() && !CurTokenIs(FEOF)) {
        // 处理注释
        SkipComment();
        t.push_back(cur_token_);
#ifdef DEBUG
        std::cout << cur_token_->GetLiteral() << std::endl;
#endif
        if (CurTokenIs(LBRACE)) {
            st.push(cur_token_);
        } else if (CurTokenIs(RBRACE)) {
            st.pop();
        } else if (CurTokenIs(USING)) {
            while (!CurTokenIs(SEMICOLON)) {
                GetNextToken();
                t.push_back(cur_token_);
            }
            t.push_back(cur_token_);
        } else if (CurTokenIs(SQUO)) {
            t.push_back(new Token(STRINGVAL, "string_val", cur_token_->GetLine()));
            GetNextToken();
            while (!CurTokenIs(SQUO)) {
                GetNextToken();
            }
            t.push_back(cur_token_);
        } else if (CurTokenIs(DQUO)) {
            t.push_back(new Token(STRINGVAL, "string_val", cur_token_->GetLine()));
            GetNextToken();
            while (!CurTokenIs(DQUO)) {
                GetNextToken();
            }
            t.push_back(cur_token_);
        }
        GetNextToken();
    }
    function_tokens_[func_name] = t;
}

void Parser::ParseEvent() {
    // 暂时不处理
    while (!CurTokenIs(SEMICOLON)) {
        GetNextToken();
    }
}

void Parser::ParseModifier() {
    // 暂时不处理
    while (!CurTokenIs(LBRACE) && !CurTokenIs(SEMICOLON)) {
        GetNextToken();
    }
    if (CurTokenIs(SEMICOLON)) {
        GetNextToken();
        return;
    }
    int st = 1;
    GetNextToken();
    while (st != 0) {
        SkipComment();
        if (CurTokenIs(LBRACE)) st++;
        if (CurTokenIs(RBRACE)) st--;
        GetNextToken();
    }
    return;
}

void Parser::ParseConstructor() {
    // 暂时不处理
    while (!CurTokenIs(LBRACE) && !CurTokenIs(SEMICOLON)) {
        GetNextToken();
    }
    if (CurTokenIs(SEMICOLON)) {
        GetNextToken();
        return;
    }
    int st = 1;
    GetNextToken();
    while (st != 0) {
        SkipComment();
        if (CurTokenIs(LBRACE)) st++;
        if (CurTokenIs(RBRACE)) st--;
        GetNextToken();
    }
    return;
}

void Parser::GetCodePatch() {
    std::unordered_map<std::string, int> visited_func;
    for (auto it = global_var_tokens_.begin(); it != global_var_tokens_.end(); ++ it) {
        std::string name = it->first;
        auto tokens = it->second;
        std::vector<Token*> patch;
        for (auto f_it = function_tokens_.begin(); f_it != function_tokens_.end(); ++ f_it) {
            auto f_tokens = f_it->second;
            bool flag = false;
            for (auto t : f_tokens) {
                if (t->GetLiteral() == name) {
                    flag = true;
                    break;
                }
            }
            if (flag) {
                visited_func[f_it->first] = 1;
                for (auto t : f_tokens) {
                    patch.push_back(t);
                }
            }
        }
        if (patch.size() > 0) {
            for (auto t : tokens) {
                patch.push_back(t);
            }
            int func_no = 1;
            int var_no = 1;
            std::unordered_map<std::string, int> func_name;
            std::unordered_map<std::string, int> var_name;
            for (auto t : patch) {
                std::string li = t->GetLiteral();
                std::string ty = t->GetType();
                if (function_tokens_.find(li) != function_tokens_.end()) {
                    if (func_name.find(li) == func_name.end()) {
                        func_name[li] = func_no;
                        func_no++;
                    }
                    t->ReplaceLiteral("FUN" + std::to_string(func_name[li]));
                } else if (ty == "IDENT") {
                    if (var_name.find(li) == var_name.end()) {
                        var_name[li] = var_no;
                        var_no++;
                    }
                    t->ReplaceLiteral("VAR" + std::to_string(var_name[li]));
                }
            }
            code_patches.push_back(patch);
        }
    }
    for (auto f_it = function_tokens_.begin(); f_it != function_tokens_.end(); ++ f_it) {
        auto f_tokens = f_it->second;
        std::vector<Token*> patch;
        if (visited_func.find(f_it->first) == visited_func.end()) {
            for (auto t : f_tokens) {
                patch.push_back(t);
                int func_no = 1;
                int var_no = 1;
                std::unordered_map<std::string, int> func_name;
                std::unordered_map<std::string, int> var_name;
                for (auto t : patch) {
                    std::string li = t->GetLiteral();
                    std::string ty = t->GetType();
                    if (function_tokens_.find(li) != function_tokens_.end()) {
                        if (func_name.find(li) == func_name.end()) {
                            func_name[li] = func_no;
                            func_no++;
                        }
                        t->ReplaceLiteral("FUN" + std::to_string(func_name[li]));
                    } else if (ty == "IDENT") {
                        if (var_name.find(li) == var_name.end()) {
                            var_name[li] = var_no;
                            var_no++;
                        }
                        t->ReplaceLiteral("VAR" + std::to_string(var_name[li]));
                    }
                }
            }
        }
        if (patch.size() > 0) {
            code_patches.push_back(patch);
        }
    }
}

bool Parser::CurTokenIs(std::string t) { return cur_token_->GetType() == t; }

bool Parser::PeekTokenIs(std::string t) { return peek_token_->GetType() == t; }

void Parser::SkipComment() {
    if (CurTokenIs(CCOMMENT)) {
        auto cur_line = cur_token_->GetLine();
        while (!CurTokenIs(FEOF) && cur_token_->GetLine() == cur_line) {
            GetNextToken();
        }
    } else if (CurTokenIs(LPCOMMENT)) {
        while (!CurTokenIs(FEOF) && !CurTokenIs(RPCOMMENT)) {
            GetNextToken();
        }
        GetNextToken();
    }
    if (CurTokenIs(CCOMMENT) || CurTokenIs(LPCOMMENT))
        SkipComment();
}

void Parser::Clear() {
    function_tokens_.clear();
}

void Parser::PrintTokenFlow() {
    std::cout << "global variables:" << std::endl;
    for (auto it = global_var_tokens_.begin(); it != global_var_tokens_.end(); ++ it) {
        std::string name = it->first;
        auto tokens = it->second;
        std::cout << name << std::endl;
        for (auto t : tokens) {
            std::cout << "(" << t->GetLine() << ","
                      << t->GetType() << ","
                      << t->GetLiteral() << ") ";
        }
        std::cout << std::endl;
    }
    std::cout << "functions:" << std::endl;
    for (auto it = function_tokens_.begin(); it != function_tokens_.end(); ++ it) {
        std::string name = it->first;
        auto tokens = it->second;
        std::cout << name << std::endl;
        for (auto t : tokens) {
            std::cout << "(" << t->GetLine() << ","
                      << t->GetType() << ","
                      << t->GetLiteral() << ") ";
        }
        std::cout << std::endl;
    }
}

} // namespace solver