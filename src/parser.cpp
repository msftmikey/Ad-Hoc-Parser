/**
 *  CPSC 323 Compilers and Languages
 *
 *  Dalton Caron, Teaching Associate
 *  dcaron@fullerton.edu, +1 949-616-2699
 *  Department of Computer Science
 */
#include <parser.h>

Parser::Parser(const token_stream_t &tokens)
: tokens(tokens) {}

Parser::~Parser() {}

ParseTree<std::string> Parser::parse() {
    return this->parseProgram();
}

token_t Parser::getNextToken() {
    // Assumes that the end of the token stream always has
    // the end of file enumeration.
    if (this->current_token == this->tokens.size()) {
        return this->tokens[this->current_token - 1];
    }

    return this->tokens[this->current_token++];
}

token_t Parser::peekNextToken() const {
    if (this->current_token == this->tokens.size()) {
        return this->tokens[this->current_token - 1];
    }
    return this->tokens[this->current_token];
}

// This is the example that was provided in the document.
PTPtr<std::string> Parser::parseProgram() {
    PTPtr<std::string> programNode =
        std::make_shared<PTNode<std::string>>("program");
    programNode->addChild(this->parseBlock());
    this->tryMatchTerminal(this->getNextToken(), PERIOD, programNode);
    return programNode;
}

PTPtr<std::string> Parser::parseBlock() {
    PTPtr<std::string> blockNode = 
        std::make_shared<PTNode<std::string>>("block");
    
    while(peekNextToken().type == CONST_KEYWORD){
        blockNode->addChild(this->parseConstDeclarations());
    }
    while(peekNextToken().type == VAR_KEYWORD){
        blockNode->addChild(this->parseVarDeclarations());
    }
    while(peekNextToken().type == PROCEDURE_KEYWORD){
        blockNode->addChild(this->parseProcedure());
    }
    blockNode->addChild(this->parseStatement());
    return blockNode;
}

PTPtr<std::string> Parser::parseConstDeclarations() {
    PTPtr<std::string> constDeclNode = 
        std::make_shared<PTNode<std::string>>("const_declaration");
    this->tryMatchTerminal(this->getNextToken(), CONST_KEYWORD, constDeclNode);
    this->tryMatchTerminal(this->getNextToken(), IDENTIFIER, constDeclNode);
    this->tryMatchTerminal(this->getNextToken(), EQUALS, constDeclNode);
    this->tryMatchTerminal(this->getNextToken(), NUMBER_LITERAL, constDeclNode);
    constDeclNode->addChild(this->parseConstDeclarationList());
    this->tryMatchTerminal(this->getNextToken(), SEMICOLON, constDeclNode);

     
    return constDeclNode;
}

PTPtr<std::string> Parser::parseConstDeclarationList() {
    PTPtr<std::string> constDeclListNode = 
        std::make_shared<PTNode<std::string>>("const_declaration_list");


    while (peekNextToken().type == COMMA){
        this->tryMatchTerminal(this->getNextToken(), COMMA, constDeclListNode);
        this->tryMatchTerminal(this->getNextToken(), IDENTIFIER, constDeclListNode);
        this->tryMatchTerminal(this->getNextToken(), EQUALS, constDeclListNode);
        this->tryMatchTerminal(this->getNextToken(), NUMBER_LITERAL, constDeclListNode);
    }
    return constDeclListNode;
}

PTPtr<std::string> Parser::parseVarDeclarations() {
    PTPtr<std::string> varDeclNode = 
        std::make_shared<PTNode<std::string>>("var_declaration");
        

    this->tryMatchTerminal(this->getNextToken(), VAR_KEYWORD, varDeclNode);
    this->tryMatchTerminal(this->getNextToken(), IDENTIFIER, varDeclNode);
    varDeclNode->addChild(this->parseVarDeclarationList());
    this->tryMatchTerminal(this->getNextToken(), SEMICOLON, varDeclNode);
    return varDeclNode;
}

PTPtr<std::string> Parser::parseVarDeclarationList() {
    PTPtr<std::string> varDeclNodeList = 
        std::make_shared<PTNode<std::string>>("var_declaration_list");
    while(peekNextToken().type == COMMA){
        this->tryMatchTerminal(this->getNextToken(), COMMA, varDeclNodeList);
        this->tryMatchTerminal(this->getNextToken(), IDENTIFIER, varDeclNodeList);
    }
    return varDeclNodeList;
}

PTPtr<std::string> Parser::parseProcedure() {
    PTPtr<std::string> procedure = 
        std::make_shared<PTNode<std::string>>("procedure");
        

    this->tryMatchTerminal(this->getNextToken(), PROCEDURE_KEYWORD, procedure);
    this->tryMatchTerminal(this->getNextToken(), IDENTIFIER, procedure);
    this->tryMatchTerminal(this->getNextToken(), SEMICOLON, procedure);
    procedure->addChild(this->parseBlock());
    this->tryMatchTerminal(this->getNextToken(), SEMICOLON, procedure);
    return procedure;
}

PTPtr<std::string> Parser::parseStatement() {
    PTPtr<std::string> statementNode = 
        std::make_shared<PTNode<std::string>>("statement");
        

    token_class_t iterativeTokens[7] = {IDENTIFIER, CALL_KEYWORD, READ_OP, WRITE_OP, BEGIN_KEYWORD, IF_KEYWORD, WHILE_KEYWORD};
    bool iterate = false;
    do{
        iterate = false;
        switch(peekNextToken().type){
            case IDENTIFIER:
                this->tryMatchTerminal(this->getNextToken(), IDENTIFIER, statementNode);
                this->tryMatchTerminal(this->getNextToken(), DEFINE_EQUALS, statementNode);
                statementNode->addChild(this->parseExpression());
                break;
            case CALL_KEYWORD:
                this->tryMatchTerminal(this->getNextToken(), CALL_KEYWORD, statementNode);
                this->tryMatchTerminal(this->getNextToken(), IDENTIFIER, statementNode);
                break;
            case READ_OP:
                this->tryMatchTerminal(this->getNextToken(), READ_OP, statementNode);
                this->tryMatchTerminal(this->getNextToken(), IDENTIFIER, statementNode);
                break;
            case WRITE_OP:
                this->tryMatchTerminal(this->getNextToken(), WRITE_OP, statementNode);
                statementNode->addChild(this->parseExpression());
                break;
            case BEGIN_KEYWORD:
                this->tryMatchTerminal(this->getNextToken(), BEGIN_KEYWORD, statementNode);
                statementNode->addChild(this->parseStatement());
                while (peekNextToken().type == SEMICOLON){
                    this->tryMatchTerminal(this->getNextToken(), SEMICOLON, statementNode);
                    statementNode->addChild(this->parseStatement());  
                }
                this->tryMatchTerminal(this->getNextToken(), END_KEYWORD, statementNode);
                break;
            case IF_KEYWORD:
                this->tryMatchTerminal(this->getNextToken(), IF_KEYWORD, statementNode);
                statementNode->addChild(this->parseCondition());
                this->tryMatchTerminal(this->getNextToken(), THEN_KEYWORD, statementNode);
                statementNode->addChild(this->parseStatement());
                break;
            case WHILE_KEYWORD:
                this->tryMatchTerminal(this->getNextToken(), WHILE_KEYWORD, statementNode);
                statementNode->addChild(this->parseCondition());
                this->tryMatchTerminal(this->getNextToken(), DO_KEYWORD, statementNode);
                statementNode->addChild(this->parseStatement());
                break;
            default:
                break;
        }
        for (int i = 0; i < 7; ++i){
            if (peekNextToken().type == iterativeTokens[i]){
                iterate = true;
                break;
            }
        }
    }while(iterate);
    return statementNode;
}

PTPtr<std::string> Parser::parseCondition() {
    PTPtr<std::string> conditionNode = 
        std::make_shared<PTNode<std::string>>("condition");
    if(peekNextToken().type == ODD_OP){
        this->tryMatchTerminal(this->getNextToken(), ODD_OP, conditionNode);
        conditionNode->addChild(this->parseExpression());
    }
    else{
        conditionNode->addChild(this->parseExpression());
        switch(peekNextToken().type){
            case EQUALS:
                this->tryMatchTerminal(this->getNextToken(), EQUALS, conditionNode);
                break;
            case COMPARE_OP:
                this->tryMatchTerminal(this->getNextToken(), COMPARE_OP, conditionNode);
                break;
            default:
                break;
                break;
        }
        conditionNode->addChild(this->parseExpression());
    }
    return conditionNode;
}

PTPtr<std::string> Parser::parseExpression() {
    PTPtr<std::string> expressionNode = 
        std::make_shared<PTNode<std::string>>("expression");
        
    while(peekNextToken().type == ADD_OP){
        std::cout << "ADD_OP" << std::endl;
        this->tryMatchTerminal(this->getNextToken(), ADD_OP, expressionNode);
    }
    expressionNode->addChild(this->parseTerm());
    while(peekNextToken().type == ADD_OP){
        this->tryMatchTerminal(this->getNextToken(), ADD_OP, expressionNode);
        expressionNode->addChild(this->parseTerm());
    }
    return expressionNode;
}

PTPtr<std::string> Parser::parseTerm() {
    PTPtr<std::string> termNode = std::make_shared<PTNode<std::string>>("term");
    

    termNode->addChild(this->parseFactor());
    while(peekNextToken().type == MUL_OP){
        this->tryMatchTerminal(this->getNextToken(), MUL_OP, termNode);
        termNode->addChild(this->parseFactor());
    }
    return termNode;
}

PTPtr<std::string> Parser::parseFactor() {
    PTPtr<std::string> factorNode = 
        std::make_shared<PTNode<std::string>>("factor");

    if (peekNextToken().type == IDENTIFIER){
        this->tryMatchTerminal(this->getNextToken(), IDENTIFIER, factorNode);
    }  
    else if (peekNextToken().type == NUMBER_LITERAL){
        this->tryMatchTerminal(this->getNextToken(), NUMBER_LITERAL, factorNode);
    }
    else if (peekNextToken().type == LEFT_PAREN){
        this->tryMatchTerminal(this->getNextToken(), LEFT_PAREN, factorNode);
        factorNode->addChild(this->parseExpression());
        this->tryMatchTerminal(this->getNextToken(), RIGHT_PAREN, factorNode);
    }
    return factorNode;
}

void Parser::tryMatchTerminal(
    const token_t &actual,
    const token_class_t expected,
    PTPtr<std::string> node
) const {
    if (actual.type != expected) {
        raiseMismatchError(actual, expected);
    }
    node->addChild(actual.lexeme);
}

void Parser::tryMatchTerminal(
    const token_t &actual,
    const std::initializer_list<token_class_t> expected,
    PTPtr<std::string> node
) const {
    for (const token_class_t type : expected) {
        if (type == actual.type) {
            node->addChild(actual.lexeme);
            return;
        }
    }
    raiseMismatchError(actual, expected);
}

void Parser::raiseMismatchError(
        const token_t &actual,
        const token_class_t expected
) const {
    ERROR_LOG(
        "expected %s got %s "
        "at %s line %d column %d",
        tokenTypeToString(expected).c_str(),
        actual.lexeme.c_str(),
        actual.file.c_str(),
        actual.line,
        actual.column
    );

    exit(EXIT_FAILURE);
}

void Parser::raiseMismatchError(
        const token_t &actual,
        const std::initializer_list<token_class_t> expected
) const {
    ERROR_LOG("expected ");

    for (auto token : expected) {
        (void) ERROR_LOG(
            "%s ",
            tokenTypeToString(token).c_str()
        );
    }

    ERROR_LOG(
        "got %s at %s line %d column %d",
        actual.lexeme.c_str(),
        actual.file.c_str(),
        actual.line,
        actual.column
    );

    exit(EXIT_FAILURE);
}
