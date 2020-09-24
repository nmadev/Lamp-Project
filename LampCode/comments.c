#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

enum state {BEGIN, ALL};
enum reading {NONE, CBLOCK, COMMENT, QUOTE};
enum comment_type {NA, SINGLE, MULTI};
enum line_loc {BEGINNING, MIDDLE};
enum tag_status {IN, OUT};

#define ERROR (stderr)

int main(int argc, char *argv[])
{
    enum state current = BEGIN;
    
    // reads in command line arguments and determines input and output stream
    // as well as if command lines arguments are feasible and which mode to
    // print comment tags 
    for (int i = 0; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (argv[i][1] == 'a')
            {
                current = ALL;
            }
            else if (argv[i][1] == 'l')
            {
                current = BEGIN;
            }
            else if (argv[i][1] == 'o' || argv[i][1] == 'i')
            {
                if (i + 1 < argc)
                {
                    if (argv[i][1] == 'o')
                    {
                        stdout = fopen(argv[i + 1], "w");
                    }
                    else if (argv[i][1] == 'i')
                    {
                        if (fopen(argv[i + 1], "r") == 0)
                        {
                            fprintf(ERROR, "Input file does not exist");
                            return 1;
                        }
                        stdin = fopen(argv[i + 1], "r");
                    }
                    i++;
                }
                else
                {
                   fprintf(ERROR, "No input file given");
                   return 1;
                }
            }
            else
            {
                fprintf(ERROR, "-%c: command does not exist", argv[i][1]);
                return 1;
            }
        }
    }
    
    // initializes a multi-level state machine to track multiple states concurrently
    enum reading status = NONE;
    enum comment_type comm = NA;
    enum line_loc location = BEGINNING;
    int bracket = 0;
    int backslash = 0;
    enum tag_status tag_loc = OUT;
    
    // reads in a file char by char
    char read;
    char last_read = 'f';
    char two_ago = 'e';
    char three_ago = 'd';

    while (fscanf(stdin, "%c", &read) != EOF)
    {
        // starts a consecutive backslash counter
        if (last_read == '\\')
        {
            backslash++;
        }
        else
        {
            backslash == 0;
        }
        switch(status)
        {
            // state when not reading in cblock, comment, or quotation
            // moves into one of the reading states once detected
            case NONE:
                if (read == '\n' && last_read == '\\')
                {
                    last_read = three_ago;
                    read = two_ago;
                    break;
                }
                if (last_read == '/' && read == '/')
                {
                    comm = SINGLE;
                    status = COMMENT;
                    location = BEGINNING;
                }
                else if (last_read == '/' && read == '*')
                {
                    comm = MULTI;
                    status = COMMENT;
                    location = BEGINNING;
                }
                else if (read == '{')
                {
                    bracket++;
                    status = CBLOCK;
                }
                else if (read == '\"')
                {
                    status = QUOTE;
                }
                break;

            // handles comment parsing, the most complicated of the states
            case COMMENT:

                // handles case of the end of a multiline comment
                if (read == '/' && last_read == '*' && comm == MULTI)
                {
                    if (tag_loc == IN)
                    {
                        fprintf(stdout, "\n");
                    }
                    status = NONE;
                    location = BEGINNING;
                    comm = NA;
                    tag_loc = OUT;
                    break;
                }

                // determines if a tag is in the middle of a comment or at the beginning
                if (location == BEGINNING && (isspace(read) == 0 && read != '*' && read != '@') && tag_loc == OUT)
                {
                    location = MIDDLE;
                }

                // resets comment to the beginning of a line if multiline or returns to reading state if
                // a single line comment and new line is detected
                if (location == MIDDLE && current == BEGIN && tag_loc == OUT)
                {
                    if (read == '\n' && comm == SINGLE && last_read != '\\')
                    {
                        status = NONE;
                        location = BEGINNING;
                    }
                    else if (read == '\n' && comm == MULTI && last_read != '\\')
                    {
                        location = BEGINNING;
                    }
                    break;
                }

                // determines if a tag has begun
                else if (read == '@' && tag_loc == OUT && (isspace(last_read) != 0 || last_read == '*' || location == BEGINNING))
                {
                    tag_loc = IN;
                    fprintf(stdout, "@");
                    break;
                }

                // when not in a tag, can reset a multiline comment to the beginning of a line
                // or can exit to reading status with a new line
                if (tag_loc == OUT)
                {
                    if (read == '\n' && last_read != '\\')
                    {
                        location = BEGINNING;
                        if (comm == SINGLE)
                        {
                            status = NONE;
                        }
                    }
                    break;
                }

                // handles when reading in a valid tag
                else if (tag_loc == IN)
                {
                    // handles the case of a multiline comment end in a tag
                    if (read == '*' && comm == MULTI)
                    {
                        break;
                    }

                    // handles when double backslashes are read
                    if (read == '\\')
                    {
                        if (last_read == '\\')
                        {
                            fprintf(stdout, "\\");
                            read = two_ago;
                            last_read = three_ago;
                        }
                        break;
                    }

                    // prints out read charachter unless it is a single backslash
                    if (isspace(read) == 0)
                    {
                        if (last_read == '\\')
                        {
                            fprintf(stdout, "\\");
                        }
                        fprintf(stdout, "%c", read);
                    }

                    // handles new lines in comment in a comment
                    else
                    {
                        if (read == '\n')
                        {
                            if (last_read == '\\')
                            {
                                break;
                            }
                            else if (comm == SINGLE)
                            {
                                fprintf(stdout, "\n");
                                status = NONE;
                                location = BEGINNING;
                                tag_loc = OUT;
                                break;
                            }
                            else if (comm == MULTI)
                            {
                                fprintf(stdout, "\n");
                                location = BEGINNING;
                                tag_loc = OUT;
                                break;
                            }
                        }
                        else
                        {
                            tag_loc = OUT;
                            location = MIDDLE;
                            fprintf(stdout, "\n");
                        }
                    }
                }
                break;

            // handles case of being in a c-block of code
            case CBLOCK:

                // determines if reading in a comment in a c-block
                if (read == '/' && last_read == '/')
                {
                    comm = SINGLE;
                    tag_loc = IN;
                }
                else if (read == '*' && last_read == '/')
                {
                    comm = MULTI;
                    tag_loc = IN;
                }

                // if in a comment, disregards all characters read in until the end of the comment
                if (tag_loc == IN)
                {
                    switch (comm)
                    {
                        case SINGLE:
                            if (read == '\n')
                            {
                                if (last_read == '\\' && backslash % 2 == 1)
                                {
                                    break;
                                }
                                tag_loc = OUT;
                            }
                            break;

                        case MULTI:
                            if (read == '/' && last_read == '*')
                            {
                                tag_loc = OUT;
                            }
                            break;
                    }
                    break;
                }

                // if not in a comment, counts the open and closed brackets to
                // determine where the c-block ends
                else if (tag_loc == OUT)
                {
                    if (read == '{')
                    {
                        bracket++;
                    }
                    else if (read == '}')
                    {
                        bracket--;
                    }
                    if (bracket == 0)
                    {
                        status = NONE;
                    }
                }
                break;

            // runs if in a string
            case QUOTE:

                // ignores an escaped quotation mark, otherwise passes
                if (read == '\"' && last_read != '\\')
                {
                    status = NONE;
                }
                break;

        }

        // set variables to hold the 3 past read characters
        three_ago = two_ago;
        two_ago = last_read;
        last_read = read;
    }

    // if exited in a tag, adds a new line to the end of the output
    if (tag_loc == IN)
    {
        fprintf(stdout, "\n");
    }
}
