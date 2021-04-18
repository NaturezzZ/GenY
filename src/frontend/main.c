#include <stdio.h>
#include "y.tab.h"
#include <stdlib.h>
//#include "typedefine.h"

extern FILE*yyin;
extern FILE*yyout;
/*extern TreeNode* node;
void init(){
	for(int i=0;i<MAXTREENODE;i++){
		node[i].childcnt = 0;
		node[i].sibling = NULL;
	}
}*/

int main(int argc, char *argv[])
{
	/*if(argc < 2){
		printf("Usage: eeyore <input> <output>");
		exit(1);
	}*/
	//FILE* fp1 = fopen(argv[1],"r");
	
	//yyin = stdin;
	//yyout = stdout;
	
	//init();

	yyparse();
	return 0;
}
