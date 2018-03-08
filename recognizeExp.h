/* recognizeExp.h, Gerard Renardel, 29 January 2014 */

#ifndef RECOGNIZEEXP_H
#define RECOGNIZEEXP_H

int acceptNumber(List *lp);
int acceptIdentifier(List *lp);
int acceptCharacter(List *lp, char c);
int acceptTerm(List *lp);
int acceptEquation(List *lp, List *lp1);
int acceptExpression(List *lp);
int countEquals(List *lp);
int acceptVariables(List *lp);
int checkVariables(List *lp);
int isDegree(List *lp);
int checkDegree(List *lp);

void recognizeEquation();
void recognizeExpressions();

#endif
