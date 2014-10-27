#include <stdio.h> /* fopen, fseek, fread, fwrite, FILE */
#include <stdlib.h> /* exit */
#include <string.h> /* strcmp, strncmp */

#define QUIT_EXIT_CODE EXIT_SUCCESS
#define EXPECT_EXIT_CODE EXIT_FAILURE

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

union Argument {
    int num;
    char *str;
};

struct Instruction {
    /* type of command determines dispatch to eval_ function
     * which in turn determines which Argument member to use
     */
    enum Command command;
    union Argument argument;
    /* next Instruction in linked list */
    struct Instruction *next;
};

struct Program {
    /* linked list of Instruction(s) */
    struct Instruction *start;
    /* file program is operating on */
    FILE *file;
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



/***** parsing functions *****/

struct Instruction * parse_print(){
    struct Instruction *i;

    i = new_instruction(print);
    if( ! i ){
        puts("Parse_print: call to new_instruction failed");
        return 0;
    }

    puts("parse_print unimplemented");
    return 0; /* FIXME unimplemented */
}

struct Instruction * parse_byte(){
    struct Instruction *i;

    i = new_instruction(byte);
    if( ! i ){
        puts("Parse_byte: call to new_instruction failed");
        return 0;
    }

    puts("parse_byte unimplemented");
    return 0; /* FIXME unimplemented */
}

struct Instruction * parse_line(){
    struct Instruction *i;

    i = new_instruction(line);
    if( ! i ){
        puts("Parse_line: call to new_instruction failed");
        return 0;
    }

    puts("parse_line unimplemented");
    return 0; /* FIXME unimplemented */
}

struct Instruction * parse_expect(){
    struct Instruction *i;

    i = new_instruction(expect);
    if( ! i ){
        puts("Parse_expect: call to new_instruction failed");
        return 0;
    }

    puts("parse_expect unimplemented");
    return 0; /* FIXME unimplemented */
}

struct Instruction * parse_write(){
    struct Instruction *i;

    i = new_instruction(write);
    if( ! i ){
        puts("Parse_write: call to new_instruction failed");
        return 0;
    }

    puts("parse_write unimplemented");
    return 0; /* FIXME unimplemented */
}

struct Instruction * parse_quit(){
    struct Instruction *i;

    i = new_instruction(quit);
    if( ! i ){
        puts("Parse_quit: call to new_instruction failed");
        return 0;
    }

    puts("parse_quit unimplemented");
    return 0; /* FIXME unimplemented */
}

/* consume comment from source
 * return 0 on success
 * return 1 on error
 */
int parse_comment(char *source, int *index){

    if( source[*index] != '#' ){
        printf("Parse_comment: expected '#', got '%c'\n", source[*index]);
        return 1;
    }

    /* consume source until \n or \0 are found */
    while( 1 ){
        switch( source[*index] ){

            case '\n':
            case '\0':
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
int parse(char *source, struct Program *program){
    /* index into source */
    int index = 0;
    /* length of source */
    int len;
    /* result from call to parse_ functions */
    struct Instruction *res;
    /* place to store next parsed Instruction */
    struct Instruction **store;

    if( ! source ){
        puts("Parse called with null source");
        return 1;
    }

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
                res = parse_quit(source, &index);
                if( ! res ){
                    puts("Parse: failed in call to parse_quit");
                    return 1;
                }
                *store = res;
                store = &(res->next);
                break;

            case '#':
                if( parse_comment(source, &index) ){
                    puts("Parsing comment failed");
                    return 1;
                }
                break;

            default:
                printf("Parse: Invalid character encountered '%c'\n", source[index]);
                return 1;
                break;
        }
    }

    /* null terminator for program */
    *store = 0;

    return 0;
}




/***** evaluation functions *****/

int eval_print(struct Program *p, struct Instruction *cur){
    /* number of bytes to read */
    int num = cur->argument.num;
    /* buffer to read into */
    char *buf;
    /* number of bytes read */
    int nr = 0;

    /* default to 100 bytes */
    if( ! num ){
        num = 100;
    }

    /* allocate buffer
     * 1 + num to fit num bytes and null
     */
    buf = calloc(1 + num, 1);
    if( ! buf ){
        puts("Call to calloc failed in eval_print");
        return 1;
    }

    /* read into buffer */
    nr = fread(buf, 1, num, p->file);
    /* make sure buffer is really a string */
    buf[nr] = '\0';

    /* print buffer, as instructed */
    printf("'%s'\n", buf);

    free(buf);

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
    int len;
    /* buffer read into */
    char *buf;
    /* num bytes read */
    int nr;

    str = cur->argument.str;
    if( ! str ){
        puts("Eval_expect: no string argument found");
        return 1;
    }

    len = strlen(str);

    /* allocate buffer
     *1 + len to fit len bytes + null terminator
     */
    buf = calloc(1 + len, 1);
    if( ! buf ){
        puts("Eval_expect: calloc call failed");
        return 1;
    }

    /* perform read */
    nr = fread(buf, 1, len, p->file);
    buf[nr] = '\0';

    /* compare number read to expected len */
    if( nr != len ){
        /* FIXME consider output when expect fails */
        printf("Eval_expect: expected to read '%d' bytes, actually read '%d'\n", len, nr);
        return 1;
    }

    /* compare read string to expected str */
    if( strcmp(str, buf) ){
        /* FIXME consider output when expect fails */
        printf("Eval_expect: expected string '%s', got '%s'\n", str, buf);
        return 1;
    }

    free(buf);

    return 0;
}

int eval_write(struct Program *p, struct Instruction *cur){
    /* string to write */
    char *str;
    /* len of str */
    int len;
    /* number of bytes written */
    int nw;

    str = cur->argument.str;
    if( ! str ){
        puts("Eval_write: no argument string found");
        return 1;
    }

    len = strlen(str);

    /* perform write */
    nw = fwrite(str, 1, len, p->file);

    /* check length */
    if( nw != len ){
        printf("Eval_write: expected to write '%d' bytes, actually wrote '%d'\n", len, nw);
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
    puts("dodo takes a single argument of <filename>\n"
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

char * slurp(FILE *file){
    return 0; /* FIXME unimplemented */
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

    // read program into source
    char *source = slurp(stdin);
    if( !source ){
        puts("Reading program failed");
        exit(EXIT_FAILURE);
    }

    // parse program
    if( parse(source, &p) ){
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

    fclose(p.file);
    exit(EXIT_SUCCESS);
}

