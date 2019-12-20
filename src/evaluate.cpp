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

#include <iostream>

#include "evaluate.h"


namespace {

  enum operand_type {
    OPERAND_TYPE_DOUBLE
    ,OPERAND_TYPE_INT32
    ,OPERAND_TYPE_ADDR
  };
  
  struct operand_data_type {
    explicit operand_data_type( double in_value )
      :value( in_value )
      ,addr( 0U )
      ,type( OPERAND_TYPE_DOUBLE )
    {}

    explicit operand_data_type( size_t in_addr )
      :value( 0. )
      ,addr( in_addr )
      ,type( OPERAND_TYPE_ADDR )
    {}

    void set_value( double in_value )
    {
      value = in_value;
      type  = OPERAND_TYPE_DOUBLE;
    }
    
    double       value;
    size_t       addr;
    operand_type type;
  };
  
}


bool evaluate(
	      const std::vector<eval_data_type> &instructions
	      ,std::vector<char>                 &data
	      )
{
  std::vector<operand_data_type> evaluation_stack;
  size_t                         stack_frame_base = 0U;

  // TODO. make this int16_t for 32-bit?
  //
  int32_t  iter_increment           = 1;


  for ( std::vector<eval_data_type>::const_iterator iter = instructions.begin()
	  ; iter != instructions.end()
	  ; iter += iter_increment ) {

    iter_increment = 1;
    
    switch ( iter->id ) {
    case EVAL_ID_TYPE_PUSHD:
      // PUSH-DOUBLE <dval>
      //  0, -0, +1
      evaluation_stack.push_back( operand_data_type( iter->value ) );
      break;

    case EVAL_ID_TYPE_OP_PUSHADDR:
      // PUSH-OFFSET <offset>
      //  0, -0, +1
      evaluation_stack.push_back( operand_data_type( iter->addr_arg ) );
      break;

    case EVAL_ID_TYPE_OP_NOT:
      // OP-NOT
      //  1, -1, +1 
      {
	if ( evaluation_stack.empty() ) {
	  return false;
	}
	  
	double value = evaluation_stack.back().value;
	  
	evaluation_stack.back().set_value( (value == 0.0) ? 1.0 : 0.0 );
      }
      break;

    case EVAL_ID_TYPE_OP_NEGATE:
      // OP-NEGATE
      //  1, -1, +1
      {
	if ( evaluation_stack.empty() ) {
	  return false;
	}
	  
	double value = evaluation_stack.back().value;
	  
	evaluation_stack.back().set_value( -1.0 * value );
      }
      break;

    case EVAL_ID_TYPE_OP_ADD:
      // OP-ADD
      //  2, -2, +1
      {
	if ( evaluation_stack.size() < 2 ) {
	  return false;
	}
	  
	double value1 = (evaluation_stack.rbegin() + 1U)->value;
	double value2 = (evaluation_stack.rbegin()     )->value;

	double result = value1 + value2;
	evaluation_stack.pop_back();
	evaluation_stack.back().set_value( result );
      }
      break;

    case EVAL_ID_TYPE_OP_SUBTRACT:
      // OP-SUB
      //  2, -2, +1
      {
	if ( evaluation_stack.size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.rbegin() + 1U)->value;
	double value2 = (evaluation_stack.rbegin()     )->value;

	double result = value1 - value2;
	evaluation_stack.pop_back();
	evaluation_stack.back().set_value( result );

      }
      break;

    case EVAL_ID_TYPE_OP_DIVIDE:
      // OP-DIV
      //  2, -2, +1
      {
	if ( evaluation_stack.size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.rbegin() + 1U)->value;
	double value2 = (evaluation_stack.rbegin()     )->value;

	if ( value2 == 0.0 ) {
	  return false;
	}

	double result = value1 / value2;
	evaluation_stack.pop_back();
	evaluation_stack.back().set_value( result );
      }
      break;

    case EVAL_ID_TYPE_OP_MULTIPLY:
      // OP-MULT
      //  2, -2, +1
      {
	if ( evaluation_stack.size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.rbegin() + 1U)->value;
	double value2 = (evaluation_stack.rbegin()     )->value;

	double result = value1 * value2;
	evaluation_stack.pop_back();
	evaluation_stack.back().set_value( result );
      }
      break;

    case EVAL_ID_TYPE_OP_EQ:
      // OP-EQ
      //  2, -2, +1
      {
	if ( evaluation_stack.size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.rbegin() + 1U)->value;
	double value2 = (evaluation_stack.rbegin()     )->value;

	bool result = ( value1 == value2 );
	evaluation_stack.pop_back();
	evaluation_stack.back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case EVAL_ID_TYPE_OP_NEQ:
      // OP-NEQ
      //  2, -2, +1
      {
	if ( evaluation_stack.size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.rbegin() + 1U)->value;
	double value2 = (evaluation_stack.rbegin()     )->value;

	bool result = ( value1 != value2 );
	evaluation_stack.pop_back();
	evaluation_stack.back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case EVAL_ID_TYPE_OP_GE:
      // OP-GE
      //  2, -2, +1
      {
	if ( evaluation_stack.size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.rbegin() + 1U)->value;
	double value2 = (evaluation_stack.rbegin()     )->value;

	bool result = ( value1 >= value2 );
	evaluation_stack.pop_back();
	evaluation_stack.back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case EVAL_ID_TYPE_OP_GT:
      // OP-GT
      //  2, -2, +1
      {
	if ( evaluation_stack.size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.rbegin() + 1U)->value;
	double value2 = (evaluation_stack.rbegin()     )->value;

	bool result = ( value1 > value2 );
	evaluation_stack.pop_back();
	evaluation_stack.back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case EVAL_ID_TYPE_OP_LE:
      // OP-LE
      //  2, -2, +1
      {
	if ( evaluation_stack.size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.rbegin() + 1U)->value;
	double value2 = (evaluation_stack.rbegin()     )->value;

	bool result = ( value1 <= value2 );
	evaluation_stack.pop_back();
	evaluation_stack.back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case EVAL_ID_TYPE_OP_LT:
      // OP-LT
      //  2, -2, +1
      {
	if ( evaluation_stack.size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.rbegin() + 1U)->value;
	double value2 = (evaluation_stack.rbegin()     )->value;

	bool result = ( value1 < value2 );
	evaluation_stack.pop_back();
	evaluation_stack.back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case EVAL_ID_TYPE_OP_AND:
      // OP-AND
      //  2, -2, +1
      {
	if ( evaluation_stack.size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.rbegin() + 1U)->value;
	bool result = ( value1 != 0.0 );

	if ( result ) {
	  double value2 = (evaluation_stack.rbegin()     )->value;
	  result &= (value2 != 0.0);
	}

	evaluation_stack.pop_back();
	evaluation_stack.back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case EVAL_ID_TYPE_OP_OR:
      // OP-OR
      //  2, -2, +1
      {
	if ( evaluation_stack.size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.rbegin() + 1U)->value;
	bool result = ( value1 != 0.0 );

	if ( !result ) {
	  double value2 = (evaluation_stack.rbegin()     )->value;
	  result |= (value2 != 0.0);
	}

	evaluation_stack.pop_back();
	evaluation_stack.back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case EVAL_ID_TYPE_OP_ASSIGN:
      // OP-ASSIGN
      //  2, -2, +1
      {
	if ( evaluation_stack.size() < 2 ) {
	  return false;
	}

	size_t dst_idx = (evaluation_stack.rbegin() + 1U)->addr;

	double new_value = (evaluation_stack.rbegin())->value; // TODO. varible type

	char *src = reinterpret_cast<char*>( &new_value );
	std::copy( src, src+8U, &(data[dst_idx + stack_frame_base]) );

	evaluation_stack.pop_back();
	evaluation_stack.back().value = new_value;
      }
      break;

    case EVAL_ID_TYPE_OP_CLEAR:
      // OP-CLEAR
      //  clears estack
      {
	if ( !evaluation_stack.empty() ) {
	  double value = (evaluation_stack.rbegin())->value;
	  std::cout << " => " << value << "\n";
	  if ( evaluation_stack.size() > 1 ) {
	    std::cout << "WARNING: final stack size is " << evaluation_stack.size() << "\n";
	  }
	}
	evaluation_stack.clear();
      }
      break;
      
    case EVAL_ID_TYPE_OP_POP:
      // OP-POP <narg>
      //  narg, -narg, +0
      {
	if ( evaluation_stack.size() < iter->pop_arg ) {
	  return false;
	}

	for ( size_t i=0; i<iter->pop_arg; ++i ) {
	  evaluation_stack.pop_back();
	}
      }
      break;

    case EVAL_ID_TYPE_OP_JNEZ:
      // OP-JNEZ <offset>
      //  1, -0, +0
      {
	if ( evaluation_stack.empty() ) {
	  return false;
	}

	double value = (evaluation_stack.rbegin())->value;

	if ( value != 0.0 ) {
	  iter_increment = iter->jump_arg;
	}
      }
      break;

    case EVAL_ID_TYPE_OP_JEQZ:
      // OP-JEQZ <offset>
      //  1, -0, +0
      {
	if ( evaluation_stack.empty() ) {
	  return false;
	}

	double value = (evaluation_stack.rbegin())->value;
	if ( value == 0.0 ) {
	  iter_increment = iter->jump_arg;
	}
      }
      break;

    case EVAL_ID_TYPE_OP_JCEQZ:
      // OP-JCEQZ <offset>
      //  1, -1, +0
      {
	if ( evaluation_stack.empty() ) {
	  return false;
	}

	double value = (evaluation_stack.rbegin())->value;

	if ( value == 0.0 ) {
	  iter_increment = iter->jump_arg;
	}
	evaluation_stack.pop_back();
      }
      break;

    case EVAL_ID_TYPE_OP_JMP:
      // OP-JMP <offset>
      //  0, -0, +0
      {
	iter_increment = iter->jump_arg;
      }
      break;

    case EVAL_ID_TYPE_OP_COPYFROMADDR:
      // OP-COPY-FROM-OFFSET <offset>
      //  0, -0, +1
      {
	double new_value;
	char *src = &(data[iter->addr_arg + stack_frame_base]);
	char *dst = reinterpret_cast<char*>( &new_value );
	std::copy( src, src+8U, dst ); // TODO. variable-size copy

	evaluation_stack.emplace_back( operand_data_type( new_value ) );
      }
      break;

    case EVAL_ID_TYPE_OP_COPYTOADDR:
      // OP-COPY-TO-OFFSET <offset>
      //  1, -0, +0
      {
	if ( evaluation_stack.empty() ) {
	  return false;
	}

	double value = (evaluation_stack.rbegin())->value;

	char *src = reinterpret_cast<char*>( &value );
	std::copy( src, src+8U, &(data[iter->addr_arg + stack_frame_base]) ); // TODO. variable-size copy
      }
      break;

    case EVAL_ID_TYPE_OP_MOVE_END_OF_STACK:
      // OP-MOVE-END-OF-STACK
      //  0, -0, +0
      {
	size_t new_size = data.size() + iter->jump_arg;
	data.resize( new_size );
      }
      break;

    case EVAL_ID_TYPE_OP_LPARENS:
    case EVAL_ID_TYPE_OP_RPARENS:
    case EVAL_ID_TYPE_OP_FINALIZE:
    case EVAL_ID_TYPE_OP_COMMA:
      // NOTE. These should never occur...
      break;
    }
  }

  return true;
}
