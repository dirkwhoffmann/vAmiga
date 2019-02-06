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

using reg_itr = std::regex_token_iterator<std::string::iterator>;

// Abstract syntax tree types
enum ASTNodeType {
    
    AST_TRUE,
    AST_BOOL,
    AST_NOT, AST_AND, AST_OR,
    AST_EQUAL, AST_UNEQUAL, AST_LESS, AST_GREATER,
    AST_VALUE,
    AST_INDIRECT,
    AST_D0, AST_D1, AST_D2, AST_D3, AST_D4, AST_D5, AST_D6, AST_D7,
    AST_A0, AST_A1, AST_A2, AST_A3, AST_A4, AST_A5, AST_A6, AST_A7,
    AST_DEC, AST_HEX,
};

class ASTNode {
    
private:
    
    ASTNodeType type = AST_TRUE;
    uint32_t value = 0;
    ASTNode *left = NULL;
    ASTNode *right = NULL;
    
public:
    
    ASTNode(ASTNodeType type, uint32_t value = 0);
    ASTNode(ASTNodeType type, ASTNode *left, ASTNode *right = NULL);
    ~ASTNode();
    
    static ASTNode *parse(string s);
    
    uint32_t eval();
    string name();
    
private:
    
    static ASTNode *parse(vector<string> tokens);
    static bool parseToken(string token, vector<string> tokens, int &i);
    static ASTNode *parseDEC(vector<string> tokens, int &i);
    static ASTNode *parseHEX(vector<string> tokens, int &i);
    static ASTNode *parseDIRECT(vector<string> tokens, int &i);
    static ASTNode *parseREGISTER(vector<string> tokens, int &i);
    static ASTNode *parseD1(vector<string> tokens, int &i);
    static ASTNode *parseA0(vector<string> tokens, int &i);
    static ASTNode *parseA1(vector<string> tokens, int &i);
    static ASTNode *parseINDIRECT(vector<string> tokens, int &i);
    static ASTNode *parseVALUE(vector<string> tokens, int &i);
    static ASTNode *parseBOOL(vector<string> tokens, int &i);
    static ASTNode *parseBOOL1(vector<string> tokens, int &i);
    static ASTNode *parseBOOL2(vector<string> tokens, int &i);
    static ASTNode *parseATOMIC(vector<string> tokens, int &i);
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

std::string label(ASTNodeType type) {
    switch (type) {
        case AST_TRUE:     return "AST_TRUE";
        case AST_NOT:      return "AST_NOT";
        case AST_AND:      return "AST_AND";
        case AST_OR:       return "AST_OR";
        case AST_EQUAL:    return "AST_EQUAL";
        case AST_UNEQUAL:  return "AST_UNEQUAL";
        case AST_LESS:     return "AST_LESS";
        case AST_GREATER:  return "AST_GREATER";
        case AST_INDIRECT: return "AST_INDIRECT";
        case AST_D0:       return "AST_D0";
        case AST_D1:       return "AST_D1";
        case AST_D2:       return "AST_D2";
        case AST_D3:       return "AST_D3";
        case AST_D4:       return "AST_D4";
        case AST_D5:       return "AST_D5";
        case AST_D6:       return "AST_D6";
        case AST_D7:       return "AST_D7";
        case AST_A0:       return "AST_A0";
        case AST_A1:       return "AST_A1";
        case AST_A2:       return "AST_A2";
        case AST_A3:       return "AST_A3";
        case AST_A4:       return "AST_A4";
        case AST_A5:       return "AST_A5";
        case AST_A6:       return "AST_A6";
        case AST_A7:       return "AST_A7";
        case AST_DEC:      return "AST_DEC";
        case AST_HEX:      return "AST_HEX";
        default:
            assert(false);
    }
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
        case AST_TRUE:     return 1;
        case AST_NOT:      return !left->eval();
        case AST_AND:      return left->eval() && right->eval();
        case AST_OR:       return left->eval() || right->eval();
        case AST_EQUAL:    return left->eval() == right->eval();
        case AST_UNEQUAL:  return left->eval() != right->eval();
        case AST_LESS:     return left->eval() <  right->eval();
        case AST_GREATER:  return left->eval() >  right->eval();
        case AST_INDIRECT: return 42;
        case AST_D0:       return 0;
        case AST_D1:       return 1;
        case AST_D2:       return 2;
        case AST_D3:       return 3;
        case AST_D4:       return 4;
        case AST_D5:       return 5;
        case AST_D6:       return 6;
        case AST_D7:       return 7;
        case AST_A0:       return 0;
        case AST_A1:       return 1;
        case AST_A2:       return 2;
        case AST_A3:       return 3;
        case AST_A4:       return 4;
        case AST_A5:       return 5;
        case AST_A6:       return 6;
        case AST_A7:       return 7;
        case AST_DEC:      return value;
        case AST_HEX:      return value;
        default:
            assert(false);
    }
}

std::string
ASTNode::name() {
    
    std::stringstream ss;
    
    bool lpara = (left && (left->type == AST_AND || left->type == AST_OR));
    bool rpara = (right && (right->type == AST_AND || right->type == AST_OR));
    
    switch (type) {
        case AST_TRUE:     return "true";
        case AST_NOT:      return "!(" + left->name() +")";
        case AST_AND:
            return
            (lpara ? "(" : "") + left->name() + (lpara ? ")" : "") + " && " +
            (rpara ? "(" : "") + right->name() + (rpara ? ")" : "");
        case AST_OR:
            return
            (lpara ? "(" : "") + left->name() + (lpara ? ")" : "") + " || " +
            (rpara ? "(" : "") + right->name() + (rpara ? ")" : "");
        case AST_EQUAL:    return left->name() + " == " + right->name();
        case AST_UNEQUAL:  return left->name() + " != " + right->name();
        case AST_LESS:     return left->name() + " < " + right->name();
        case AST_GREATER:  return left->name() + " > " + right->name();
        case AST_INDIRECT: return "(" + left->name() + ")";
        case AST_D0:       return "D0";
        case AST_D1:       return "D1";
        case AST_D2:       return "D2";
        case AST_D3:       return "D3";
        case AST_D4:       return "D4";
        case AST_D5:       return "D5";
        case AST_D6:       return "D6";
        case AST_D7:       return "D7";
        case AST_A0:       return "A0";
        case AST_A1:       return "A1";
        case AST_A2:       return "A2";
        case AST_A3:       return "A3";
        case AST_A4:       return "A4";
        case AST_A5:       return "A5";
        case AST_A6:       return "A6";
        case AST_A7:       return "A7";
        case AST_DEC:      ss << value; return ss.str();
        case AST_HEX:     ss << "$" << std::hex << value; return ss.str();
        default:
            assert(false);
    }
}

ASTNode *
ASTNode::parse(string s) {
    
    // Split into tokens
    std::string rex =
    "(("
    "==|!=|<|>|!|&&|\\|\\||\\(|\\)|"
    "D0|D1|D2|D3|D4|D5|D6|D7|A0|A1|A2|A3|A4|A5|A6|A7|"
    "[\\$]|[a-z|A-Z|0-9]+"
    "))";
    std::vector<std::string> splitted = split(s, rex);
    
    return parse(splitted);
}

ASTNode *
ASTNode::parse(std::vector<std::string> tokens) {
    
    int i = 0;
    return parseBOOL(tokens, i);
}

bool
ASTNode::parseToken(string token, std::vector<std::string> tokens, int &i) {
    
    if (i < tokens.size() && tokens[i] == token) {
        i++;
        return true;
    }
    
    return false;
}


ASTNode *
ASTNode::parseDEC(std::vector<std::string> tokens, int &i) {
    
    // [0-9]+
    PARSE_PREPARE
    
    uint32_t value;
    std::stringstream ss;
    ss << tokens[i];
    ss >> value;
    i++;
    
    if (!ss.fail())
        return new ASTNode(AST_DEC, value);
    
    SYNTAX_ERROR
}

ASTNode *
ASTNode::parseHEX(std::vector<std::string> tokens, int &i) {
    
    // [a-f|A-F|0-9]+
    PARSE_PREPARE
    
    uint32_t value;
    std::stringstream ss;
    ss << std::hex << tokens[i];
    ss >> value;
    i++;
    
    if (!ss.fail())
        return new ASTNode(AST_HEX, value);
    
    SYNTAX_ERROR
}

ASTNode *
ASTNode::parseDIRECT(std::vector<std::string> tokens, int &i) {
    
    // <DIRECT> ::= '$'[a-f|A-F|0-9]+ | [0-9]+
    PARSE_PREPARE
    
    if (parseToken("$", tokens, i)) {
        if ((left = parseHEX(tokens, i)))
            return left;
        
    } else {
        if ((left = parseDEC(tokens, i)))
            return left;
    }
    
    SYNTAX_ERROR
}

ASTNode *
ASTNode::parseREGISTER(std::vector<std::string> tokens, int &i) {
    
    // <REGISTER> ::= ['D0' - 'D7', 'A0' - 'A7']
    if (parseToken("D0", tokens, i)) { return new ASTNode(AST_D0); }
    if (parseToken("D1", tokens, i)) { return new ASTNode(AST_D1); }
    if (parseToken("D2", tokens, i)) { return new ASTNode(AST_D2); }
    if (parseToken("D3", tokens, i)) { return new ASTNode(AST_D3); }
    if (parseToken("D4", tokens, i)) { return new ASTNode(AST_D4); }
    if (parseToken("D5", tokens, i)) { return new ASTNode(AST_D5); }
    if (parseToken("D6", tokens, i)) { return new ASTNode(AST_D6); }
    if (parseToken("D7", tokens, i)) { return new ASTNode(AST_D7); }
    
    if (parseToken("A0", tokens, i)) { return new ASTNode(AST_A0); }
    if (parseToken("A1", tokens, i)) { return new ASTNode(AST_A1); }
    if (parseToken("A2", tokens, i)) { return new ASTNode(AST_A2); }
    if (parseToken("A3", tokens, i)) { return new ASTNode(AST_A3); }
    if (parseToken("A4", tokens, i)) { return new ASTNode(AST_A4); }
    if (parseToken("A5", tokens, i)) { return new ASTNode(AST_A5); }
    if (parseToken("A6", tokens, i)) { return new ASTNode(AST_A6); }
    if (parseToken("A7", tokens, i)) { return new ASTNode(AST_A7); }
    
    return NULL;
}

ASTNode *
ASTNode::parseINDIRECT(std::vector<std::string> tokens, int &i) {
    
    // <INDIRECT> ::= '(' <VALUE> ')'
    PARSE_PREPARE
    
    if (parseToken("(", tokens, i)) {
        if ((left = parseVALUE(tokens, i))) {
            if (parseToken(")", tokens, i)) {
                return new ASTNode(AST_INDIRECT, left);
            }
        }
    }
    
    SYNTAX_ERROR
}

ASTNode *
ASTNode::parseVALUE(std::vector<std::string> tokens, int &i) {
    
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
ASTNode::parseBOOL(std::vector<std::string> tokens, int &i) {
    
    // <BOOL> ::= <BOOL1> [ '||' <BOOL> ]
    PARSE_PREPARE
    
    // <BOOl1>
    if ((left = parseBOOL1(tokens, i))) {
        
        // [ '||' <BOOL> ]
        if (parseToken("||", tokens, i)) {
            if ((right = parseBOOL(tokens, i))) {
                return new ASTNode(AST_OR, left, right);
            }
        }
        return left;
    }
    
    SYNTAX_ERROR
}

ASTNode *
ASTNode::parseBOOL1(std::vector<std::string> tokens, int &i) {
    
    // <BOOL1> ::= '!' <BOOL> | <BOOL2> [ '&&' <BOOL1> ]
    PARSE_PREPARE
    
    if (parseToken("!", tokens, i)) {
        
        // '!' <BOOL>
        if ((left = parseBOOL(tokens, i))) {
            return new ASTNode(AST_NOT, left);
        }
        
    } else {
        
        // <BOOL2>
        if ((left = parseBOOL2(tokens, i))) {
            
            // [ '&&' <BOOL1> ]
            if (parseToken("&&", tokens, i)) {
                if ((right = parseBOOL1(tokens, i))) {
                    return new ASTNode(AST_AND, left, right);
                }
            }
            return left;
        }
    }
    
    SYNTAX_ERROR
}

ASTNode *
ASTNode::parseBOOL2(std::vector<std::string> tokens, int &i) {
    
    // <BOOL2> ::= <ATOMIC> | (<BOOL>)
    PARSE_PREPARE
    
    // <ATOMIC>
    if ((left = parseATOMIC(tokens, i))) {
        return left;
    }
    
    // (<BOOL>)
    if (parseToken("(", tokens, i)) {
        if ((left = parseBOOL(tokens, i))) {
            if (parseToken(")", tokens, i)) {
                return left;
            }
        }
    }
    
    SYNTAX_ERROR
}

ASTNode *
ASTNode::parseATOMIC(std::vector<std::string> tokens, int &i) {
    
    // <ATOMIC> ::= <VALUE> <COMP> <VALUE>
    PARSE_PREPARE
    
    if ((left = parseVALUE(tokens, i))) {
        
        ASTNodeType type = AST_TRUE;
        if (parseToken("==", tokens, i)) type = AST_EQUAL;
        else if (parseToken("!=", tokens, i)) type = AST_UNEQUAL;
        else if (parseToken("<", tokens, i)) type = AST_LESS;
        else if (parseToken(">", tokens, i)) type = AST_GREATER;
        
        if (type != AST_TRUE) {
            if ((right = parseVALUE(tokens, i))) {
                return new ASTNode(type, left, right);
            }
        }
    }
    
    SYNTAX_ERROR
}

Breakpoint::Breakpoint(uint32_t addr, bool soft)
{
    this->addr = addr;
    this->soft = soft;
}

bool
Breakpoint::setCondition(const char *description)
{
    assert(description != NULL);
    
    // Create a C++ string from the provided C string
    string str(description);
    
    // Parse textual description
    condition = ASTNode::parse(str);
    
    // Return true if we got an AST
    return condition != NULL;
}

void
Breakpoint::removeCondition()
{
    if (condition) {
        delete condition;
        condition = NULL;
    }
}

