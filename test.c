#include <stdio.h>
#include <string.h>
int main( int argc, char *argv[]) {

   char str[100];
   char s[100];
   int c;
   int d;

   printf( "Enter a value : ");
   scanf("%s",str);
   fgets(s,100,stdin);
   //sscanf(s,"%d",&c);
   if(strchr(s, 0) == NULL){
       printf("Not Found");
   }
   //sscanf(s,"%d",&c);
   printf("%d\n",atoi(s) > 0);
   printf("%d\n",sscanf(s,"%d",&c));
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