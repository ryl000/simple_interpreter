/*
 * Copyright 2019 Ray Li
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <string>


enum eval_id_type {
  EVAL_ID_TYPE_CONSTANT = 0
  ,EVAL_ID_TYPE_NAME
  
  ,EVAL_ID_TYPE_OP_NOT
  ,EVAL_ID_TYPE_OP_NEGATE
  
  ,EVAL_ID_TYPE_OP_ADD
  ,EVAL_ID_TYPE_OP_SUBTRACT

  ,EVAL_ID_TYPE_OP_DIVIDE
  ,EVAL_ID_TYPE_OP_MULTIPLY
  
  ,EVAL_ID_TYPE_OP_EQ
  ,EVAL_ID_TYPE_OP_NEQ
  ,EVAL_ID_TYPE_OP_GE
  ,EVAL_ID_TYPE_OP_GT
  ,EVAL_ID_TYPE_OP_LE
  ,EVAL_ID_TYPE_OP_LT
  
  ,EVAL_ID_TYPE_OP_AND
  ,EVAL_ID_TYPE_OP_OR
  
  ,EVAL_ID_TYPE_OP_ASSIGN

  ,EVAL_ID_TYPE_OP_COMMA

  ,EVAL_ID_TYPE_OP_LPARENS
  ,EVAL_ID_TYPE_OP_RPARENS

  ,EVAL_ID_TYPE_OP_SEMICOLON

  ,EVAL_ID_TYPE_OP_CREATE_DOUBLE
};


enum short_circuit_type {
  SHORT_CIRCUIT_NONE
  ,SHORT_CIRCUIT_FALSE
  ,SHORT_CIRCUIT_TRUE
};


struct eval_data_type {

  explicit eval_data_type( double in_value )
    :value( in_value )
    ,name()
    ,id( EVAL_ID_TYPE_CONSTANT )
    ,short_circuit( SHORT_CIRCUIT_NONE )
    ,short_circuit_offset( 0U )
  {}

  explicit eval_data_type( const std::string &in_name )
    :value( 0.0 )
    ,name( in_name )
    ,id( EVAL_ID_TYPE_NAME )
    ,short_circuit( SHORT_CIRCUIT_NONE )
    ,short_circuit_offset( 0U )
  {}

  explicit eval_data_type( eval_id_type in_id )
    :value( 0.0 )
    ,name()
    ,id( in_id )
    ,short_circuit( SHORT_CIRCUIT_NONE )
    ,short_circuit_offset( 0U )
  {}
  
  double              value;
  std::string         name;
  eval_id_type        id;
  short_circuit_type  short_circuit;
  size_t              short_circuit_offset;
};
