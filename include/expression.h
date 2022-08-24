/*
 * Expression.h
 * Definition of the structure used to build the syntax tree.
 */
#ifndef __EXPRESSION_H__
#define __EXPRESSION_H__

#include <map>
#include <string>

//extern std::map<std::string, int *> dict;
extern std::map<std::string, int> dict_name_id;
extern std::map<int, int *> dict_id_ptr;

/**
 * @brief The operation type
 */
typedef enum tagEOperationType
{
    eVAL,
    eVAR,
    eDIV,
    eMUL,
    eSUB,
    eADD
} EOperationType;

/**
 * @brief The expression structure
 */
typedef struct tagSExpression
{
    EOperationType type; /* /< type of operation */
    float val; /* /< valid only when type is eVALUE */
    int *p_var;
    struct tagSExpression *left; /* /<  left side of the tree */
    struct tagSExpression *right; /* /< right side of the tree */
} SExpression;

/**
 * @brief It creates an identifier
 * @param value The number value
 * @return The expression or NULL in case of no memory
 */
SExpression *createNumber(float value);

void reg_var(std::string name, int *p);

/**
 * @brief It creates an identifier
 * @param value Variable name
 * @return The expression or NULL in case of no memory
 */
SExpression *createVar(int var_id);

/**
 * @brief It creates an operation
 * @param type The operation type
 * @param left The left operand
 * @param right The right operand
 * @return The expression or NULL in case of no memory
 */
SExpression *createOperation(EOperationType type, SExpression *left, SExpression *right);

/**
 * @brief Deletes a expression
 * @param b The expression
 */
void deleteExpression(SExpression *b);

#endif /* __EXPRESSION_H__ */