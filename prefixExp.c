/* prefixExp.c, Gerard Renardel, 29 January 2014
 *
 * In this file functions are defined for the construction of expression trees
 * from prefix expressions generated by the following BNF grammar:
 *
 * <prefexp>   ::= <number> | <identifier> | '+' <prefexp> <prefexp> 
 *             | '-' <prefexp> <prefexp> | '*' <prefexp> <prefexp> | '/' <prefexp> <prefexp> 
 * 
 * <number>      ::= <digit> { <digit> }
 *
 * <identifier> ::= <letter> { <letter> | <digit> }
 *
 * Starting point is the token list obtained from the scanner (in scanner.c).
 */

#include <stdio.h>  /* printf */
#include <stdlib.h> /* malloc, free */
#include <assert.h> /* assert */
#include "scanner.h"
#include "recognizeExp.h"
#include "prefixExp.h"

/* The function newExpTreeNode creates a new node for an expression tree.
 */

ExpTree newExpTreeNode(TokenType tt, Token t, ExpTree tL, ExpTree tR) {
  ExpTree new = malloc(sizeof(ExpTreeNode));
  assert (new!=NULL);
  new->tt = tt;
  new->t = t;
  new->left = tL;
  new->right = tR;
  return new;
}

/* The function valueIdentifier recognizes an identifier in a token list and
 * makes the second parameter point to it.
 */

int valueIdentifier(List *lp, char **sp) {
  // printf("valueIdentifier check : Char = %c , List = ", *sp[0]);
  // printList(*lp);
  if (*lp != NULL && (*lp)->tt == Identifier ) {
    *sp = ((*lp)->t).identifier;
    *lp = (*lp)->next;
    return 1;
  }
  return 0;
}

// acceptNumber, except that this function writes the value to wp.
int valueNumber(List *lp, double *wp) {
    if (*lp != NULL && (*lp)->tt == Number ) {
        *wp = ((*lp)->t).number;
        *lp = (*lp)->next;
        return 1;
    }
    return 0;
}

/* The function valueOperator recognizes an arithmetic operator in a token list
 * and makes the second parameter point to it.
 * Here the auxiliary function isOperator is used.
 */
int isDivMultOperator(char c){
  return (c == '/' || c == '*');
}

int acceptDivisionMultiplication(List *lp, char *cp){
  if (*lp != NULL && (*lp)->tt == Symbol && isDivMultOperator(((*lp)->t).symbol) ) {
    *cp = ((*lp)->t).symbol;
    *lp = (*lp)->next;
    return 1;
  }
  return 0;
}

int isPlusMinOperator(char c){
  return (c == '+' || c == '-');
}

int acceptAdditionSubstraction(List *lp, char *cp){
  if (*lp != NULL && (*lp)->tt == Symbol && isPlusMinOperator(((*lp)->t).symbol) ) {
    *cp = ((*lp)->t).symbol;
    *lp = (*lp)->next;
    return 1;
  }
  return 0;  
}

/* De functie freeExpTree frees the memory of the nodes in the expression tree.
 * Observe that here, unlike in freeList, the strings in indentifier nodes
 * are not freed. The reason is that the function newExpTree does not allocate
 * memory for strings in nodes, but only a pointer to a string in a node
 * in the token list.
 */

void freeExpTree(ExpTree tr) {
  if (tr==NULL) {
    return;
  }
  freeExpTree(tr->left);
  freeExpTree(tr->right);
  free(tr);
}

/* The function treeExpression tries to build a tree from the tokens in the token list 
 * (its first argument) and makes its second argument point to the tree.
 * The return value indicates whether the action is successful.
 * Observe that we use ordinary recursion, not mutual recursion.
 */

/*  
 * <expression>  ::= <term> { '+'  <term> | '-' <term> }
 * 
 * <term>       ::= <factor> { '*' <factor> | '/' <factor> }
 *
 * <factor>     ::= <number> | <identifier> | '(' <expression> ')'
*/

int factorNode(List *lp, ExpTree *tree){
    if((*lp) == NULL ) return 0;
    switch((*lp)->tt) {
      case Number:
        *tree = newExpTreeNode(Number, (*lp)->t, NULL, NULL);
            (*lp) = (*lp)->next;
            return 1;
      case Identifier:
//        valueIdentifier(lp, &c);
        *tree = newExpTreeNode(Identifier, (*lp)->t, NULL, NULL);
            *lp = (*lp)->next;
            return 1;
      case Symbol:
        if ((*lp)->t.symbol == '(') {
          acceptCharacter(lp, '(') && treeInfixExpression(lp, tree) && acceptCharacter(lp, '(');
          return 1;
        }
        return 0;
    }

  return 0;
}

// accept factor, accept *or/, accept factor
int termNode(List *lp, ExpTree *tree, int count){
  ExpTree leftTree, rightTree;
  char operator;
  Token newToken;
  if(count == 0) {
    if (!factorNode(lp, &leftTree)) return 0;
    //  add new node to tree
    *tree = leftTree;
  }

  if(!acceptDivisionMultiplication(lp, &operator)) return (count>0);
  if(factorNode(lp, &rightTree)){
    newToken.symbol = operator;
    *tree = newExpTreeNode(Symbol,newToken,*tree,rightTree);
// check if next operator is /or*, if it is call termNode(with counter++, and tree)
//    This check might not work properly.
    if(isDivMultOperator(((*lp)->next->t.symbol))){
      termNode(lp, tree, ++count);
    }
  } else {
    return (count>0);
  }
// add node to tree

  return 1;


}

int infixResult = 1;
int treeInfixExpression(List *lp, ExpTree *tp){
  char operator;
  char *variable;
  double number;
  Token t;
  ExpTree treeLeft, treeRight;
}





int prefixResult = 1;
int treePrefixExpression(List *lp, ExpTree *tp) { 
  double w;
  char *s;
  char c;
  Token t;
  ExpTree tL, tR;

  // This while-loop will have to create an expression tree tp.

  while (*lp != NULL){
    printf("Initial : ");
    printList(*lp);
    if ( valueNumber(lp,&w) ) {
      // printf("Found Number : %d\n", (int)w);
      t.number = (int)w;
      *tp = newExpTreeNode(Number, t, NULL, NULL);
    }
    else if ( valueIdentifier(lp,&s) ) {
      t.identifier = s;
      *tp = newExpTreeNode(Identifier, t, NULL, NULL);
      printf("\nFound variable --> Not numerical!\n\n");
      prefixResult = 2;
    }
    else if (valueOperator(lp, &c)){ 
      if (treePrefixExpression(lp,&tR)){
        if (treePrefixExpression(lp, &tL)){
          printf("symbol : %c\n", c);
          t.symbol = c;
          *tp = newExpTreeNode(Symbol, t, tL, tR);
        }
      } else { /* without 'else' the program works fine, but there is a memory leak */
        printf("\nShould probably not get here?\n\n");
        freeExpTree(tL);
        return 0;
      }
    } 
  }
  printf("\n");
  printExpTreeInfix(*tp);
  printf("\n");
  printf("Number   when returned : %d\n", (int)w);
  printf("Operator when returned : %c\n", c);
  //printf("Returned %d\n\n", prefixResult);
  return prefixResult;
}

/* The function printExpTreeInfix does what its name suggests.
 */

void printExpTreeInfix(ExpTree tr) {
  if (tr == NULL) {
    return;
  }
  switch (tr->tt) {
  case Number: 
    printf("%d",(tr->t).number);
   break;
  case Identifier: 
    printf("%s",(tr->t).identifier);
    break;
  case Symbol: 
    printf("(");
    printExpTreeInfix(tr->left);
    printf(" %c ",(tr->t).symbol);
    printExpTreeInfix(tr->right);
    printf(")");
    break;
  }
}

/* The function isNumerical checks for an expression tree whether it represents 
 * a numerical expression, i.e. without identifiers.
 */

int isNumerical(ExpTree tr) {
  if (tr == NULL){
    return 0;
  }
  if (tr->tt==Number) {
    return 1;
  }
  if (tr->tt==Identifier) {
    return 0;
  }
  return (isNumerical(tr->left) && isNumerical(tr->right));
}

/* The function valueExpTree computes the value of an expression tree that represents a
 * numerical expression.
 */

double valueExpTree(ExpTree tr) {  /* precondition: isNumerical(tr)) */
  double lval, rval;
  assert(tr!=NULL);
  if (tr->tt==Number) {
    return (tr->t).number;
  }
  lval = valueExpTree(tr->left);
  rval = valueExpTree(tr->right);
  switch ((tr->t).symbol) {
  case '+':
    return (lval + rval);
  case '-':
    return (lval - rval);
  case '*':
    return (lval * rval);
  case '/':
    assert( rval!=0 );
    return (lval / rval);
  default:
    abort();
  }
}

/* the function prefExpressionExpTrees performs a dialogue with the user and tries
 * to recognize the input as a prefix expression. When it is a numerical prefix 
 * expression, its value is computed and printed.
 */ 

void prefExpTrees() {
  char *ar;
  List tl, tl1;  
  ExpTree t = NULL; 
  printf("give a prefix expression: ");
  ar = readInput();
  while (ar[0] != '!') {
    tl = tokenList(ar); 
    printf("the token list is ");
    printList(tl);
    tl1 = tl;
    printf("\n-------------Check1-------------\n");
    int valid = treePrefixExpression(&tl1, &t);
    printf("-------------Check2-------------\n");
    printf("valid = %d", valid);
    if ( valid > 0 && tl1 == NULL ) { 
         /* there should be no tokens left */
      printf("\n-------------Check3-------------\n");  
      if (valid == 2){
        printf("\nNot numerical!\n");
        t = NULL;
      }
      printf("in infix notation: ");
      printExpTreeInfix(t);
      printf("\n");
      if ( isNumerical(t) ) {
        printf("the value is %g\n",valueExpTree(t));
      } else {
        printf("this is not a numerical prefix expression\n");
      }
    } else {
      printf("this is not a prefix expression\n"); 
    }
    freeExpTree(t);
    t = NULL; 
    freeTokenList(tl);  
    free(ar);
    printf("\ngive a prefix expression: ");
    ar = readInput();
  }
  free(ar);
  printf("good bye\n");
}
