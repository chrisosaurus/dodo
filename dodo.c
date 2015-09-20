#include <stdio.h> /* fopen, fseek, fread, fwrite, FILE */
#include <stdlib.h> /* exit */
#include <string.h> /* strcmp, strncmp */
#include <ctype.h> /* isdigit */

/***** data structures and manipulation *****/

enum Command {
    /* optionally takes num
     * prints $num bytes
     * $num defaults to 100 if not supplied
     */
    PRINT,
    /* takes num
     * goto line in file
     */
    LINE,
    /* takes num
     * goto byte in file
     */
    BYTE,
    /* takes string
     * compares string to current file location
     * exits with code EXIT_FAILURE if string doesn't match
     */
    EXPECT,
    /* takes string
     * writes string to current location in file
     * leaves the cursor positioned after the write
     */
    WRITE,
    /* exits with code EXIT_SUCCESS
     */
    QUIT
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
    struct Instruction *i = 0;

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
    char *buf = 0;

    if( ! file ){
        return 0;
    }

    buf = calloc(size, 1);
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
 * source must be a c-string meaning that is is an array of characters
 * terminated by the null-terminator ('\0')
 *
 * returns instruction on success
 * 0 on error
 */
struct Instruction * parse_string(struct Instruction *i, char *source, size_t *index){
    int len = 0;

    /* check arguments */
    if( ! i ){
        puts("Parse_string: null instruction");
        return 0;
    }

    if( ! source ){
        puts("Parse_string: null source");
        return 0;
    }

    if( ! index ){
        puts("Parse_string: null index");
        return 0;
    }


    if( '/' != source[*index] ){
        printf("Parse_string: unexpected character '%c', expected beginning delimiter'/'\n", source[*index]);
        return 0;
    }

    /* skip past starting delimiter */
    ++(*index);

    /* save start of string */
    i->argument.str = &(source[*index]);

    /* count length of string */
    for( len=0; ; ++(*index) ){
        switch( source[*index] ){
            /* end of buffer */
            case '\0':
                /* error, expected terminating / */
                puts("Parse_string: unexpected end of source buffer, expected terminating delimiter'/'");
                return 0;
                break;

            case '\\':
                /* A bit yucky: Rework the string to delete the escape character '\'
                 * Note the use of memmove as the source and destination overlap
                 */
                memmove(source+*index,
                        source+*index+1,
                        strlen(source+*index));
                (*index)++;
                len+=2;
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

/* parsing helper method for parsing a number argument to a command
 * used for byte bnumber
 *
 * returns instruction on success
 * 0 on error
 */
struct Instruction * parse_number(struct Instruction *i, char *source, size_t *index){
    /* check arguments */
    if( ! i ){
        puts("Parse_number: null instruction");
        return 0;
    }

    if( ! source ){
        puts("Parse_number: null source");
        return 0;
    }

    if( ! index ){
        puts("Parse_number: null index");
        return 0;
    }


    /* read in number */
    if( ! sscanf(&(source[*index]), "%d", &(i->argument.num)) ){
        puts("Parse_number: failed to read in number");
        free(i);
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

struct Instruction * parse_print(char *source, size_t *index){
    struct Instruction *ret = 0;
    struct Instruction *i = 0;

    i = new_instruction(PRINT);
    if( ! i ){
        puts("Parse_print: call to new_instruction failed");
        return 0;
    }

    /* w/string/ */
    switch( source[*index] ){
        case 'p':
        case 'P':
            ++(*index);
            break;

        default:
            printf("Parse_print: unexpected character '%c', expected 'w'\n", source[*index]);
            free(i);
            return 0;
            break;
    }

    /* print has 2 different forms
     *  p127
     *  p
     * if next character is a number then we are in the first form
     */
    if( isdigit(source[*index]) ){
        ret = parse_number(i, source, index);
        if (ret == 0)
            free(ret);
        return ret;
    }

    /* otherwise there is no number and we are in second form (default to 100 bytes) */
    return i;
}

struct Instruction * parse_byte(char *source, size_t *index){
    struct Instruction *ret = 0;
    struct Instruction *i = 0;

    i = new_instruction(BYTE);
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
            free(i);
            return 0;
            break;
    }

    ret = parse_number(i, source, index);
    if (ret == 0)
        free(ret);

    return ret;
}

struct Instruction * parse_line(char *source, size_t *index){
    struct Instruction *i = 0;

    i = new_instruction(LINE);
    if( ! i ){
        puts("Parse_line: call to new_instruction failed");
        return 0;
    }

    free(i);

    puts("parse_line unimplemented");
    return 0; /* FIXME unimplemented */
}

struct Instruction * parse_expect(char *source, size_t *index){
    struct Instruction *ret = 0;
    struct Instruction *i = 0;

    i = new_instruction(EXPECT);
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
            free(i);
            return 0;
            break;
    }

    ret = parse_string(i, source, index);
    if (ret == 0)
        free(i);

    return ret;
}

struct Instruction * parse_write(char *source, size_t *index){
    struct Instruction *ret = 0;
    struct Instruction *i = 0;

    i = new_instruction(WRITE);
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
            free(i);
            return 0;
            break;
    }
    ret = parse_string(i, source, index);
    if (ret == 0)
        free(i);

    return ret;
}

struct Instruction * parse_quit(char *source, size_t *index){
    struct Instruction *i = 0;

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


    i = new_instruction(QUIT);
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
                goto EXIT;
                break;

            default:
                ++(*index);
                break;

        }
    }

EXIT:

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
    size_t len = 0;
    /* result from call to parse_ functions */
    struct Instruction *res = 0;
    /* place to store next parsed Instruction */
    struct Instruction **store = 0;
    /* temporary pointer to program->source */
    char *source = 0;

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

/* eval PRINT command
 * print specified number of bytes
 * defaults to 100 bytes if number isn't specified
 *
 *  p
 *  p127
 *
 * uses cur->argument.num
 *
 * returns 0 on success
 * returns 1 on failure
 * failure will cause program to halt
 */
int eval_print(struct Program *p, struct Instruction *cur){
    /* number of bytes to read */
    int num = cur->argument.num;
    char *buf = 0;
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

/* eval BYTE command
 * move spcified number of bytes 'forward' through text
 *
 *  b427
 *
 * uses cur->argument.num
 *
 * returns 0 on success
 * returns 1 on failure
 * failure will cause program to halt
 */
int eval_byte(struct Program *p, struct Instruction *cur){
    /* byte number argument to seek to */
    int byte = 0;

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

/* eval EXPECT command
 * check current location matches specified string
 * throws error if string does not match
 *
 *  e/hello/
 *
 * uses cur->argument.str
 *
 * returns 0 on success
 * returns 1 on failure
 * failure will cause program to halt
 */
int eval_expect(struct Program *p, struct Instruction *cur){
    /* string to compare to */
    char *str = 0;
    /* length of string */
    size_t len = 0;
    /* buffer read into */
    char *buf = 0;
    /* num bytes read */
    size_t nr = 0;

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

/* eval WRITE command
 * write specified string
 * will overwrite existing text in place
 *
 *  w/world/
 *
 * uses cur->argument.str
 *
 * returns 0 on success
 * returns 1 on failure
 * failure will cause program to halt
 */
int eval_write(struct Program *p, struct Instruction *cur){
    /* string to write */
    char *str = 0;
    /* len of str */
    size_t len = 0;
    /* number of bytes written */
    size_t nw = 0;

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
    struct Instruction *cur = 0;
    /* return code from individual eval_ calls */
    int ret = 0;

    if( !p ){
        puts("Execute called with null program");
        return 1;
    }

    /* simple dispatch function */
    for( cur = p->start; cur; cur = cur->next ){
        switch( cur->command ){
            case PRINT:
                ret = eval_print(p, cur);
                if( ret ){
                    return ret;
                }
                break;

            case LINE:
                ret = eval_line(p, cur);
                if( ret ){
                    return ret;
                }
                break;

            case BYTE:
                ret = eval_byte(p, cur);
                if( ret ){
                    return ret;
                }
                break;

            case EXPECT:
                ret = eval_expect(p, cur);
                if( ret ){
                    return ret;
                }
                break;

            case WRITE:
                ret = eval_write(p, cur);
                if( ret ){
                    return ret;
                }
                break;

            case QUIT:
                /* escape from loop */
                goto EXIT;
                break;

            default:
                puts("Invalid command type encountered in execute");
                return 1;
                break;
        }
    }

    /* implicit (EOF) or explicit (Command quit) => exit */
EXIT:

    return 0;
}


/* frees the elements of the linked list of instructions
 * allocated while parsing
 */
void scrub(struct Program *p)
{
    struct Instruction *now = 0;
    struct Instruction *next = 0;
    if ( p->start ){
        now = p->start;
        do {
            next = now->next;
            free(now);
            now = next;
        } while( next );
    }
    p->start = NULL;
}

int repl(struct Program *p){
    int exit_code = EXIT_FAILURE;
    char line[4096]; /* FIXME: Perhaps use slurp-like behaviour instead */

    /* FIXME: doesn't handle quit command */
    while( 1 ){
        printf("dodo: ");
        p->source = fgets(line, sizeof(line), stdin);

        if ( ! p->source ){
            printf("fgets failed in repl\n");
            exit_code = feof(stdin)? EXIT_SUCCESS : EXIT_FAILURE;
            goto EXIT;
        }

        /* note we don't error-out on parse or execute,
         * keep the repl rolling */
        parse(p);
        execute(p);

        scrub(p);
    }

EXIT:
    /* force null to stop free() */
    p->source = NULL;
    return exit_code;
}



/***** main *****/
void usage(void){
    puts("dodo - scriptable in place file editor\n"
         "In non-interactive mode, dodo takes a single argument of <filename>\n"
         "and will read commands from stdin\n"
         "\n"
         "example:\n"
         "  dodo [-i|--interactive] <filename> <<EOF\n"
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
    int exit_code = EXIT_SUCCESS;
    int do_repl = 0;
    struct Program p = {0};

    if(    argc < 2
        || argc > 3
        || !strcmp("--help", argv[1])
        || !strcmp("-h", argv[1])
    ){
        usage();
        exit(EXIT_FAILURE);
    }

    /* catch 'interactive' command line argument */
    if (    argc == 3
         && (!strcmp("--interactive", argv[1])
         || !strcmp("-i", argv[1]))){
        do_repl = 1;
    }

    /* one-shot read and execute if we're not heading into the repl */
    if ( ! do_repl )
    {
        /* read program into source */
        p.source = slurp(stdin);
        if( ! p.source ){
            puts("Reading program failed");
            exit_code = EXIT_FAILURE;
            goto EXIT;
        }

        /* parse program */
        if( parse(&p) ){
            puts("Parsing program failed");
            exit_code = EXIT_FAILURE;
            goto EXIT;
        }
    }

    /* open file */
    p.file = fopen(argv[1 + do_repl], "r+b");
    if( ! p.file ){
        printf("Failed to open specified file '%s'\n", argv[1 + do_repl]);
        exit_code = EXIT_FAILURE;
        goto EXIT;
    }

    if ( do_repl ) {
        /* execute the repl */
        repl(&p);
    } else {
        /* execute program */
        if( execute(&p) ){
            puts("Program execution failed");
            exit_code = EXIT_FAILURE;
            goto EXIT;
        }
    }

EXIT:

    scrub(&p);

    if( p.buf ){
        free(p.buf);
    }

    if( p.file ){
        fclose(p.file);
    }

    if( p.source ){
        free(p.source);
    }

    exit(exit_code);
}

