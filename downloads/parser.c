/*
Teammate 1: Nicolas Polanco
Email: ni813729@ucf.edu
Teammate 2: Sharon Pullukara
Email: sh091109@ucf.edu
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>


#define MAX_NUM_LEN 5
#define MAX_ID_LEN 11
#define MAX_LEXEMES 1000
#define MAX_SOURCE_SIZE 10000
#define MAX_SYMBOL_TABLE_SIZE 500

// Token types
typedef enum {
    oddsym = 1, identsym, numbersym, plussym, minussym,
    multsym, slashsym, fisym, eqlsym, neqsym, lessym, leqsym,
    gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
    periodsym, becomessym, beginsym, endsym, ifsym, thensym, whilesym,
    dosym, constsym, varsym, writesym, readsym, modsym, callsym,
    name_toolong, num_toolong, invalid_sym, unclosed_comment, proceduresym
} TokenType;

// Lexeme structure
typedef struct {
    char lexeme[100];
    TokenType token;
    int val;
} Lexeme;

typedef struct {
    int kind; // const = 1, var = 2, proc = 3
    char name[10]; // name up to 11 chars
    int val; // number (ASCII value)
    int level; // L level
    int addr; // M address
    int mark; // to indicate unavailable or deleted
} Symbol;

typedef struct {
    char op[4]; //operation code of the current instruction
    int l; //lexical level of the current instruction
    int m; //argument for the current instruction
} Instruction;

int pas[MAX_SYMBOL_TABLE_SIZE]; //array used to store program's instructions and data
int pc = 10; //pc keeps track of the current instruction to be fetched and executed in the program
Instruction ir[MAX_ID_LEN] = {0,0,0}; //ir struct represents current instruction being executed

Symbol symbols[MAX_SYMBOL_TABLE_SIZE];
int symbolTableIndex = 0;

// Array to store lexemes
Lexeme lexemes[MAX_LEXEMES];
int lexemeCount = 0;

int currentCodeIndex = 0; // Current index in the generated code
int codeIndex = 0;
int token;
char identName[11];
int numberValue;
char code[500][100]; // Array to store generated code

FILE *inputFile, *outputFile;

//Function Prototypes
void addLexeme(const char* lex, TokenType token);
void printSourceCode(const char* sourceCode);
void printLexemeTable();
TokenType getTokenType(const char* lex);
void lex(const char* sourceCode);
void SYMBOLTABLECHECK(char *ident, int *symIdx);
void printSymbolTable();
void error(int n, FILE *outputFile);
int base(int BP, int L);
void emit(char *op, int l, int m);
void print_stack(int bp, int sp, int counter, FILE *outputFile);
void PROGRAM(FILE *inputFile, FILE *outputFile);
void PROCEDURE_DECLARATION(FILE *inputFile, FILE *outputFile);
void BLOCK(FILE *inputFile, FILE *outputFile);
void CONST_DECLARATION(FILE *inputFile, FILE *outputFile);
int VAR_DECLARATION(FILE *inputFile, FILE *outputFile);
void STATEMENT(FILE *inputFile, FILE *outputFile, int level);
void CONDITION(FILE *inputFile, FILE *outputFile, int *i);
void EXPRESSION(FILE *inputFile, FILE *outputFile, int *i);
void add_to_symbol_table(int kind, char *identName, int value, int level, int addr, int mark);
void TERM(FILE *inputFile, FILE *outputFile, int *i);
void FACTOR(FILE *inputFile, FILE *outputFile, int *i);

// Main function to read source code from a file and perform lexical analysis
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    inputFile = fopen(argv[1], "r");
    outputFile = fopen("output.txt", "w");
    if (!inputFile) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    if(outputFile == NULL) {
        printf("Can't write to output file");
        return EXIT_FAILURE;
    }

    char sourceCode[MAX_SOURCE_SIZE];
    size_t length = fread(sourceCode, 1, MAX_SOURCE_SIZE - 1, inputFile);
    sourceCode[length] = '\0';
    fclose(inputFile);

    printSourceCode(sourceCode);


    lex(sourceCode);

    PROGRAM(inputFile, outputFile);

    // Print a newline character to properly terminate the output
    fprintf(outputFile, "\n");
    printf("\n");

    // Close the output file
    fclose(outputFile);

    return EXIT_SUCCESS;
}

void add_to_symbol_table(int kind, char *identName, int value, int level, int addr,int mark){
    Symbol symbol;
    symbol.kind = kind;
    strcpy(symbol.name, identName);
    symbol.val = value;
    symbol.level = level;
    symbol.addr = addr;
    symbol.mark = mark;
    symbols[symbolTableIndex++] = symbol;


}

void emit(char *op, int l, int m){
    strcpy(ir[currentCodeIndex].op, op);
    ir[currentCodeIndex].l = l;
    ir[currentCodeIndex].m = m;
    currentCodeIndex++;
}

void printCode() {

    for (int i = 0; i < currentCodeIndex-1; i++) {
        printf("%s\t%d\t%d\n", ir[i].op, ir[i].l, ir[i].m);
        fprintf(outputFile,"%s\t%d\t%d\n", ir[i].op, ir[i].l, ir[i].m);
    }
    printf("\n");
    fprintf(outputFile,"\n");

}

int getOpNum(int i) {
    if (strcmp(ir[i].op, "LIT") == 0)
        return 1;
    if (strcmp(ir[i].op, "OPR") == 0)
        return 2;
    if (strcmp(ir[i].op, "LOD") == 0)
        return 3;
    if (strcmp(ir[i].op, "STO") == 0)
        return 4;
    if (strcmp(ir[i].op, "CAL") == 0)
        return 5;
    if (strcmp(ir[i].op, "INC") == 0)
        return 6;
    if (strcmp(ir[i].op, "JMP") == 0)
        return 7;
    if (strcmp(ir[i].op, "JPC") == 0)
        return 8;
    if (strcmp(ir[i].op, "SYS") == 0)
        return 9;
    return 0;  // Default case, if no match found
}


void printCode2() {
    FILE *newOutputFile;

    // Open the new file for writing
    newOutputFile = fopen("ELF.txt", "w");
    if (newOutputFile == NULL) {
        perror("Error opening file");
        return; // or handle the error as needed
    }


    fprintf(newOutputFile, "7 0 3\n");

    for (int i = 0; i < currentCodeIndex-1; i++) {
        int tokenNumber = getOpNum(i);
        fprintf(newOutputFile, "%d %d %d\n", tokenNumber, ir[i].l, ir[i].m);
    }


    fprintf(newOutputFile, "\n");

    // Close the file
    fclose(newOutputFile);
}

// Function to add a lexeme to the lexeme array
void addLexeme(const char* lex, TokenType token) {
    if (lexemeCount < MAX_LEXEMES) {
        strncpy(lexemes[lexemeCount].lexeme, lex, sizeof(lexemes[lexemeCount].lexeme) - 1);
        lexemes[lexemeCount].lexeme[sizeof(lexemes[lexemeCount].lexeme) - 1] = '\0';
        lexemes[lexemeCount].token = token;
        lexemeCount++;
    }
}

// Function to print the source code
void printSourceCode(const char* sourceCode) {
    fprintf(outputFile, "Source Program:\n%s\n", sourceCode);
    printf("Source Program:\n%s\n", sourceCode);
}

// Function to print the lexeme table
void printLexemeTable() {
    // Determine the maximum width needed for the lexeme column
    int maxLexemeLength = 0;
    for (int i = 0; i < lexemeCount; i++) {
        int len = strlen(lexemes[i].lexeme);
        if (len > maxLexemeLength) {
            maxLexemeLength = len;
        }
    }

    for (int i = 0; i < lexemeCount; i++) {
        if (lexemes[i].token == name_toolong) {
            error(26, outputFile);
            exit(1);
        }
        else if (lexemes[i].token == num_toolong) {
            error(25, outputFile);
            exit(1);
        }
        else if (lexemes[i].token == invalid_sym) {
            error(27, outputFile);
            exit(1);
        }
        else if (lexemes[i].token == unclosed_comment) {
            error(32, outputFile);
            exit(1);
        }
       
    }
}

TokenType getTokenType(const char *lex) {
    if (strcmp(lex, "const") == 0) return constsym;
    if (strcmp(lex, "procedure") == 0) return proceduresym;
    if (strcmp(lex, "var") == 0) return varsym;
    if (strcmp(lex, "begin") == 0) return beginsym;
    if (strcmp(lex, "end") == 0) return endsym;
    if (strcmp(lex, "if") == 0) return ifsym;
    if (strcmp(lex, "then") == 0) return thensym;
    if (strcmp(lex, "while") == 0) return whilesym;
    if (strcmp(lex, "call") == 0) return callsym;
    if (strcmp(lex, "do") == 0) return dosym;
    if (strcmp(lex, "read") == 0) return readsym;
    if (strcmp(lex, "write") == 0) return writesym;
    if (strcmp(lex, "fi") == 0) return fisym;
    if (strcmp(lex, "+") == 0) return plussym;
    if (strcmp(lex, "-") == 0) return minussym;
    if (strcmp(lex, "*") == 0) return multsym;
    if (strcmp(lex, "/") == 0) return slashsym;
    if (strcmp(lex, ":=") == 0) return becomessym;
    if (strcmp(lex, "=") == 0) return eqlsym;
    if (strcmp(lex, "<>") == 0) return neqsym;
    if (strcmp(lex, "<") == 0) return lessym;
    if (strcmp(lex, "<=") == 0) return leqsym;
    if (strcmp(lex, ">") == 0) return gtrsym;
    if (strcmp(lex, ">=") == 0) return geqsym;
    if (strcmp(lex, "(") == 0) return lparentsym;
    if (strcmp(lex, ")") == 0) return rparentsym;
    if (strcmp(lex, ",") == 0) return commasym;
    if (strcmp(lex, ";") == 0) return semicolonsym;
    if (strcmp(lex, ".") == 0) return periodsym;
    if (strcmp(lex, "%") == 0 ) return modsym;
    return identsym; // Default to identifier
}



// Function to perform lexical analysis on the source code
void lex(const char* sourceCode) {
    char buffer[ MAX_LEXEMES];  // Buffer to hold the current lexeme
    int bufferIndex = 0;  // Index for the buffer
    long length = strlen(sourceCode);  // Length of the source code
    int inComment = 0;  // Flag to indicate if inside a comment

    for (int i = 0; i < length; i++) {
        char ch = sourceCode[i];

        // Check for the start of a comment
        if (ch == '/' && sourceCode[i + 1] == '*') {
            inComment = 1;
            i++;  // Skip the next character ('*')
            continue;
        }

        // Check for the end of a comment
        if (inComment && ch == '*' && sourceCode[i + 1] == '/') {
            inComment = 0;
            i++;  // Skip the next character ('/')
            continue;
        }

        if (inComment) continue;  // Skip characters inside comments
        if (isspace(ch)) continue;  // Skip whitespace

        // Handle identifiers (alphanumeric starting with a letter)
        if (isalpha(ch)) {
            bufferIndex = 0;
            while (isalnum(sourceCode[i]) && bufferIndex < sizeof(buffer) - 1) {
                buffer[bufferIndex++] = sourceCode[i++];
            }
            buffer[bufferIndex] = '\0';
            i--;

            if (bufferIndex > 11) {
                addLexeme(buffer, name_toolong);
            } else {
                addLexeme(buffer, getTokenType(buffer));
            }
        }
        // Handle numbers (digits)
        else if (isdigit(ch)) {
            bufferIndex = 0;
            while (isdigit(sourceCode[i]) && bufferIndex < sizeof(buffer) - 1) {
                buffer[bufferIndex++] = sourceCode[i++];
            }
            buffer[bufferIndex] = '\0';
            i--;

            if (bufferIndex > 5) {
                addLexeme(buffer, num_toolong);
            } else {
                addLexeme(buffer, numbersym);
            }
        }
        // Handle the ":=" operator
        else if (ch == ':' && sourceCode[i + 1] == '=') {
            addLexeme(":=", becomessym);
            i++;
        }
        // Handle the "<=", ">=", and "<>" operators
        else if ((ch == '<' && sourceCode[i + 1] == '=') || (ch == '>' && sourceCode[i + 1] == '=')) {
            buffer[0] = ch;
            buffer[1] = '=';
            buffer[2] = '\0';
            addLexeme(buffer, getTokenType(buffer));
            i++;
        }
        else if (ch == '<' && sourceCode[i + 1] == '>') {
            buffer[0] = '<';
            buffer[1] = '>';
            buffer[2] = '\0';
            addLexeme(buffer, neqsym);
            i++;
        }
        // Handle single-character tokens
        else {
            buffer[0] = ch;
            buffer[1] = '\0';
            TokenType token = getTokenType(buffer);
            if (token == identsym) {
                addLexeme(buffer, invalid_sym);
            } else {
                addLexeme(buffer, token);
            }
        }
    }

    // Check if we exited the loop while still inside a comment
    if (inComment) {
        addLexeme("/*", unclosed_comment);
    }
}
void SYMBOLTABLECHECK(char *ident, int *symIdx) {
    // Linear search through the symbol table
    for (int i = 0; i < symbolTableIndex; ++i) {
        if (strcmp(symbols[i].name, ident) == 0) {
            // Symbol found, set symIdx to the index and return
            *symIdx = i;
            return;
        }
    }

    // Symbol not found, set symIdx to -1
    *symIdx = -1;
}


// Function to find the base address of the current activation record
int base(int BP, int L) {
    int arb = BP;
    while(L>0) {
        arb = pas[arb];
        L--;
    }
    return arb;
}

// Function to print the stack contents recursively
void print_stack(int bp, int sp, int counter, FILE *outputFile) {
    if(&pas[bp] >= &pas[counter]){
        print_stack(pas[bp], bp+1, counter, outputFile);
        if(pas[bp] != 0) {
            printf("| ");
            fprintf(outputFile, "| ");
        }
    }

    for (int i = bp; i >= sp; i--) {
        printf("%d ", pas[i]);
        fprintf(outputFile, "%d ", pas[i]);
    }

}

void PROGRAM(FILE *inputFile, FILE *outputFile) {
    codeIndex++;
    BLOCK(inputFile, outputFile);


    // Check for end of program '.'
    if (lexemes[lexemeCount - 1].token == periodsym) {
        //emit("HLT", 0, 2); // Emit HALT instruction
       // printf("%d\t%s\t%d\t%d\n", currentCodeIndex++, "HLT", 0, 0);
       // fprintf(outputFile, "%d\t%s\t%d\t%d\n", currentCodeIndex++, "HLT", 0, 0);
        currentCodeIndex++;
        error(0, outputFile);
    } else {
        error(9, outputFile);
        // fprintf(outputFile, "Error: Missing '.' at the end of the program\n");
        // printf("Error: Missing '.' at the end of the program\n");

        exit(EXIT_FAILURE);
    }


    printf("\n\nAssembly Code:\n");
    fprintf(outputFile, "Assembly Code:\n");

    printf("OP\tL\tM\n");
    fprintf(outputFile, "OP\tL\tM\n");

    // Emit code generation (JMP 0 3)
    printf("JMP\t0\t3\n");
    fprintf(outputFile, "JMP\t0\t3\n");

    FILE *outputFile2 = fopen("ELF.txt", "w");
    printCode();
   // printSymbolTable();
    printCode2();
}

void PROCEDURE_DECLARATION(FILE *inputFile, FILE *outputFile) {
    int i = 0;
    while (i < lexemeCount) {
        if (lexemes[i].token == proceduresym) {
            i++;
            if (i >= lexemeCount || lexemes[i].token != identsym) {
                error(4, outputFile);
                exit(1);
            }

            int symIdx;
            SYMBOLTABLECHECK(lexemes[i].lexeme, &symIdx);
            if (symIdx != -1) {
                // Error handling for redeclared constant
                error(14, outputFile);
                exit(1);
            } else {
                strcpy(symbols[symbolTableIndex].name, lexemes[i].lexeme);
                symbols[symbolTableIndex].kind = 3; // proc
                symbolTableIndex++;
            }

            i++;
            if (i >= lexemeCount || lexemes[i].token != semicolonsym) {
                error(5, outputFile);
                exit(1);
            }

            i++; // Move past ';'

            // Ensure no assignments to procedures
            if (symIdx != -1 && symbols[symIdx].kind == 3) {
                error(21, outputFile);
                exit(1);
            }

            // Check for assignment to constant or procedure within the procedure block
            int blockStartIndex = i;
            while (i < lexemeCount && lexemes[i].token != semicolonsym) {
                if (lexemes[i].token == identsym) {
                    int assignSymIdx;
                    SYMBOLTABLECHECK(lexemes[i].lexeme, &assignSymIdx);
                    if (assignSymIdx != -1 && (symbols[assignSymIdx].kind == 1 || symbols[assignSymIdx].kind == 3)) {
                        error(12, outputFile);
                        exit(1);
                    } else {
                        error(21, outputFile);
                        exit(1);
                    }
                }
                i++;
            }

            // Handle the block
            i = blockStartIndex;
            BLOCK(inputFile, outputFile);
            
            if (i >= lexemeCount || lexemes[i].token != semicolonsym) {
                error(5, outputFile);
                exit(1);
            }

            i++; // Move past ';'
        } else {
            i++;
        }
    }
}


// Function to handle the BLOCK production rule
void BLOCK(FILE *inputFile, FILE *outputFile) {
    printLexemeTable();

    CONST_DECLARATION(inputFile, outputFile);

    int numVars = VAR_DECLARATION(inputFile, outputFile);

    PROCEDURE_DECLARATION(inputFile, outputFile);

    // emit("INC", 0, 3 + numVars); // INC (M = 3 + numVars)

    STATEMENT(inputFile, outputFile, 0);
}

void checkValue(const char *lexeme)
{
    // Check if the lexeme starts with a digit and is not empty
    if (isdigit(lexeme[0])) {
        // Check if the length of the lexeme exceeds 5 digits
        if (strlen(lexeme) > 5) {
            error(25, outputFile);
            exit(1);
        }
    }

    if (isalpha(lexeme[0])) {
        // Check if the length of the lexeme exceeds 5 digits
        if (strlen(lexeme) > 11) {
            error(26, outputFile);
            exit(1);

        }
    }


}

void CONST_DECLARATION(FILE *inputFile, FILE *outputFile) {
    int symIdx;
    int i = 0; // Iterator for lexemes array

    if (lexemes[i].token == constsym) {

        i++; // Move to the next token

        while (1) {
            if (lexemes[i].token == identsym) {

                SYMBOLTABLECHECK(lexemes[i].lexeme, &symIdx);

                if (symIdx != -1) {
                    // Error handling for redeclared constant
                    error(14, outputFile);
                    exit(1);
                } else {

                    strcpy(symbols[symbolTableIndex].name, lexemes[i].lexeme);
                    symbols[symbolTableIndex].kind = 1; // const

                    i++; // Move to the next token


                    if (lexemes[i].token == eqlsym) {

                        i++; // Move to the next token


                        if (lexemes[i].token == numbersym) {

                            symbols[symbolTableIndex].val = atoi(lexemes[i].lexeme);
                            symbols[symbolTableIndex].level = 0; // Level is usually 0 for constant
                            symbols[symbolTableIndex].addr = 0;  // Not applicable for constants
                            symbols[symbolTableIndex].mark = 1;
                            symbolTableIndex++;
                        } else {
                            // Error handling for non-integer constant value
                            error(29, outputFile);
                            exit(1);
                        }

                        i++; // Move to the next token

                    } else {
                        // Error handling for missing assignment symbol
                        error(3, outputFile);
                        exit(1);
                    }
                }

            } else {
                // Error handling for missing identifier in const declaration
                error(4, outputFile);
                exit(1);

            }

            // Check for comma to continue with the next constant declaration or semicolon to end
            if (lexemes[i].token == commasym) {

                i++; // Move to the next token
            } else if (lexemes[i].token == semicolonsym) {

                i++; // Move to the next token
                break; // Exit the loop
            } else {
                error(5, outputFile);
                exit(1);
            }
        }
    }
}

int VAR_DECLARATION(FILE *inputFile, FILE *outputFile) {
    int symIdx;
    int numVars = 0;

    for (int i = 0; i < lexemeCount; i++) {
       // printf("Token: %d, Lexeme: %s\n", lexemes[i].token, lexemes[i].lexeme);

        if (lexemes[i].token == varsym) {
           // printf("Found 'var'\n");

            do {
                i++; // Move to the next token
                if (i >= lexemeCount) {
                    fprintf(outputFile, "Error: Unexpected end of input.\n");
                    printf("Error: Unexpected end of input.\n");
                    return numVars;
                }
                checkValue(lexemes[i].lexeme);

                if (lexemes[i].token != identsym) {
                    error(4, outputFile);
                    // fprintf(outputFile, "Error: Expected identifier.\n");
                    // printf("Error: Expected identifier.\n");
                     exit(1);
                    return numVars;
                }

                SYMBOLTABLECHECK(lexemes[i].lexeme, &symIdx);
                if (symIdx != -1) {
                    error(30, outputFile);
                    // fprintf(outputFile, "Error: Identifier '%s' already declared.\n", lexemes[i].lexeme);
                    // printf("Error: Identifier '%s' already declared.\n", lexemes[i].lexeme);
                    exit(1);
                    return numVars;
                }

                numVars++;
                add_to_symbol_table(2, lexemes[i].lexeme, 0, 0, numVars + 2,1); // Add variable to symbol table

                i++; // Move to the next token
                if (i >= lexemeCount) {
                    fprintf(outputFile, "Error: Unexpected end of input.\n");
                    printf("Error: Unexpected end of input.\n");
                    return numVars;
                }


            } while (lexemes[i].token == commasym);


            emit("INC", 0, 3 + numVars);
            STATEMENT(inputFile, outputFile, i+1);

            if (lexemes[i].token != semicolonsym) {
                error(5, outputFile);
                // fprintf(outputFile, "Error: Expected ';' after variable declaration.\n");
                // printf("Error: Expected ';' after variable declaration.\n");
                 exit(1);
                return numVars;
            }


        }
    }
    return numVars;

}

void STATEMENT(FILE *inputFile, FILE *outputFile, int level) {
    int symIdx;
    int i = level; // Iterator for lexemes array
    
    if (lexemes[i].token == identsym) {
        SYMBOLTABLECHECK(lexemes[i].lexeme, &symIdx);
        if (symIdx == -1) {
            error(11, outputFile);
            exit(1);
        }  if (symbols[symIdx].kind == 1 || symbols[symIdx].kind == 3) { // Assignment to constant or procedure is not allowed
            error(12, outputFile);
            exit(1);
        }
        if (symbols[symIdx].kind != 2) { // Not a variable
            fprintf(outputFile, "Error: '%s' is not a variable.\n", lexemes[i].lexeme);
            printf( "Error: '%s' is not a variable.\n", lexemes[i].lexeme);
            
            exit(1);
        }
        i++;
        if (i < lexemeCount && lexemes[i].token != becomessym) {
            //error()
            fprintf(outputFile, "Error: Expected ':=' after identifier.\n");
            printf( "Error: Expected ':=' after identifier.\n");
            
            exit(1);
        }
        
        i++; // Move past ':='
        
        EXPRESSION(inputFile, outputFile, &i);
        emit("STO", 0, symbols[symIdx].addr);
        STATEMENT(inputFile, outputFile, i);
        
    } else if (lexemes[i].token == callsym) {
        i++;
        if (i >= lexemeCount || lexemes[i].token != identsym) {
            error(2, outputFile);
            exit(1);
        }
        SYMBOLTABLECHECK(lexemes[i].lexeme, &symIdx);
        
        if (symIdx == -1) {
            error(11, outputFile);
            exit(1);
        }
        if (symbols[symIdx].kind != 3) { // Not a procedure
            fprintf(outputFile, "Error: '%s' is not a procedure.\n", lexemes[currentCodeIndex].lexeme);
            printf("Error: '%s' is not a procedure.\n", lexemes[currentCodeIndex].lexeme);
            exit(1);
        }
        i++;
        emit("CAL", 0, symbols[symIdx].addr); // Assuming 0 is the current level, adjust as necessary
        
    } else if (lexemes[i].token == beginsym) {
        do {
            i++;
            STATEMENT(inputFile, outputFile, i);
            while (i < lexemeCount && lexemes[i].token != endsym) {
                i++;
            }
        } while (lexemes[i].token == semicolonsym);

        if (i >= lexemeCount || lexemes[i].token != endsym) {
            error(17, outputFile);
             exit(1);
        }

        i++; // Move past 'end'

    } else if (lexemes[i].token == ifsym) {
        i++;
        CONDITION(inputFile, outputFile, &i);

        int jpcIdx = codeIndex;
        emit("JPC", 0, 0);
        while (i < lexemeCount && lexemes[i].token != thensym) {
            i++;
        }

        if (i >= lexemeCount || lexemes[i].token != thensym) {
            error(16, outputFile);
             exit(1);
        } else {
            i++; // Move past 'then'
        }

        STATEMENT(inputFile, outputFile, i);
        ir[jpcIdx].m = codeIndex; // Update JPC address

    } else if (lexemes[i].token == whilesym) {
        i++;
        int loopIdx = codeIndex;
        CONDITION(inputFile, outputFile, &i);
        if (i >= lexemeCount || lexemes[i].token != dosym) {
            error(18, outputFile);
             exit(1);
        }
        if (i < lexemeCount) {
            i++; // Move past 'do'
        }
        int jpcIdx = codeIndex;
        emit("JPC", 0, 0);
        STATEMENT(inputFile, outputFile, i);
        emit("JMP", 0, loopIdx);
        ir[jpcIdx].m = codeIndex; // Update JPC address

    } else if (lexemes[i].token == readsym) {
        i++;
        if (i >= lexemeCount || lexemes[i].token != identsym) {
            fprintf(outputFile, "Error: Expected identifier after 'read'.\n");
            printf("Error: Expected identifier after 'read'.\n");

            exit(1);
        }
        SYMBOLTABLECHECK(lexemes[i].lexeme, &symIdx);
        if (symIdx == -1) {
            error(27, outputFile);
            // fprintf(outputFile, "Error: Undefined symbol '%s'.\n", lexemes[i].lexeme);
            // printf( "Error: Undefined symbol '%s'.\n", lexemes[i].lexeme);

             exit(1);
        }
        if (symbols[symIdx].kind != 2) { // Not a variable
            fprintf(outputFile, "Error: '%s' is not a variable.\n", lexemes[i].lexeme);
            printf( "Error: '%s' is not a variable.\n", lexemes[i].lexeme);

            exit(1);
        }
        i++;
        emit("SYS", 0, 2);
        emit("STO", 0, symbols[symIdx].addr);
    } else if (lexemes[i].token == writesym) {
        i++;
        EXPRESSION(inputFile, outputFile, &i);
        emit("SYS", 0, 1);
    }else if (lexemes[i].token == callsym) {
        i++;
        if (i >= lexemeCount || lexemes[i].token != identsym) {
            error(14, outputFile); // Call must be followed by an identifier.
            exit(1);
        }
        SYMBOLTABLECHECK(lexemes[i].lexeme, &symIdx);
        if (symIdx == -1) {
            error(11, outputFile); // Undeclared identifier.
            exit(1);

        }
        if (symbols[symIdx].kind != 3) { // Not a procedure
            error(15, outputFile); // Call of a constant or variable is meaningless.
            exit(1);

        }
        emit("CAL", 0, symbols[symIdx].addr);
        i++;

    }  else if (lexemes[i].token != semicolonsym && lexemes[i].token != periodsym) {
        emit("SYS", 0, 3);
    }

    // Check for semicolon to move to the next statement
    if (i < lexemeCount && lexemes[i].token == semicolonsym) {
        i++;
    }
}

void CONDITION(FILE *inputFile, FILE *outputFile, int *i) {


    if (lexemes[*i].token == oddsym) {
       // FACTOR(inputFile, outputFile, i); // Handle 'odd' condition
        emit("ODD", 0, 11); // Example: emit ODD operation
    } else {
        EXPRESSION(inputFile, outputFile, i);

        switch (lexemes[*i].token) {
            case eqlsym:

                (*i)++;
                EXPRESSION(inputFile, outputFile, i);
                emit("OPR", 0, 5); // Example: emit EQL operation
                break;
            case neqsym:

                (*i)++;
                EXPRESSION(inputFile, outputFile, i);
                emit("OPR", 0, 6); // Example: emit NEQ operation
                break;
            case lessym:
                (*i)++;
                EXPRESSION(inputFile, outputFile, i);
                emit("OPR", 0, 7); // Example: emit LSS operation
                break;
            case leqsym:
                (*i)++;
                EXPRESSION(inputFile, outputFile, i);
                emit("OPR", 0, 8); // Example: emit LEQ operation
                break;
            case gtrsym:
                (*i)++;
                EXPRESSION(inputFile, outputFile, i);
                emit("OPR", 0, 9); // Example: emit GTR operation
                break;
            case geqsym:
                (*i)++;
                EXPRESSION(inputFile, outputFile, i);
                emit("OPR", 0, 10); // Example: emit GEQ operation
                break;
            default:
                error(20, outputFile);
                // fprintf(outputFile, "Error: Comparison operator expected\n");
                // printf("Error: Comparison operator expected\n");

               exit(EXIT_FAILURE);
        }
    }

}

void EXPRESSION(FILE *inputFile, FILE *outputFile, int *i) {

    if (lexemes[*i].token == minussym) {
        (*i)++;
        TERM(inputFile, outputFile, i);
        emit("NEG", 0, 1); // NEG
        while (lexemes[*i].token == plussym || lexemes[*i].token == minussym) {
            if (lexemes[*i].token == plussym) {
                (*i)++;
                TERM(inputFile, outputFile, i);
                emit("OPR", 0, 1); // ADD
            } else {
                (*i)++;
                TERM(inputFile, outputFile, i);
               emit("OPR", 0, 2); // SUB
            }
        }
    } else {
        if (lexemes[*i].token == plussym) {
            (*i)++;
        }
        TERM(inputFile, outputFile, i);
        while (lexemes[*i].token == plussym || lexemes[*i].token == minussym) {
            if (lexemes[*i].token == plussym) {

                (*i)++;
                TERM(inputFile, outputFile, i);
                emit("OPR", 0, 1); // ADD
            } else {

                (*i)++;
                TERM(inputFile, outputFile, i);
                emit("OPR", 0, 2); // SUB
            }
        }
    }
}

void TERM(FILE *inputFile, FILE *outputFile, int *i) {
    FACTOR(inputFile, outputFile, i);
    while (lexemes[*i].token == multsym || lexemes[*i].token == slashsym || lexemes[*i].token == modsym) {
        switch (lexemes[*i].token) {
            case multsym:
               (*i)++;
                //printf("%d %s\n",*i,lexemes[*i].lexeme);

                FACTOR(inputFile, outputFile, i);
                //printf("%d %s",*i,lexemes[*i].lexeme);
                emit("OPR", 0,3 ); // MUL
                break;
            case slashsym:
                (*i)++;
                FACTOR(inputFile, outputFile, i);
                emit("OPR", 0, 4); // DIV
                break;
            case modsym:

                (*i)++;
                FACTOR(inputFile, outputFile, i);
                //emit("OPR", 0, 7); // MOD
                break;
            default:
                error(27, outputFile);
                // fprintf(stderr, "Unexpected token in TERM.\n");
                // fprintf(outputFile, "Unexpected token in TERM.\n");

                exit(EXIT_FAILURE);
        }
    }

}

void FACTOR(FILE *inputFile, FILE *outputFile, int *i) {
    int symIdx;

    switch (lexemes[*i].token) {
        case oddsym:
            emit("ODD", 0, 11);
            do {
                (*i)++;
                if (lexemes[*i].token != lparentsym) {
                    error(31, outputFile);
                    // fprintf(stderr, "Error: '(' expected after 'odd'\n");
                    // fprintf(outputFile, "Error: '(' expected after 'odd'\n");

                     exit(EXIT_FAILURE);
                }
                (*i)++;
                EXPRESSION(inputFile, outputFile, i);
                if (lexemes[*i].token != rparentsym) {
                    error(22, outputFile);
                    // fprintf(stderr, "Error: ')' expected after expression in 'odd' condition\n");
                    // fprintf(outputFile, "Error: ')' expected after expression in 'odd' condition\n");

                     exit(EXIT_FAILURE);
                }
                (*i)++;
                 // Example: emit ODD operation
            } while (lexemes[*i].token == oddsym);
            break;

        case identsym:
            SYMBOLTABLECHECK(lexemes[*i].lexeme, &symIdx);
            if (symIdx == -1) {
                error(11, outputFile);
                // fprintf(stderr, "Error: Undeclared identifier '%s'\n", lexemes[*i].lexeme);
                 exit(EXIT_FAILURE);
            }
            if (symbols[symIdx].kind == 1) {
                //printf("%d %s",*i, lexemes[*i].lexeme);
                emit("LIT", 0, symbols[symIdx].kind); // LIT (M = symbols[symIdx].val)
            } else {
                emit("LOD", 0, symbols[symIdx].addr); // LOD (M = symbols[symIdx].addr)
            }
            (*i)++;
            break;

        case numbersym:
            emit("LIT", 0, atoi(lexemes[*i].lexeme)); // LIT
            (*i)++;
            break;

        case lparentsym:
            (*i)++;

            CONDITION(inputFile, outputFile, i);

            if (lexemes[*i].token != rparentsym) {
                error(22, outputFile);
                // fprintf(stderr, "Error: ')' expected\n");
                // fprintf(outputFile, "Error: ')' expected\n");

                exit(EXIT_FAILURE);
            }
            (*i)++;


            break;

        default:
          // error()
            fprintf(stderr, "Error: Factor expected\n");
            exit(EXIT_FAILURE);
    }
}

void error(int n, FILE *outputFile) {

    // Debugging statement:
    // printf("Token: Type: %s Val: %s\n", curr_token.type, curr_token.value);

    switch (n) {
    case 0:
        printf("\nNo errors, program is syntactically correct \n");
        fprintf(outputFile, "\nNo errors, program is syntactically correct \n");
        break;
    case 1:
        printf("***** Error number 1, Use = instead of :=\n");
        fprintf(outputFile, "***** Error number 1, Use = instead of :=\n");
        break;
    case 2:
        printf("***** Error number 2, = must be followed by a number\n");
        fprintf(outputFile, "***** Error number 2, = must be followed by a number\n");
        break;
    case 3:
        printf("***** Error number 3, Identifier must be followed by =\n");
        fprintf(outputFile, "***** Error number 3, Identifier must be followed by =\n");
        break;
    case 4:
        printf("***** Error number 4, Const, var, procedure must be followed by identifier\n");
        fprintf(outputFile, "***** Error number 4, Const, var, procedure must be followed by identifier\n");
        break;
    case 5:
        printf("***** Error number 5, Semicolon or comma missing\n");
        fprintf(outputFile, "***** Error number 5, Semicolon or comma missing\n");
        break;
    case 6:
        printf("***** Error number 6, Incorrect symbol after procedure declaration\n");
        fprintf(outputFile, "***** Error number 6, Incorrect symbol after procedure declaration\n");
        break;
    case 7:
        printf("***** Error number 7, Statement expected\n");
        fprintf(outputFile, "***** Error number 7, Statement expected\n");
        break;
    case 8:
        printf("***** Error number 8, Incorrect symbol after statement part in block\n");
        fprintf(outputFile, "***** Error number 8, Incorrect symbol after statement part in block\n");
        break;
    case 9:
        printf("***** Error number 9, Period expected\n");
        fprintf(outputFile, "***** Error number 9, Period expected\n");
        break;
    case 10:
        printf("***** Error number 10, Semicolon between statements missing\n");
        fprintf(outputFile, "***** Error number 10, Semicolon between statements missing\n");
        break;
    case 11:
        printf("***** Error number 11, Undeclared identifier\n");
        fprintf(outputFile, "***** Error number 11, Undeclared identifier\n");
        break;
    case 12:
        printf("***** Error number 12, Assignment to constant or procedure is not allowed\n");
        fprintf(outputFile, "***** Error number 12, Assignment to constant or procedure is not allowed\n");
        break;
    case 13:
        printf("***** Error number 13, Assignment operator expected\n");
        fprintf(outputFile, "***** Error number 13, Assignment operator expected\n");
        break;
    case 14:
        printf("***** Error number 14, Call must be followed by an identifier\n");
        fprintf(outputFile, "***** Error number 14, Call must be followed by an identifier\n");
        break;
    case 15:
        printf("***** Error number 15, Call of a constant or variable is meaningless\n");
        fprintf(outputFile, "***** Error number 15, Call of a constant or variable is meaningless\n");
        break;
    case 16:
        printf("***** Error number 16, Then expected\n");
        fprintf(outputFile, "***** Error number 16, Then expected\n");
        break;
    case 17:
        printf("***** Error number 17, Semicolon or end expected\n");
        fprintf(outputFile, "***** Error number 17, Semicolon or end expected\n");
        break;
    case 18:
        printf("***** Error number 18, do expected\n");
        fprintf(outputFile, "***** Error number 18, do expected\n");
        break;
    case 19:
        printf("***** Error number 19, Incorrect symbol following statement\n");
        fprintf(outputFile, "***** Error number 19, Incorrect symbol following statement\n");
        break;
    case 20:
        printf("***** Error number 20, Relational operator expected\n");
        fprintf(outputFile, "***** Error number 20, Relational operator expected\n");
        break;
    case 21:
        printf("***** Error number 21, Expression must not contain a procedure identifier\n");
        fprintf(outputFile, "***** Error number 21, Expression must not contain a procedure identifier\n");
        break;
    case 22:
        printf("***** Error number 22, Right parenthesis missing\n");
        fprintf(outputFile, "***** Error number 22, Right parenthesis missing\n");
        break;
    case 23:
        printf("***** Error number 23, The preceding factor cannot begin with this symbol\n");
        fprintf(outputFile, "***** Error number 23, The preceding factor cannot begin with this symbol\n");
        break;
    case 24:
        printf("***** Error number 24, An expression cannot begin with this symbol\n");
        fprintf(outputFile, "***** Error number 24, An expression cannot begin with this symbol\n");
        break;
    case 25:
        printf("***** Error number 25, This number is too large\n");
        fprintf(outputFile, "***** Error number 25, This number is too large\n");
        break;
    case 26:
        printf("***** Error number 26, Identifier too long\n");
        fprintf(outputFile, "***** Error number 26, Identifier too long\n");
        break;
    case 27:
        printf("***** Error number 27, Invalid symbol.\n");
        fprintf(outputFile, "***** Error number 27, Invalid symbol.\n");
        break;
    case 28:
        printf("***** Error number 28, Constant has already been declared.\n");
        fprintf(outputFile, "***** Error number 28, Constant has already been declared.\n");
        break;
    case 29:
        printf("***** Error number 29, Constant must be assigned an integer value.\n");
        fprintf(outputFile, "***** Error number 29, Constant must be assigned an integer value.\n");
        break;
    case 30:
        printf("***** Error number 30, Identifier already declared.\n");
        fprintf(outputFile, "***** Error number 30, Identifier already declared.\n");
        break;
    case 31:
        printf("***** Error number 31, Left parenthesis missing\n");
        fprintf(outputFile, "***** Error number 31, Left parenthesis missing\n");
        break;
    case 32:
        printf("***** Error number 32, Unclosed comment\n");
        fprintf(outputFile, "***** Error number 32, Unclosed comment\n");
        break;
    default:
        printf("No errors, program is syntactically correct\n");
        fprintf(outputFile, "No errors, program is syntactically correct\n");
        break;
    }
}




