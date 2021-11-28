#pragma once

#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>

#include "token.h"

// #define DEBUG
#define HANDEL
// #define NOHANDLE
#define ILLEGAL "ILLEGAL"
#define FEOF "EOF"

// define identifiers and literals
#define IDENT "IDENT"
#define NUMB "NUMB"
#define STRINGVAL "STRINGVAL"

// define operators
#define ASSIGN "="
#define ARROW "=>"
#define PLUS "+"
#define PLUSEQ "+="
#define MINUS "-"
#define MINUSEQ "-="
#define BANG "!"
#define ASTERISK "*"
#define SPLASH "/"
#define MOD "%"
#define MODEQ "%="
#define LT "<"
#define LTOEQ "<="
#define GT ">"
#define GTOEQ ">="
#define EQ "=="
#define NEQ "!="
#define BAND "&"
#define AND "&&"
#define BOR "|"
#define OR "||"
#define RSPLASH "\\" // 转译字符
// define comment symbol
#define LPCOMMENT "/*"
#define RPCOMMENT "*/"
#define CCOMMENT "//"
// define delimiters
#define POINT "."
#define COMMA ","
#define SEMICOLON ";"
#define LPAREN "("
#define RPAREN ")"
#define LSBRACE "["
#define RSBRACE "]"
#define LBRACE "{"
#define RBRACE "}"
#define SQUO "\'"
#define DQUO "\""

// define keywords
#define PRAGMA "PRAGMA"
#define CONTRACT "CONTRACT"
#define LIBRARY "LIBRARY"
#define EVENT "EVENT"
#define PUBLIC "PUBLIC"
#define PRIVATE "PRIVATE"
#define REQUIRE "REQUIRE"
#define INDEXED "INDEXED"
#define VIEW "VIEW"
#define PURE "PURE"
#define CONSTANT "CONSTANT"
#define PAYABLE "PAYABLE"
#define INTERNAL "INTERNAL"
#define EXTERNAL "EXTERNAL"
#define MODIFIER "MODIFIER"
#define CONSTRUCTOR "CONSTRUCTOR"
#define FUNCTION "FUNCTION"
#define RETURN "RETURN"
#define RETURNS "RETURNS"
#define IF "IF"
#define ELSE "ELSE"
#define IS "IS"
#define FOR "FOR"
#define USING "USING"
#define THROW "THROW"
#define EMIT "EMIT"
#define WHILE "WHILE"
#define DELETE "DELETE"
// define value type (mis fixed,ufixed,bytes)
#define VBOOL "BOOL"
#define VINT "INT"
#define VINT8 "INT8"
#define VINT16 "INT16"
#define VINT32 "INT32"
#define VINT64 "INT64"
#define VINT128 "INT128"
#define VINT256 "INT256"
#define VUINT "UINT"
#define VUINT8 "UINT8"
#define VUINT16 "UINT16"
#define VUINT32 "UINT32"
#define VUINT64 "UINT64"
#define VUINT128 "UINT128"
#define VUINT256 "UINT256"
#define ADDRESS "ADDRESS"
#define VSTRING "STRING"
#define VENUM "ENUM"
#define MAPPING "MAPPING"
#define STRUCT "STRUCT"
#define BYTES "BYTES"
#define FIXED "FIXED"
#define UFIXED "UFIXED"

// define API
#define BALANCE "BALANCE"
#define TRANSFER "TRANSFER"
#define SEND "SEND"
#define CALL "CALL"
#define CALLCODE "CALLCODE"
#define DELEGATECALL "DELEGATECALL"
#define THIS "THIS"
#define MSG "MSG"
#define SENDER "SENDER"
#define VALUE "VALUE"
#define BLOCK "BLOCK"
#define NOW "NOW"
#define TIMESTAMP "TIMESTAMP"
#define GAS "GAS"
#define DAYS "DAYS"
#define WEEKS "WEEKS"
#define YEARS "YEARS"

// define SafeMath API
#define SAFEMATH "SAFEMATH"
#define MUL "MUL"
#define DIV "DIV"
#define SUB "SUB"
#define ADD "ADD"

// define EVENT 部分
#define HALTED "HALTED" // Halted
#define ETRANSFER "ETRANSFER" // Transfer
#define APPROVAL "APPROVAL" // Approval
#define BURN "BURN" // Burn
#define BURNED "BURNED" // Burned
#define MINT "MINT" // Mint
#define MINTED "MINTED" // Minted
#define UPGRADE "UPGRAGE" // Upgrade
#define INVESTED "INVESTED" //Invested
#define WHITELISTED "WHITELISTED" // Whitelisted
#define RELEASE "RELEASE" // Release
#define LOCK "LOCK" // Lock
#define PAUSE "PAUSE" // Pause
#define UNPAUSE "UNPAUSE" // Unpause
#define DEPOSIT "DEPOSIT" // Deposit
#define BUY "BUY" // Buy
#define SELL "SELL" // Sell
#define LOG "LOG" // Log

namespace solver {

static const std::unordered_map<std::string, std::string> keywords{
    {"Halted", HALTED},
    {"Transfer", ETRANSFER},
    {"Approval", APPROVAL},
    {"Burn", BURN},
    {"Burned", BURNED},
    {"Mint", MINT},
    {"Mint", MINTED},
    {"Upgrade", UPGRADE},
    {"Invested", INVESTED},
    {"Whitelisted", WHITELISTED},
    {"Release", RELEASE},
    {"Lock", LOCK},
    {"Pause", PAUSE},
    {"Unpause", UNPAUSE},
    {"Deposit", DEPOSIT},
    {"Buy", BUY},
    {"Sell", SELL},
    {"Log", LOG},
    {"pragma", PRAGMA},
    {"contract", CONTRACT},
    {"library", LIBRARY},
    {"event", EVENT},
    {"public", PUBLIC},
    {"private", PRIVATE},
    {"require", PRIVATE},
    {"indexed", INDEXED},
    {"view", VIEW},
    {"constant", CONSTANT},
    {"pure", PURE},
    {"payable", PAYABLE},
    {"internal", INTERNAL},
    {"external", EXTERNAL},
    {"constructor", CONSTRUCTOR},
    {"modifier", MODIFIER},
    {"function", FUNCTION},
    {"return", RETURN},
    {"returns", RETURNS},
    {"if", IF},
    {"else", ELSE},
    {"is", IS},
    {"using", USING},
    {"for", FOR},
    {"delete", DELETE},
    {"throw", THROW},
    {"bool", VBOOL},
    {"int", VINT},
    {"int8", VINT8},
    {"int16", VINT16},
    {"int32", VINT32},
    {"int64", VINT64},
    {"int128", VINT128},
    {"int256", VINT256},
    {"uint", VUINT},
    {"uint8", VUINT8},
    {"uint16", VUINT16},
    {"uint32", VUINT32},
    {"uint64", VUINT64},
    {"uint128", VUINT128},
    {"uint256", VUINT256},
    {"address", ADDRESS},
    {"string", VSTRING},
    {"enum", VENUM},
    {"mapping", MAPPING},
    {"struct", STRUCT},
    {"balance", BALANCE},
    {"transfer", TRANSFER},
    {"send", SEND},
    {"value", VALUE},
    {"call", CALL},
    {"callcode", CALLCODE},
    {"delegatecall", DELEGATECALL},
    {"gas", GAS},
    {"this", THIS},
    {"msg", MSG},
    {"sender", SENDER},
    {"now", NOW},
    {"timestamp", TIMESTAMP},
    {"block", BLOCK},
    {"days", DAYS},
    {"mouths", YEARS},
    {"weeks", WEEKS},
    {"years", YEARS},
    {"day", YEARS},
    {"wekk", YEARS},
    {"mouth", YEARS},
    {"year", YEARS},
    {"emit", EMIT},
    {"while", WHILE},
    {"SafeMath", SAFEMATH},
    {"mul", MUL},
    {"div", DIV},
    {"sub", SUB},
    {"add", ADD}};

static const std::set<std::string> variable_define{
    "int", "uint", "bytes", "fixed", "ufixed",
    "address", "string", "enum", "mapping", "struct"};

static const std::set<std::string> api_set{
    BALANCE, TRANSFER, SEND,   CALL, CALLCODE,  DELEGATECALL, GAS,
    THIS,    MSG,      SENDER, NOW,  TIMESTAMP, BLOCK,        DAYS,
    YEARS, MUL, DIV, SUB, ADD, SAFEMATH};

const std::string result_dir = "../smartbugs-results/results/oyente/icse20/";
// const std::string contract_dir = "../smartbugs-wild/contracts/";
const std::string contract_dir = "../smartbugs/dataset/unchecked_low_level_calls/";

class Util {
  public:
    static std::string LookUpIdent(std::string literal);
    static bool IsVirableDefine(std::string type);
    static bool IsAPI(std::string type);
    static bool IsLetter(char ch);
    static bool IsDigit(char ch);
    static std::string ReadFileIntoString(std::string file_name);
};

} // namespace solver