/* A program to count the characters, words and lines in a document*/

/*declarations of variables and regular definitions required for the
lexical recognizer*/

%{
  unsigned charCount = 0, wordCount = 0, lineCount = 0;
%} /*everything in these braces are copied into lex.yy.c verbatim*/

word [^ \t\n]+
eol  \n

%%
{word} { wordCount++; charCount += yyleng; }
{eol}  { charCount++; lineCount++; }
.      { charCount++; } /*for everything else, increment the character 
                          count*/

%%
/*auxiliary procedures*/

/*lex.yy.c usually reads its input from stdin.  however, I would prefer it if lex.yy.c would read its input from a file passed as a command lineargument.  Hence I am declaring my own main function*/

main(argc,argv)
int argc;
char **argv;
{
  if (argc > 1) { /*open the file*/
    FILE *file;

    file = fopen(argv[1], "r");
    if (!file) {
      fprintf(stderr,"could not open %s\n",argv[1]);
      exit(1);
    }
    yyin = file; /*set lex's input stream (yyin) to the file.*/
  }
  yylex(); /*run the lexical recognizer*/
  printf("%u %u %u\n",charCount, wordCount, lineCount);
  /*output the results*/
  return 0;
}


