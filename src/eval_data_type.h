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


enum eval_id_type {
  EVAL_ID_TYPE_PUSHD = 0
  
  ,EVAL_ID_TYPE_OP_NOT
  ,EVAL_ID_TYPE_OP_NEGATE
  
  ,EVAL_ID_TYPE_OP_LPARENS
  ,EVAL_ID_TYPE_OP_RPARENS

  ,EVAL_ID_TYPE_OP_FINALIZE

  ,EVAL_ID_TYPE_OP_CLEAR
  ,EVAL_ID_TYPE_OP_POP
  ,EVAL_ID_TYPE_OP_JNEZ
  ,EVAL_ID_TYPE_OP_JEQZ
  ,EVAL_ID_TYPE_OP_JCEQZ
  ,EVAL_ID_TYPE_OP_JMP
  ,EVAL_ID_TYPE_OP_JMPB

  ,EVAL_ID_TYPE_OP_PUSHADDR
  ,EVAL_ID_TYPE_OP_COPYTOADDR
  ,EVAL_ID_TYPE_OP_COPYFROMADDR

  // NOTE: from this point down,
  //  it is assumed these enums match up
  //  with token_id_type
  //
  ,EVAL_ID_FIRST_DIRECT_TOKEN_TO_CMD
  
  ,EVAL_ID_TYPE_OP_ADD = EVAL_ID_FIRST_DIRECT_TOKEN_TO_CMD
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
  
  ,EVAL_ID_TYPE_OP_COMMA

  ,EVAL_ID_TYPE_OP_ASSIGN

};


struct eval_data_type {

  explicit eval_data_type( double in_value )
    :value( in_value )
    ,id( EVAL_ID_TYPE_PUSHD )
    ,addr_arg( 0U )
    ,jump_arg( 0U )
    ,pop_arg( 0U )
  {}

  explicit eval_data_type( eval_id_type in_id )
    :value( 0.0 )
    ,id( in_id )
    ,addr_arg( 0U )
    ,jump_arg( 0U )
    ,pop_arg( 0U )
  {}
  
  double              value;
  eval_id_type        id;
  size_t              addr_arg;
  size_t              jump_arg;
  size_t              pop_arg;
};
