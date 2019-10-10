#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int main( int argc, char *argv[]) {

   char *str = (char *)malloc(sizeof(char) * 1024);
   char s[100];
   int c;
   int d;

   printf( "Enter a value : ");
   scanf("%s",str);
   fgets(s,100,stdin);
   //sscanf(s,"%d",&c);
   //sscanf(s,"%d",&c);
   printf("%d\n",atoi(s) > 0);
   printf("%d\n",sscanf(s,"%[^\n]%*c", str));
   printf("%s",str+3);
   // if(stdin == NULL){
   //    printf("empty\n");
   // }
   // else{
   //    printf("not empty\n");
   // }

   //printf( "\nYou entered: %s\n", str);
   // printf("%s\n",s);
   // printf("%d\n",s[1]=='\n');
   // printf("%d\n",d);

   return 0;
}