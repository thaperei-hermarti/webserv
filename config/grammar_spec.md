## Grammar specification
### EBNF Syntax

= — "is defined as"

| — alternatives ("either/or")

[ x ] — optional (zero or one)

{ x } — repeated (zero or more)

( x y ) — grouping

"literal" — an exact keyword or symbol that must appear verbatim

? ... ? — Special sequence

### EBNF rules

config = server_block, { server_block };

server_block = "server", "{", { server_directive | location_block }, "}";

server_directive = listen_directive
                  | server_name_directive
                  | root_directive
                  | index_directive
                  | client_max_body_size_directive
                  | error_page_directive;

location_block = "location", PATH, "{", { location_directive }, "}";

location_directive = method_directive
                    | autoindex_directive
                    | root_directive
                    | index_directive
                    | client_max_body_size_directive
                    | upload_dir_directive
                    | return_directive
                    | error_page_directive
                    | cgi_directive;


listen_directive = "listen", [ host, ":"], port, TERMINATOR;

server_name_directive = "server_name", hostname, { hostname }, TERMINATOR;

host = ipv4_address | hostname;

index_directive = "index", filename, { filename }, TERMINATOR;

root_directive = "root", PATH, TERMINATOR;

client_max_body_size_directive = "client_max_body_size", number, "m", TERMINATOR;

method_directive = "method", HTTP_METHOD, { HTTP_METHOD }, TERMINATOR;

error_page_directive = "error_page", status_code, { status_code }, PATH, TERMINATOR;

autoindex_directive = "autoindex", ( "on" | "off" ), TERMINATOR;

upload_dir_directive = "upload_dir", PATH, TERMINATOR;

return_directive = "return", [ status_code ], ( PATH | url ), TERMINATOR;

cgi_directive = "cgi_pass", extension, PATH, TERMINATOR;

filename = word, [ extension ];
extension = ".", word;
word = WORD_CHAR, { WORD_CHAR };
hostname = word, { ".", word };
ipv4_address = octet, ".", octet, ".", octet, ".", octet;
octet = DIGIT, [ DIGIT ], [ DIGIT ];
url = scheme, "://", hostname, [ ":", port ], [ PATH ];
scheme = "http" | "https";
port = number;
status_code = DIGIT, DIGIT, DIGIT;
number = DIGIT, { DIGIT };

WORD_CHAR = ( LETTER | DIGIT | "-" | "_" );

HTTP_METHOD = "GET" | "POST" | "DELETE";

LETTER = "A" | "B" | "C" | "D" | "E" | "F" | "G"
       | "H" | "I" | "J" | "K" | "L" | "M" | "N"
       | "O" | "P" | "Q" | "R" | "S" | "T" | "U"
       | "V" | "W" | "X" | "Y" | "Z" | "a" | "b"
       | "c" | "d" | "e" | "f" | "g" | "h" | "i"
       | "j" | "k" | "l" | "m" | "n" | "o" | "p"
       | "q" | "r" | "s" | "t" | "u" | "v" | "w"
       | "x" | "y" | "z" ;

DIGIT = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;

TERMINATOR = ";";

PATH = "/", { ALLOWED_CHAR };

ALLOWED_CHAR = ? any character except whitespaces, ';', '{', '}' ?;

## Business rules

- In listen directive, only hostname or ipv4 are accepted for the interface (each octet must be between 0-255)
- client_max_body_size must be in megabytes (mandatory m suffix)
- Empty values are not accepted
- Wildcard and regex is not allowed in any field
- File extension must have alphabetic characters and no spaces
- Nested location are not allowed
- Port numbers must be between 1-65535
- Duplicate root must return error
- Status code must have 3 digits and between 100-511
- listen is localhost:80 by default
- if error_page is absent, the server serves a generic error page at internal path of webserv
- index is index.html by default
- method directive is GET, POST and DELETE by default
- Return has 302 code by default
- Return accept both local path and absolute URL as a destination (external redirect)
- Paths must not have spaces
- Paths with ".." must be removed before split into script-path
- Keywords are case-sensitive
- Comments (#) are ignores by lexer

### Cardinality and politics of duplicates by directive

| Directive                | Allowed Cardinality                       | If violated                                         |
|--------------------------|-------------------------------------------|-----------------------------------------------------|
| `listen`                 | 0 or 1 per server                      | error                                               |
| `server_name`            | 0 or 1 per server (with multiple server_name on the same line)      | error if duplicated   |
| `root` (server)          | exactly 1 per server               | error if duplicated or absent  |
| `root` (location)        | 0 or 1 per block                    | error if duplicated; location heritage from server if absent |
| `index`                     | 0 or 1 (with multiple filenames on the same line)       | error if duplicated                 |
| `client_max_body_size`       | 0 or 1                                    | error if duplicated                             |
| `error_page`                   | 0 or more                                 | accumulate (each occurrence maps code -> path)   |
| `method`                         | 0 or 1 per location (with multiple methods) | error if duplicated                             |
| `autoindex`                       | 0 or 1 per location                       | error if duplicated                                 |
| `upload_dir`                        | 0 or 1 per location                       | error if duplicated                                 |
| `return`                              | 0 or 1 per location                       | error if duplicated                                 |
| `cgi_pass`                              | 0 or more (one per extension)              | error if the same extension is repeated          |

### Mandatory directives

**Per server block**: root.
