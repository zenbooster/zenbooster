/*
 * Expression.c
 * Implementation of functions used to build the syntax tree.
 */

#include "Expression.h"

#include <stdlib.h>
#include <string.h>
#include <arduino.h>

std::map<std::string, int> dict_name_id;
std::map<int, int *> dict_id_ptr;

/**
 * @brief Allocates space for expression
 * @return The expression or NULL if not enough memory
 */
static SExpression *allocateExpression()
{
    SExpression *b = (SExpression *)malloc(sizeof(SExpression));

    if (b)
    {
        b->type = eVAL;
        b->val = 0;

        b->left = NULL;
        b->right = NULL;
    }
    return b;
}

SExpression *createNumber(float value)
{
    SExpression *b = allocateExpression();

    if (b)
    {
        b->type = eVAL;
        b->val = value;
        b->p_var = NULL;
    }
    return b;
}

void reg_var(std::string name, int *p)
{
    int id = dict_name_id.size();
    dict_name_id[name] = id;
    dict_id_ptr[id] = p;
}

SExpression *createVar(int var_id)
{
    SExpression *b = allocateExpression();

    if (b)
    {
        b->type = eVAR;
        b->p_var = dict_id_ptr[var_id];
    }
    return b;
}

SExpression *createOperation(EOperationType type, SExpression *left, SExpression *right)
{
    SExpression *b = allocateExpression();

    if (b)
    {
        b->type = type;
        b->left = left;
        b->right = right;
    }
    return b;
}

void deleteExpression(SExpression *b)
{
    if (b)
    {
        deleteExpression(b->left);
        deleteExpression(b->right);

        free(b);
    }
}