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
};

struct Program {
    /* linked list of Instruction(s) */
    struct Instruction *start;
    /* file program is operating on */
    FILE *file;
};



/***** parsing functions *****/
/* parse provided source into Program
 * return 0 on success
 * return 1 on failure
 */
int parse(struct Program *p, FILE *source){
    return 1; /* FIXME unimplemented */
}



/***** evaluation functions *****/
/* execute provided Program
 * return 0 on success
 * return 1 on failure
 */
int execute(struct Program *p){
    return 1; /* FIXME unimplemented */
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
         "  ln        # goto line <n> of file\n"
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

    // parse program
    if( parse(&p, stdin) ){
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

