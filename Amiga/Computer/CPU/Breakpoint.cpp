// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

#include <sstream>
#include <regex>

// Token identifiers
enum Token {
    TOK_DELIM,
    TOK_LEFT, TOK_RIGHT,
    TOK_B, TOK_W, TOK_L,
    TOK_D0, TOK_D1, TOK_D2, TOK_D3, TOK_D4, TOK_D5, TOK_D6, TOK_D7,
    TOK_A0, TOK_A1, TOK_A2, TOK_A3, TOK_A4, TOK_A5, TOK_A6, TOK_A7,
    TOK_DEC, TOK_HEX,
    TOK_EQ, TOK_UNEQ, TOK_LESSEQ, TOK_LESS, TOK_GREATEREQ, TOK_GREATER,
    TOK_NOT, TOK_AND, TOK_OR
};

// Regular expressions for all tokens
const vector<pair<Token, regex>> regexes
{
    { TOK_DELIM,     regex("^(\\s)+") },
    { TOK_LEFT,      regex("^\\(") },
    { TOK_RIGHT,     regex("^\\)") },
    { TOK_B,         regex("^\\.b") },
    { TOK_W,         regex("^\\.w") },
    { TOK_L,         regex("^\\.l") },
    { TOK_D0,        regex("^D0") },
    { TOK_D1,        regex("^D1") },
    { TOK_D2,        regex("^D2") },
    { TOK_D3,        regex("^D3") },
    { TOK_D4,        regex("^D4") },
    { TOK_D5,        regex("^D5") },
    { TOK_D6,        regex("^D6") },
    { TOK_D7,        regex("^D7") },
    { TOK_A0,        regex("^A0") },
    { TOK_A1,        regex("^A1") },
    { TOK_A2,        regex("^A2") },
    { TOK_A3,        regex("^A3") },
    { TOK_A4,        regex("^A4") },
    { TOK_A5,        regex("^A5") },
    { TOK_A6,        regex("^A6") },
    { TOK_A7,        regex("^A7") },
    { TOK_DEC,       regex("^[0-9]+") },
    { TOK_HEX,       regex("^[$][0-9a-fA-F]+") },
    { TOK_EQ,        regex("^==") },
    { TOK_UNEQ,      regex("^==") },
    { TOK_LESSEQ,    regex("^<=") },
    { TOK_LESS,      regex("^<") },
    { TOK_GREATEREQ, regex("^>=") },
    { TOK_GREATER,   regex("^>") },
    { TOK_NOT,       regex("^!") },
    { TOK_AND,       regex("^&&") },
    { TOK_OR,        regex("^\\|\\|") },
};

typedef vector<pair<Token, string>> TokenStream;

// Abstract syntax tree types
enum ASTNodeType {
    
    AST_D0, AST_D1, AST_D2, AST_D3, AST_D4, AST_D5, AST_D6, AST_D7,
    AST_A0, AST_A1, AST_A2, AST_A3, AST_A4, AST_A5, AST_A6, AST_A7,
    AST_DEC, AST_HEX,
    AST_IND_B, AST_IND_W, AST_IND_L,
    AST_EQ, AST_UNEQ, AST_LESSEQ, AST_LESS, AST_GREATEREQ, AST_GREATER,
    AST_NOT, AST_AND, AST_OR
};

class ASTNode {
    
private:
    
    ASTNodeType type;
    uint32_t value = 0;
    ASTNode *left = NULL;
    ASTNode *right = NULL;
    
public:
    
    ASTNode(ASTNodeType type, uint32_t value = 0);
    ASTNode(ASTNodeType type, ASTNode *left, ASTNode *right = NULL);
    ~ASTNode();
    
    static ASTNode *parse(string s);
    
    // Evaluates a breakpoint
    uint32_t eval();
    
    /* Returns a textual representation of the breakpoint condition
     * If an AST is present, the returned string is derived by traversing the
     * tree. If not AST is present, the user input string is returned.
     */
    string name();
    
private:
    
    static ASTNode *parse(TokenStream tokens);
    static bool parseToken(TokenStream tokens, Token token, int &i);
    static ASTNode *parseDEC(TokenStream tokens, int &i);
    static ASTNode *parseHEX(TokenStream tokens, int &i);
    static ASTNode *parseDIRECT(TokenStream tokens, int &i);
    static ASTNode *parseREGISTER(TokenStream tokens, int &i);
    static ASTNode *parseD1(TokenStream tokens, int &i);
    static ASTNode *parseA0(TokenStream tokens, int &i);
    static ASTNode *parseA1(TokenStream tokens, int &i);
    static ASTNode *parseINDIRECT(TokenStream tokens, int &i);
    static ASTNode *parseVALUE(TokenStream tokens, int &i);
    static ASTNode *parseBOOL(TokenStream tokens, int &i);
    static ASTNode *parseBOOL1(TokenStream tokens, int &i);
    static ASTNode *parseBOOL2(TokenStream tokens, int &i);
    static ASTNode *parseATOMIC(TokenStream tokens, int &i);
};

#define PARSE_PREPARE \
ASTNode *left = NULL, *right = NULL; \
int oldi = i; \
if (tokens.size() <= i) { return NULL; } \

#define SYNTAX_ERROR \
{ \
i = oldi; \
if (left) delete left; \
if (right) delete right; \
return NULL; \
}

// A simple greedy tokenizer
vector<pair<Token, string>> tokenize(string input) {
    
    vector<pair<Token, string>> result;
    std::smatch m;
    bool hit;
    
    do {
        hit = false;
        
        // Iterate through the token list
        for (auto r = regexes.begin(); r != regexes.end(); ++r) {
            
            // Does the current token match the beginning of the input string?
            if (std::regex_search(input, m, r->second)) {
                
                // Only proceed if we have at least one matching character
                size_t size = m[0].str().size();
                assert(size != 0);
                
                // Collect the token if it is not a delimiter
                if (r->first != TOK_DELIM) {
                    result.push_back(make_pair(r->first, input.substr(0, size)));
                }
                
                // Delete the scanned token from the input string
                input.erase(0, size);
                hit = true;
                break;
            }
        }
    } while (hit);
    
    /*
     cout << "Result:\n";
     for (auto match : result) {
     cout << match.first << ": " << match.second << endl;
     }
     */
    
    // Check for syntax error
    if (input != "") {
        result.clear();
    }
    
    return result;
}

std::vector<std::string> split(const string& input, const string& regex) {
    
    std::regex re(regex);
    std::sregex_token_iterator first{input.begin(), input.end(), re, 1}, last;
    return {first, last};
}

ASTNode::ASTNode(ASTNodeType type, uint32_t value) {
    
    this->type = type;
    this->value = value;
}

ASTNode::ASTNode(ASTNodeType type, ASTNode *left, ASTNode *right) {
    
    this->type = type;
    this->left = left;
    this->right = right;
}

ASTNode::~ASTNode() {
    
    if (left) delete left;
    if (right) delete right;
}

uint32_t
ASTNode::eval() {
    
    switch (type) {
        case AST_D0:        return m68k_get_reg(NULL, M68K_REG_D0);
        case AST_D1:        return m68k_get_reg(NULL, M68K_REG_D1);
        case AST_D2:        return m68k_get_reg(NULL, M68K_REG_D2);
        case AST_D3:        return m68k_get_reg(NULL, M68K_REG_D3);
        case AST_D4:        return m68k_get_reg(NULL, M68K_REG_D4);
        case AST_D5:        return m68k_get_reg(NULL, M68K_REG_D5);
        case AST_D6:        return m68k_get_reg(NULL, M68K_REG_D6);
        case AST_D7:        return m68k_get_reg(NULL, M68K_REG_D7);
        case AST_A0:        return m68k_get_reg(NULL, M68K_REG_A0);
        case AST_A1:        return m68k_get_reg(NULL, M68K_REG_A1);
        case AST_A2:        return m68k_get_reg(NULL, M68K_REG_A2);
        case AST_A3:        return m68k_get_reg(NULL, M68K_REG_A3);
        case AST_A4:        return m68k_get_reg(NULL, M68K_REG_A4);
        case AST_A5:        return m68k_get_reg(NULL, M68K_REG_A5);
        case AST_A6:        return m68k_get_reg(NULL, M68K_REG_A6);
        case AST_A7:        return m68k_get_reg(NULL, M68K_REG_A7);
        case AST_DEC:       return value;
        case AST_HEX:       return value;
        case AST_IND_B:     return m68k_read_memory_8(value);
        case AST_IND_W:     return m68k_read_memory_16(value);
        case AST_IND_L:     return m68k_read_memory_32(value);
        case AST_EQ:        return left->eval() == right->eval();
        case AST_UNEQ:      return left->eval() != right->eval();
        case AST_LESSEQ:    return left->eval() <= right->eval();
        case AST_LESS:      return left->eval() <  right->eval();
        case AST_GREATEREQ: return left->eval() >= right->eval();
        case AST_GREATER:   return left->eval() >  right->eval();
        case AST_NOT:       return !left->eval();
        case AST_AND:       return left->eval() && right->eval();
        case AST_OR:        return left->eval() || right->eval();
            
        default:
            assert(false);
    }
}

std::string
ASTNode::name() {
    
    std::stringstream ss;
    string ll = "";
    string lr = "";
    string rl = "";
    string rr = "";
    
    if (type == AST_AND || type == AST_OR) {
        if (left->type == AST_AND || left->type == AST_OR) {
            ll = "("; lr = ")";
        }
        if (right->type == AST_AND || right->type == AST_OR) {
            rl = "("; rr = ")";
        }
    }
    
    switch (type) {
        case AST_D0:        return "D0";
        case AST_D1:        return "D1";
        case AST_D2:        return "D2";
        case AST_D3:        return "D3";
        case AST_D4:        return "D4";
        case AST_D5:        return "D5";
        case AST_D6:        return "D6";
        case AST_D7:        return "D7";
        case AST_A0:        return "A0";
        case AST_A1:        return "A1";
        case AST_A2:        return "A2";
        case AST_A3:        return "A3";
        case AST_A4:        return "A4";
        case AST_A5:        return "A5";
        case AST_A6:        return "A6";
        case AST_A7:        return "A7";
        case AST_DEC:       ss << value; return ss.str();
        case AST_HEX:       ss << "$" << std::hex << value; return ss.str();
        case AST_IND_B:     return "(" + left->name() + ").b";
        case AST_IND_W:     return "(" + left->name() + ").w";
        case AST_IND_L:     return "(" + left->name() + ").l";
        case AST_EQ:        return left->name() + " == " + right->name();
        case AST_UNEQ:      return left->name() + " != " + right->name();
        case AST_LESSEQ:    return left->name() + " <= " + right->name();
        case AST_LESS:      return left->name() + " < " + right->name();
        case AST_GREATEREQ: return left->name() + " >= " + right->name();
        case AST_GREATER:   return left->name() + " > " + right->name();
        case AST_NOT:       return "!(" + left->name() + ")";
        case AST_AND:       return ll + left->name() + lr + " && " + rl + right->name() + rr;
        case AST_OR:        return ll + left->name() + lr + " || " + rl + right->name() + rr;
            
        default:
            assert(false);
    }
}

ASTNode *
ASTNode::parse(string s) {
    
    // Tokenize input
    vector<pair<Token, string>> tokens = tokenize(s);
    
    // Check for a lexical error
    if (tokens.size() == 0) {
        return NULL;
    }
    
    // Parse the token stream
    return parse(tokens);
}

ASTNode *
ASTNode::parse(TokenStream tokens) {
    
    int i = 0;
    return parseBOOL(tokens, i);
}

bool
ASTNode::parseToken(TokenStream tokens, Token token, int &i) {
    
    if (tokens[i].first != token)
        return false;
    
    i++;
    return true;
}

ASTNode *
ASTNode::parseDEC(TokenStream tokens, int &i) {
    
    PARSE_PREPARE // [0-9]+
    
    if (tokens[i].first == TOK_DEC) {
        
        uint32_t value;
        std::stringstream ss;
        ss << tokens[i].second;
        ss >> value;
        i++;
        return new ASTNode(AST_DEC, value);
    }
    
    SYNTAX_ERROR
}

ASTNode *
ASTNode::parseHEX(TokenStream tokens, int &i) {
    
    PARSE_PREPARE // [a-f|A-F|0-9]+
    
    if (tokens[i].first == TOK_HEX) {
        
        string digits = tokens[i].second.substr(1);
        uint32_t value;
        std::stringstream ss;
        ss << digits;
        ss >> value;
        i++;
        return new ASTNode(AST_DEC, value);
    }
    
    SYNTAX_ERROR
}

ASTNode *
ASTNode::parseDIRECT(TokenStream tokens, int &i) {
    
    PARSE_PREPARE // <DIRECT> ::= <DEC> | <HEX>
    
    if ((left = parseDEC(tokens, i)))
        return left;
    
    if ((left = parseHEX(tokens, i)))
        return left;
    
    SYNTAX_ERROR
}

ASTNode *
ASTNode::parseREGISTER(TokenStream tokens, int &i) {
    
    PARSE_PREPARE // <REGISTER> ::= ['D0' - 'D7', 'A0' - 'A7']
    
    if (parseToken(tokens, TOK_D0, i)) return new ASTNode(AST_D0);
    if (parseToken(tokens, TOK_D1, i)) return new ASTNode(AST_D1);
    if (parseToken(tokens, TOK_D2, i)) return new ASTNode(AST_D2);
    if (parseToken(tokens, TOK_D3, i)) return new ASTNode(AST_D3);
    if (parseToken(tokens, TOK_D4, i)) return new ASTNode(AST_D4);
    if (parseToken(tokens, TOK_D5, i)) return new ASTNode(AST_D5);
    if (parseToken(tokens, TOK_D6, i)) return new ASTNode(AST_D6);
    if (parseToken(tokens, TOK_D7, i)) return new ASTNode(AST_D7);
    if (parseToken(tokens, TOK_A0, i)) return new ASTNode(AST_A0);
    if (parseToken(tokens, TOK_A1, i)) return new ASTNode(AST_A1);
    if (parseToken(tokens, TOK_A2, i)) return new ASTNode(AST_A2);
    if (parseToken(tokens, TOK_A3, i)) return new ASTNode(AST_A3);
    if (parseToken(tokens, TOK_A4, i)) return new ASTNode(AST_A4);
    if (parseToken(tokens, TOK_A5, i)) return new ASTNode(AST_A5);
    if (parseToken(tokens, TOK_A6, i)) return new ASTNode(AST_A6);
    if (parseToken(tokens, TOK_A7, i)) return new ASTNode(AST_A7);
    
    SYNTAX_ERROR
}

ASTNode *
ASTNode::parseINDIRECT(TokenStream tokens, int &i) {
    
    PARSE_PREPARE // <INDIRECT> ::= '(' <VALUE> ').[bwl]'
    
    if (parseToken(tokens, TOK_LEFT, i))
        if ((left = parseVALUE(tokens, i)))
            if (parseToken(tokens, TOK_RIGHT, i)) {
                if (parseToken(tokens, TOK_B, i))
                    return new ASTNode(AST_IND_B, left);
                if (parseToken(tokens, TOK_W, i))
                    return new ASTNode(AST_IND_W, left);
                if (parseToken(tokens, TOK_L, i))
                    return new ASTNode(AST_IND_L, left);
            }
    
    SYNTAX_ERROR
}

ASTNode *
ASTNode::parseVALUE(TokenStream tokens, int &i) {
    
    // <VALUE> ::= <REGISTER> | <DIRECT> | <INDIRECT>
    PARSE_PREPARE
    
    if ((left = parseREGISTER(tokens, i)))
        return left;
    
    if ((left = parseDIRECT(tokens, i)))
        return left;
    
    if ((left = parseINDIRECT(tokens, i)))
        return left;
    
    SYNTAX_ERROR
}

ASTNode *
ASTNode::parseBOOL(TokenStream tokens, int &i) {
    
    PARSE_PREPARE // <BOOL> ::= <BOOL1> [ '||' <BOOL> ]
    
    if ((left = parseBOOL1(tokens, i))) {
        
        if (!parseToken(tokens, TOK_OR, i))
            return left;
        
        if ((right = parseBOOL(tokens, i)))
            return new ASTNode(AST_OR, left, right);
    }
    
    SYNTAX_ERROR
}

ASTNode *
ASTNode::parseBOOL1(TokenStream tokens, int &i) {
    
    PARSE_PREPARE // <BOOL1> ::= '!' <BOOL> | <BOOL2> [ '&&' <BOOL1> ]
    
    if (parseToken(tokens, TOK_NOT, i)) {
        if ((left = parseBOOL(tokens, i))) {
            return new ASTNode(AST_NOT, left);
        }
        
    } else if ((left = parseBOOL2(tokens, i))) {
        
        if (!parseToken(tokens, TOK_AND, i))
            return left;
        
        if ((right = parseBOOL1(tokens, i))) {
            return new ASTNode(AST_AND, left, right);
        }
    }
    
    SYNTAX_ERROR
}

ASTNode *
ASTNode::parseBOOL2(TokenStream tokens, int &i) {
    
    
    PARSE_PREPARE // <BOOL2> ::= <ATOMIC> | (<BOOL>)
    
    if ((left = parseATOMIC(tokens, i)))
        return left;
    
    if (parseToken(tokens, TOK_LEFT, i))
        if ((left = parseBOOL(tokens, i)))
            if (parseToken(tokens, TOK_RIGHT, i))
                return left;
    
    SYNTAX_ERROR
}

ASTNode *
ASTNode::parseATOMIC(TokenStream tokens, int &i) {
    
    PARSE_PREPARE // <ATOMIC> ::= <VALUE> <COMP> <VALUE>
    
    if ((left = parseVALUE(tokens, i))) {
        
        if (parseToken(tokens, TOK_EQ, i) && (right = parseVALUE(tokens, i)))
            return new ASTNode(AST_EQ, left, right);
        if (parseToken(tokens, TOK_UNEQ, i) && (right = parseVALUE(tokens, i)))
            return new ASTNode(AST_UNEQ, left, right);
        if (parseToken(tokens, TOK_LESSEQ, i) && (right = parseVALUE(tokens, i)))
            return new ASTNode(AST_LESSEQ, left, right);
        if (parseToken(tokens, TOK_LESS, i) && (right = parseVALUE(tokens, i)))
            return new ASTNode(AST_LESS, left, right);
        if (parseToken(tokens, TOK_GREATEREQ, i) && (right = parseVALUE(tokens, i)))
            return new ASTNode(AST_GREATEREQ, left, right);
        if (parseToken(tokens, TOK_GREATER, i) && (right = parseVALUE(tokens, i)))
            return new ASTNode(AST_GREATER, left, right);
    }
    
    SYNTAX_ERROR
}

const char *
Breakpoint::getCondition()
{
    if (ast == NULL)
        return conditionStr.c_str();
    
    return ast->name().c_str();
}

bool
Breakpoint::setCondition(const char *description)
{
    assert(description != NULL);
    
    // Remove old condition (if any)
    removeCondition();
    
    // Remember the original text
    conditionStr = string(description);
    
    // Parse the description
    return ((ast = ASTNode::parse(conditionStr)));
}

bool
Breakpoint::removeCondition()
{
    conditionStr = "";
    if (ast) {
        delete ast;
        ast = NULL;
    }
    return true;
}

bool
Breakpoint::eval()
{
    if (!enabled)
        return false;
    
    if (!ast)
        return true;
    
    return ast->eval();
}
