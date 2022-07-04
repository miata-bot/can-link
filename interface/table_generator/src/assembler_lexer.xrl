Definitions.

INT        = [0-9]+
ATOM       = :[a-z_]+
WHITESPACE = [\s\t\n\r]
HEX        = #[0-9a-fA-F]+

Rules.

{HEX}         : {token, {value,    TokenLine, list_to_integer(lists:sublist(TokenChars, 2, TokenLen - 1), 16)}}.
{INT}         : {token, {value,    TokenLine, list_to_integer(TokenChars)}}.
CLS           : {token, {mnemonic, TokenLine, cls}}.
LD            : {token, {mnemonic, TokenLine, ld}}.
FILL          : {token, {mnemonic, TokenLine, fill}}.
FLUSH         : {token, {mnemonic, TokenLine, flush}}.
SET           : {token, {mnemonic, TokenLine, set}}.
INC           : {token, {mnemonic, TokenLine, inc}}.
DEC           : {token, {mnemonic, TokenLine, dec}}.
HALT          : {token, {mnemonic, TokenLine, halt}}.
NOP           : {token, {mnemonic, TokenLine, nop}}.
JNZ           : {token, {mnemonic, TokenLine, jnz}}.
[a-z_A-Z]+    : {token, {opperand,     TokenLine, list_to_atom(TokenChars)}}.
\;.*          : skip_token.
\.[a-z_A-Z]+\:   : {token, {symbol,  TokenLine, list_to_atom(lists:sublist(TokenChars, 2, TokenLen - 2))}}.
\.[a-z_A-Z]+     : {token, {address, TokenLine, list_to_atom(lists:sublist(TokenChars, 2, TokenLen - 1))}}.
{WHITESPACE}+ : skip_token.

Erlang code.