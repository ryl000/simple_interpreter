/*
 * Copyright 2019-2020 Ray Li
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

#include "symbol_table_data_type.h"


enum instruction_id_type {
   INSTRUCTION_ID_TYPE_PUSHDOUBLE = 0
  ,INSTRUCTION_ID_TYPE_PUSHINT32
  ,INSTRUCTION_ID_TYPE_PUSHSIZET
  
  ,INSTRUCTION_ID_TYPE_NOT
  ,INSTRUCTION_ID_TYPE_NEGATE
  
  ,INSTRUCTION_ID_TYPE_LPARENS
  ,INSTRUCTION_ID_TYPE_RPARENS

  ,INSTRUCTION_ID_TYPE_FINALIZE

  ,INSTRUCTION_ID_TYPE_CLEAR
  ,INSTRUCTION_ID_TYPE_POP
  ,INSTRUCTION_ID_TYPE_JNEZ
  ,INSTRUCTION_ID_TYPE_JEQZ
  ,INSTRUCTION_ID_TYPE_JCEQZ
  ,INSTRUCTION_ID_TYPE_JMP
  ,INSTRUCTION_ID_TYPE_JMPA

  // TODO. we need several modes
  //  "absolute" address (for things like globals)
  //  relative to stack base frame
  //  relative to top-of-stack?
  ,INSTRUCTION_ID_TYPE_COPYTOADDR
  ,INSTRUCTION_ID_TYPE_COPYFROMADDR
  ,INSTRUCTION_ID_TYPE_COPYTOSTACKOFFSET
  ,INSTRUCTION_ID_TYPE_COPYFROMSTACKOFFSET

  ,INSTRUCTION_ID_TYPE_MOVE_END_OF_STACK
  ,INSTRUCTION_ID_TYPE_CALL
  ,INSTRUCTION_ID_TYPE_RETURN

  ,INSTRUCTION_ID_TYPE_DEBUG_PRINT_STACK

  ,INSTRUCTION_ID_TYPE_FN

  // NOTE: from this point down,
  //  it is assumed these enums match up
  //  with token_id_type
  //
  ,INSTRUCTION_ID_TYPE_FIRST_DIRECT_TOKEN_ID
  
  ,INSTRUCTION_ID_TYPE_ADD = INSTRUCTION_ID_TYPE_FIRST_DIRECT_TOKEN_ID
  ,INSTRUCTION_ID_TYPE_SUBTRACT

  ,INSTRUCTION_ID_TYPE_DIVIDE
  ,INSTRUCTION_ID_TYPE_MULTIPLY
  
  ,INSTRUCTION_ID_TYPE_EQ
  ,INSTRUCTION_ID_TYPE_NEQ
  ,INSTRUCTION_ID_TYPE_GE
  ,INSTRUCTION_ID_TYPE_GT
  ,INSTRUCTION_ID_TYPE_LE
  ,INSTRUCTION_ID_TYPE_LT
  
  ,INSTRUCTION_ID_TYPE_AND
  ,INSTRUCTION_ID_TYPE_OR
  
  ,INSTRUCTION_ID_TYPE_COMMA

  ,INSTRUCTION_ID_TYPE_ASSIGN

};


struct instruction_type {

  explicit instruction_type( double in_value )
    :id( INSTRUCTION_ID_TYPE_PUSHDOUBLE )
    ,linked_idx( 0U )
    ,symbol_data( nullptr )
  {
    arg.d = in_value;
  }

  explicit instruction_type( int in_ivalue )
    :id( INSTRUCTION_ID_TYPE_PUSHINT32 )
    ,linked_idx( 0U )
    ,symbol_data( nullptr )
  {
    arg.i32 = in_ivalue;
  }

  explicit instruction_type( instruction_id_type in_id )
    :id( in_id )
    ,linked_idx( 0U )
    ,symbol_data( nullptr )
  {}
  
  instruction_id_type  id;
  size_t               linked_idx;

  union {
    double  d;
    int32_t i32;
    size_t  sz;
  } arg;

  const symbol_table_data_type *symbol_data;
};
