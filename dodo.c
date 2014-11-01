#include <stdio.h> /* fopen, fseek, fread, fwrite, FILE */
#include <stdlib.h> /* exit */
#include <string.h> /* strcmp, strncmp */

/***** data structures and manipulation *****/

enum Command {
    /* optionally takes num
     * prints $num bytes
     * $num defaults to 100 if not supplied
     */
    print,
    /* takes num
     * goto line in file
     */
    line,
    /* takes num
     * goto byte in file
     */
    byte,
    /* takes string
     * compares string to current file location
     * exits with code <EXPECT_EXIT_CODE> if string doesn't match
     */
    expect,
    /* takes string
     * writes string to current location in file
     * leaves the cursor positioned after the write
     */
    write,
    /* exists with code <QUIT_EXIT_CODE>
     */
    quit
};

/* interpretation depends on Command */
struct Argument {
    /* either numeric argument OR length of string */
    int num;
    char *str;
};

struct Instruction {
    /* type of command determines dispatch to eval_ function
     * which in turn determines interpretation of argument
     */
    enum Command command;
    struct Argument argument;
    /* next Instruction in linked list */
    struct Instruction *next;
};

struct Program {
    /* linked list of Instruction(s) */
    struct Instruction *start;
    /* file program is operating on */
    FILE *file;
    /* current offset into file */
    int offset;
    /* program source read into a buffer */
    char *source;
    /* shared buffer (and length) used for reading into */
    char *buf;
    size_t buf_len;
};

struct Instruction *new_instruction(enum Command command){
    struct Instruction *i;

    i = calloc(1, sizeof(struct Instruction));
    if( ! i ){
        puts("new_instruction: call to calloc failed");
        return 0;
    }

    i->command = command;

    return i;
}

/***** internal helpers ******/
/* return a buffer of at least size required_len
 * returns 0 on error
 */
char *get_buffer(struct Program *p, size_t required_len){
    if( ! p ){
        return 0;
    }

    if( p->buf_len >= required_len ){
        return p->buf;
    }

    p->buf_len = required_len;
    p->buf = realloc(p->buf, required_len);

    if( ! p->buf ){
        puts("get_buffer: failed to allocate buffer");
        return 0;
    }

    return p->buf;
}

#define BUF_INCR 1024

/* return a char* containing data from provided FILE*
 * returns 0 on error
 */
char * slurp(FILE *file){
    size_t size = BUF_INCR;
    size_t offset = 0;
    size_t nr = 0;
    char *buf;

    if( ! file ){
        return 0;
    }

    buf = malloc(size);
    if( ! buf ){
        return 0;
    }

    while( BUF_INCR == (nr = fread(&(buf[offset]), 1, BUF_INCR, file)) ){
        offset = size;
        size += BUF_INCR;

        buf = realloc(buf, size);

        if( ! buf ){
            return 0;
        }
    }

    /* check for fread errors */
    if( ferror(file) ){
        puts("slurp: file read failed");
        return 0;
    }

    return buf;
}


/***** parsing functions *****/

/* parsing helper method for parsing a string argument to a command
 * used for expect e/string/
 * and for write w/string/
 *
 * returns instruction on success
 * 0 on error
 */
struct Instruction * parse_string(struct Instruction *i, char *source, size_t *index){
    int len = 0;

    if( '/' != source[*index] ){
        printf("Parse_string: unexpected character '%c', expected beginning delimiter'/'\n", source[*index]);
        return 0;
    }

    /* skip past starting delimiter */
    ++(*index);

    /* save start of string */
    i->argument.str = &(source[*index]);

    /* count length of string */
    /* FIXME may want to have buffer length passed in
     * 'just incase; buffer is not \0 terminated
     */
    for( len=0; ; ++(*index) ){
        switch( source[*index] ){
            /* end of buffer */
            case '\0':
                /* error, expected terminating / */
                puts("Parse_string: unexpected end of source buffer, expected terminating delimiter'/'");
                return 0;
                break;

            /* terminating delimiter */
            case '/':
                /* skip past / */
                ++(*index);
                /* we are finished here */
                goto EXIT;
                break;

            /* just another character in our string */
            default:
                ++len;
                break;
        }
    }

EXIT:

    i->argument.num = len;

    return i;
}

struct Instruction * parse_print(char *source, size_t *index){
    struct Instruction *i;

    i = new_instruction(print);
    if( ! i ){
        puts("Parse_print: call to new_instruction failed");
        return 0;
    }

    puts("parse_print unimplemented");
    return 0; /* FIXME unimplemented */
}

struct Instruction * parse_byte(char *source, size_t *index){
    struct Instruction *i;

    i = new_instruction(byte);
    if( ! i ){
        puts("Parse_byte: call to new_instruction failed");
        return 0;
    }

    /* bn where n is positive integer */
    switch( source[*index] ){
        case 'b':
        case 'B':
            ++(*index);
            break;
        default:
            printf("Unexpected character '%c', expected 'b'\n", source[*index]);
            break;
    }

    /* read in number */
    if( ! sscanf(&(source[*index]), "%d", &(i->argument.num)) ){
        puts("Parse_byte: failed to read in specified number of bytes");
        return 0;
    }

    /* advance past number */
    for( ; ; ++(*index) ){
        switch( source[*index] ){
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                break;

            default:
                goto EXIT;
                break;
        }
    }

EXIT:

    return i;
}

struct Instruction * parse_line(char *source, size_t *index){
    struct Instruction *i;

    i = new_instruction(line);
    if( ! i ){
        puts("Parse_line: call to new_instruction failed");
        return 0;
    }

    puts("parse_line unimplemented");
    return 0; /* FIXME unimplemented */
}

struct Instruction * parse_expect(char *source, size_t *index){
    struct Instruction *i;

    i = new_instruction(expect);
    if( ! i ){
        puts("Parse_expect: call to new_instruction failed");
        return 0;
    }

    /* e/string/ */
    switch( source[*index] ){
        case 'e':
        case 'E':
            ++(*index);
            break;

        default:
            printf("Parse_expect: unexpected character '%c', expected 'e'\n", source[*index]);
            return 0;
            break;
    }

    return parse_string(i, source, index);
}

struct Instruction * parse_write(char *source, size_t *index){
    struct Instruction *i;

    i = new_instruction(write);
    if( ! i ){
        puts("Parse_write: call to new_instruction failed");
        return 0;
    }

    /* w/string/ */
    switch( source[*index] ){
        case 'w':
        case 'W':
            ++(*index);
            break;

        default:
            printf("Parse_write: unexpected character '%c', expected 'w'\n", source[*index]);
            return 0;
            break;
    }

    return parse_string(i, source, index);
}

struct Instruction * parse_quit(char *source, size_t *index){
    struct Instruction *i;

    switch( source[*index] ){
        case 'q':
        case 'Q':
            /* advance past letter */
            ++(*index);
        case '\0': /* treat \0 as implicit quit */
            break;

        default:
            printf("Parse_quit: unexpected character '%c'\n", source[*index]);
            return 0;
    }


    i = new_instruction(quit);
    if( ! i ){
        puts("Parse_quit: call to new_instruction failed");
        return 0;
    }

    return i;
}

/* consume comment from source
 * return 0 on success
 * return 1 on error
 */
int parse_comment(char *source, size_t *index){

    if( source[*index] != '#' ){
        printf("Parse_comment: expected '#', got '%c'\n", source[*index]);
        return 1;
    }

    /* consume source until \n or \0 are found */
    while( 1 ){
        switch( source[*index] ){

            case '\n':
            case '\0':
                /* end comment
                 * leave character for parent to look at
                 */
                break;

            default:
                ++(*index);
                break;

        }
    }

    return 0;
}

/* parse provided source into Program
 * return 0 on success
 * return 1 on failure
 */
int parse(struct Program *program){
    /* index into source */
    size_t index = 0;
    /* length of source */
    size_t len;
    /* result from call to parse_ functions */
    struct Instruction *res;
    /* place to store next parsed Instruction */
    struct Instruction **store;
    /* temporary pointer to program->source */
    char *source;

    if( ! program || ! program->source ){
        puts("Parse called with null program or source");
        return 1;
    }

    source = program->source;

    len = strlen(source);
    store = &(program->start);

    while( index < len ){
        switch( source[index] ){
            case 'p':
            case 'P':
                res = parse_print(source, &index);
                if( ! res ){
                    puts("Parse: failed in call to parse_print");
                    return 1;
                }
                *store = res;
                store = &(res->next);
                break;

            case 'b':
            case 'B':
                res = parse_byte(source, &index);
                if( ! res ){
                    puts("Parse: failed in call to parse_byte");
                    return 1;
                }
                *store = res;
                store = &(res->next);
                break;

            case 'l':
            case 'L':
                res = parse_line(source, &index);
                if( ! res ){
                    puts("Parse: failed in call to parse_line");
                    return 1;
                }
                *store = res;
                store = &(res->next);
                break;

            case 'e':
            case 'E':
                res = parse_expect(source, &index);
                if( ! res ){
                    puts("Parse: failed in call to parse_expect");
                    return 1;
                }
                *store = res;
                store = &(res->next);
                break;

            case 'w':
            case 'W':
                res = parse_write(source, &index);
                if( ! res ){
                    puts("Parse: failed in call to parse_write");
                    return 1;
                }
                *store = res;
                store = &(res->next);
                break;

            case 'q':
            case 'Q':
            case '\0': /* treat \0 as implicit quit */
                res = parse_quit(source, &index);
                if( ! res ){
                    puts("Parse: failed in call to parse_quit");
                    return 1;
                }
                *store = res;
                store = &(res->next);
                goto EXIT;
                break;

            case '#':
                if( parse_comment(source, &index) ){
                    puts("Parsing comment failed");
                    return 1;
                }
                break;

            /* skip over whitespace
             * whitespace is insignificant EXCEPT for \n denoting end of comment
             */
            case ' ':
            case '\t':
            case '\n':
                /* actually skip over whitespace */
                ++index;
                break;

            default:
                printf("Parse: Invalid character encountered '%c'\n", source[index]);
                return 1;
                break;
        }
    }

EXIT:

    /* null terminator for program */
    *store = 0;

    return 0;
}




/***** evaluation functions *****/

int eval_print(struct Program *p, struct Instruction *cur){
    /* number of bytes to read */
    int num = cur->argument.num;
    char *buf;
    /* number of bytes read */
    size_t nr = 0;

    /* default to 100 bytes */
    if( ! num ){
        num = 100;
    }

    /* 1 + num to fit num bytes and null */
    buf = get_buffer(p, 1+num);
    if( ! buf ){
        puts("eval_prin: call to get_buffer failed");
        return 1;
    }

    /* read into buffer */
    nr = fread(buf, 1, num, p->file);
    /* make sure buffer is really a string */
    buf[nr] = '\0';

    /* seek back to previous position */
    if( fseek(p->file, p->offset, SEEK_SET) ){
        puts("Eval_expect: fseek failed");
        return 1;
    }

    /* print buffer, as instructed */
    printf("'%s'\n", buf);

    return 0;
}

int eval_byte(struct Program *p, struct Instruction *cur){
    /* byte number argument to seek to */
    int byte;

    byte = cur->argument.num;

    if( fseek(p->file, byte, SEEK_SET) ){
        puts("Eval_byte: fseek failed");
        return 1;
    }

    /* update file offset */
    p->offset = byte;

    return 0;
}

int eval_line(struct Program *p, struct Instruction *cur){
    puts("eval_line unimplemented");
    return 1; /* FIXME unimplemented */
}

int eval_expect(struct Program *p, struct Instruction *cur){
    /* string to compare to */
    char *str;
    /* length of string */
    size_t len;
    /* buffer read into */
    char *buf;
    /* num bytes read */
    size_t nr;

    str = cur->argument.str;
    if( ! str ){
        puts("Eval_expect: no string argument found");
        return 1;
    }

    len = cur->argument.num;

    /*1 + len to fit len bytes + null terminator */
    buf = get_buffer(p, 1+len);
    if( ! buf ){
        puts("Eval_expect: call to get_buffer failed");
        return 1;
    }

    /* perform read */
    nr = fread(buf, 1, len, p->file);
    /* make sure buffer is really a string */
    buf[nr] = '\0';

    /* seek back to previous position */
    if( fseek(p->file, p->offset, SEEK_SET) ){
        puts("Eval_expect: fseek failed");
        return 1;
    }

    /* compare number read to expected len */
    if( nr != len ){
        /* FIXME consider output when expect fails */
        printf("Eval_expect: expected to read '%zu' bytes, actually read '%zu'\n", len, nr);
        return 1;
    }

    /* compare read string to expected str */
    if( strncmp(str, buf, len) ){
        /* add terminating \0 to allow printf-ing */
        str[len] = '\0';
        /* FIXME consider output when expect fails */
        printf("Eval_expect: expected string '%s', got '%s'\n", str, buf);
        return 1;
    }

    return 0;
}

int eval_write(struct Program *p, struct Instruction *cur){
    /* string to write */
    char *str;
    /* len of str */
    size_t len;
    /* number of bytes written */
    size_t nw;

    str = cur->argument.str;
    if( ! str ){
        puts("Eval_write: no argument string found");
        return 1;
    }

    len = cur->argument.num;

    /* perform write */
    nw = fwrite(str, 1, len, p->file);

    /* check length */
    if( nw != len ){
        printf("Eval_write: expected to write '%zu' bytes, actually wrote '%zu'\n", len, nw);
        return 1;
    }

    /* flush file */
    if( fflush(p->file) ){
        puts("Eval_write: error flushing file");
        return 1;
    }

    return 0;
}

/* execute provided Program
 * return 0 on success
 * return 1 on failure
 */
int execute(struct Program *p){
    /* cursor into program */
    struct Instruction *cur;
    /* return code from individual eval_ calls */
    int ret = 0;

    if( !p ){
        puts("Execute called with null program");
        return 1;
    }

    /* simple dispatch function */
    for( cur = p->start; cur; cur = cur->next ){
        switch( cur->command ){
            case print:
                ret = eval_print(p, cur);
                if( ret ){
                    return ret;
                }
                break;

            case line:
                ret = eval_line(p, cur);
                if( ret ){
                    return ret;
                }
                break;

            case byte:
                ret = eval_byte(p, cur);
                if( ret ){
                    return ret;
                }
                break;

            case expect:
                ret = eval_expect(p, cur);
                if( ret ){
                    return ret;
                }
                break;

            case write:
                ret = eval_write(p, cur);
                if( ret ){
                    return ret;
                }
                break;

            case quit:
                /* escape from loop */
                goto EXIT;
                break;

            default:
                puts("Invalid command type encountered in execute");
                return 1;
                break;
        }
    }

    /* implicit (EOF) or explicit (Command quit) exit */
EXIT:

    return 0;
}



/***** main *****/
void usage(void){
    puts("dodo - scriptable in place file editor\n"
         "dodo takes a single argument of <filename>\n"
         "and will read commands from stdin\n"
         "\n"
         "example:\n"
         "  dodo <filename> <<EOF\n"
         "  b6        # goto byte 6\n"
         "  e/world/  # check for string 'world'\n"
         "  w/hello/  # write string 'hello'\n"
         "  q         #quit\n"
         "  EOF\n"
         "\n"
         "supported commands:\n"
         "  bn        # goto byte <n> of file\n"
         "  ln        # goto line <n> of file -- UNIMPLEMENTED\n"
         "  p         # print 100 bytes\n"
         "  pn        # print n bytes\n"
         "  e/str/    # compare <str> to current position, exit if not equal\n"
         "  w/str/    # write <str> to current position\n"
         "  q         # quit editing\n"
         "  # used for commenting out rest of line\n"
    );
}

int main(int argc, char **argv){
    if(    argc != 2
        || !strcmp("--help", argv[1])
        || !strcmp("-h", argv[1])
    ){
        usage();
        exit(EXIT_FAILURE);
    }

    struct Program p;
    p.start = 0;
    p.file = 0;
    p.buf = 0;
    p.buf_len = 0;
    p.offset = 0;

    // read program into source
    p.source = slurp(stdin);
    if( ! p.source ){
        puts("Reading program failed");
        exit(EXIT_FAILURE);
    }

    // parse program
    if( parse(&p) ){
        puts("Parsing program failed");
        exit(EXIT_FAILURE);
    }

    /* open file */
    p.file = fopen(argv[1], "r+b");
    if( ! p.file ){
        printf("Failed to open specified file '%s'\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    // execute program
    if( execute(&p) ){
        puts("Program execution failed");
        fclose(p.file);
        exit(EXIT_FAILURE);
    }

    if( p.buf ){
        free(p.buf);
    }

    free(p.source);

    fclose(p.file);

    exit(EXIT_SUCCESS);
}

