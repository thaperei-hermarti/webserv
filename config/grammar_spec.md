<!--
EBNF
::= or = — "is defined as"
| — alternatives ("either/or")
[ x ] — optional (zero or one)
{ x } — repeated (zero or more)
( x y ) — grouping
"literal" — an exact keyword or symbol that must appear verbatim
-->

config ::= { server_block }; <!-- pelo menos um -->
server_block ::= "server" "{" server_directive { location_block } "}";
server_directive ::= listen_directive
                  | server_name_directive
                  | root_directive
                  | index_directive
                  | client_max_body_size_directive
                  | error_page_directive;

location_block ::= path "{" method_directive
                | autoindex_directive
                | root_directive
                | index_directive
                | upload_store_directive
                | redirect_directive
                | { cgi_directive } "}";

server_name_directive ::= "server_name" string { string } TERMINATOR;
<!-- Não sei se é necessário ter múltiplos server_names -->

listen_directive ::= "listen" [ interface ] number TERMINATOR

interface ::= string ":";

index_directive ::= "index" filename { filename } TERMINATOR;

root_directive ::= "root" relative_path | path TERMINATOR;

client_max_body_size_directive ::= "client_max_body_size" ( NUMBER "M" )
                                    | "0" TERMINATOR;

method_directive ::= "method" METHOD { METHOD } TERMINATOR;

error_page_directive ::= "error_page" number { number } path "/" filename TERMINATOR;

autoindex_directive ::= "autoindex" "on" | "off" TERMINATOR;

redirect_directive ::= "return" number path TERMINATOR;

cgi_directive ::= "cgi_pass" extension path TERMINATOR;

filename ::= string extension;

extension ::= "." string;

relative_path ::= "." path;

path ::= "/" string { path };

number ::= DIGIT { DIGIT };

string ::= LETTER { LETTER };

METHOD ::= "GET" | "POST" | "DELETE";

LETTER = "A" | "B" | "C" | "D" | "E" | "F" | "G"
       | "H" | "I" | "J" | "K" | "L" | "M" | "N"
       | "O" | "P" | "Q" | "R" | "S" | "T" | "U"
       | "V" | "W" | "X" | "Y" | "Z" | "a" | "b"
       | "c" | "d" | "e" | "f" | "g" | "h" | "i"
       | "j" | "k" | "l" | "m" | "n" | "o" | "p"
       | "q" | "r" | "s" | "t" | "u" | "v" | "w"
       | "x" | "y" | "z" ;

DIGIT = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;

TERMINATOR ::= ";";
