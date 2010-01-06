#pragma once
#ifndef PREDICT_H
#define PREDICT_H

/**
 * \def likely(expr)
 * \brief Predict expression to be true
 *
 * \param expr expression
 *
 * \return truth value of expression
 */

/**
 * \def unlikely(expr)
 * \brief Predict expression to be false
 *
 * \param expr expression
 *
 * \return truth value of expression
 */

#if defined(__GNUC__) || defined(__clang__)
#define likely(expr)   __builtin_expect(!!(expr), 1)
#define unlikely(expr) __builtin_expect(!!(expr), 0)
#else
#define likely(expr)   (expr)
#define unlikely(expr) (expr)
#endif

#endif
