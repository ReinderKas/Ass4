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
#include "infixExp.h"

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
  // printf("\nEvaluated %c\n", c);
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
        *tree = newExpTreeNode(Identifier, (*lp)->t, NULL, NULL);
        *lp = (*lp)->next;
        return 1;
      case Symbol:
        if ((*lp)->t.symbol == '(') {
          if (acceptCharacter(lp, '(') && expressionNode(lp, tree, 0) && acceptCharacter(lp, ')'));
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

  if(count == -1){
    leftTree = *tree;
    count = 1;
  }

  if(count == 0) {
    if (!factorNode(lp, &leftTree)) return 0;
    //  add new node to tree
    *tree = leftTree;
  }

  if(acceptDivisionMultiplication(lp, &operator)) {
    if(factorNode(lp, &rightTree)){
      newToken.symbol = operator;
      *tree = newExpTreeNode(Symbol,newToken,*tree,rightTree);
//    check if next operator is /or*, if it is call termNode(with counter++, and tree)
//    This check might not work properly.
      if(*lp != NULL && isDivMultOperator((*lp)->t.symbol)){
        // printList(*lp);
        termNode(lp, tree, -1);
      }
    } else {
      return (count>0);
    }
  }
  
// add node to tree
  return 1;
}

// accept term, accept *or/, accept factor
int expressionNode(List *lp, ExpTree *tree, int count){
  ExpTree leftTree, rightTree;
  char operator;
  Token newToken;

  if(count == -1){
    // This means that the given tree is the left portion of the new tree.
    leftTree = *tree;
    count = 1;
  }
  
  if(count == 0) {
    if (!termNode(lp, &leftTree, 0)) return 0;
    //  add new node to tree
    *tree = leftTree;
  }
  if(acceptAdditionSubstraction(lp, &operator)) {
    if(termNode(lp, &rightTree, 0)){
      newToken.symbol = operator;
      *tree = newExpTreeNode(Symbol,newToken,*tree,rightTree);
//    check if next operator is /or*, if it is call termNode(with counter++, and tree)
//    This check might not work properly.
      if(*lp != NULL && isPlusMinOperator((*lp)->t.symbol)){
        expressionNode(lp, tree, -1);
      }
    } else {
      return (count>0);
    }
  }
// add node to tree
  return 1;
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

// We assume that this function will only be called if it is certain that either A or B is equal to checkValue
int giveCorrectValue(int inputA, int inputB, int checkValue){
  return (inputA == checkValue) ? inputB : inputA;
}


/* The function Simplify prepares the Expression Tree for taking its Derivative according to the rules:
   0∗E =================
   E∗0  =================
   are simplified to 0;
   
   
   0+E ================= 
   E+0 ================= 
   E−0 
   1∗E ================= 
   E∗1 =================
   E/1 =================
   are simplified to E.

   @@@@@@@@@@@ Currently not taking in consideration that E might also be a variable!!! @@@@@@@@@@@
   */

ExpTree simplify(ExpTree tree){
  // Should go through the entire tree recursively, applying the rules if possible.
  // Start at the bottom of the tree (again);
  ExpTree newLeft = (*tree).left;
  ExpTree newRight = (*tree).right;

  if ((*tree).left != NULL && (*tree).right != NULL){
    // Not at the bottom yet. Go down recursively.
    newRight = simplify((*tree).right);
    newLeft = simplify((*tree).left);
  } else {
    // Currently at the bottom Node in the tree.
    return tree;
  }

  if ((*tree).tt == Symbol){
    Token t;
    switch ((*tree).t.symbol){
      case '*':
        printf("\n");
        printf("-------------New Iteration-------------\n");
        printf("List = ");
        printExpTreeInfix(tree);
        printf("\n");
        printf("Left = %d , right = %d\n", ((*tree).left)->t.number, ((*tree).right)->t.number);
        if (((*tree).right)->t.number == 0 || ((*tree).left)->t.number == 0){
          t.number = 0;
          return newExpTreeNode(Number, t, NULL, NULL);
        } else if (((*tree).left)->t.number == 1 || ((*tree).right)->t.number == 1){
          t.number = giveCorrectValue(((*tree).left)->t.number, ((*tree).right)->t.number, 1);
          return newExpTreeNode(Number, t, NULL, NULL);
        }
        break;
      
      case '/':
        if (((*tree).right)->t.number == 1){
          t.number = ((*tree).left)->t.number;
          return newExpTreeNode(Number, t, NULL, NULL);
        }
        break;
      
      case '+':
        if (((*tree).left)->t.number == 0 || ((*tree).right)->t.number == 0){
          t.number = giveCorrectValue(((*tree).left)->t.number, ((*tree).right)->t.number, 0);
          return newExpTreeNode(Number, t, NULL, NULL);
        }
        break;
      
      case '-':
        if (((*tree).right)->t.number == 0){
          t.number = ((*tree).left)->t.number;
          return newExpTreeNode(Number, t, NULL, NULL);
        }
        break;
      
      default:
        abort();
    }
  }
  return newExpTreeNode((*tree).tt, (*tree).t, newLeft, newRight);
}




/* the function prefExpressionExpTrees performs a dialogue with the user and tries
 * to recognize the input as a prefix expression. When it is a numerical prefix 
 * expression, its value is computed and printed.
 */ 

void prefExpTrees() {
  char *ar;
  List tl, tl1;  
  ExpTree t = NULL; 
  ExpTree x = NULL;
  printf("give an expression: ");
  ar = readInput();
  while (ar[0] != '!') {
    tl = tokenList(ar); 
    printList(tl);
    tl1 = tl;
    int valid = expressionNode(&tl1, &t, 0);
    if ( valid > 0 && tl1 == NULL ) { 
         /* there should be no tokens left */
      if (valid == 2){
        printf("\nNot numerical!\n");
        t = NULL;
      }
      printf("in infix notation: ");
      printExpTreeInfix(t);
      printf("\n-------------Check1-------------\n");
      x = simplify(t);
      printf("\n-------------Check2-------------\n");
      printf("Simplified Tree : ");
      printExpTreeInfix(x);
      printf("\n\n");
      if ( isNumerical(t) ) {
        printf("the value is %g\n",valueExpTree(t));
      } else {
        printf("this is not a numerical expression\n");
      }
    } else {
      printf("this is not an expression\n"); 
    }
    freeExpTree(t);
    t = NULL; 
    freeTokenList(tl);  
    free(ar);
    printf("\ngive an expression: ");
    ar = readInput();
  }
  free(ar);
  printf("good bye\n");
}
