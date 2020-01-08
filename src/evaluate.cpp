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
      ,ivalue( 0 )
      ,addr( 0U )
      ,type( OPERAND_TYPE_DOUBLE )
    {}

    explicit operand_data_type( int32_t in_ivalue )
      :value( 0. )
      ,ivalue( in_ivalue )
      ,addr( 0U )
      ,type( OPERAND_TYPE_INT32 )
    {}

    explicit operand_data_type( size_t in_addr )
      :value( 0. )
      ,ivalue( 0 )
      ,addr( in_addr )
      ,type( OPERAND_TYPE_ADDR )
    {}

    void set_value( double in_value )
    {
      value = in_value;
      type  = OPERAND_TYPE_DOUBLE;
    }
    
    double       value;
    int          ivalue;
    size_t       addr;
    operand_type type;
  };
  
}


bool evaluate(
	      const std::vector<eval_data_type> &instructions
	      ,std::vector<char>                 &data
	      )
{
  std::vector<std::vector<operand_data_type>> evaluation_stack( 1U );
  size_t                                      stack_frame_base = 0U;

  size_t instr_index = 0U;
  while ( instr_index < instructions.size() ) {

    bool    jump_absolute  = false;
    // TODO. make this int16_t for 32-bit?
    int32_t iter_increment = 1;

    std::vector<eval_data_type>::const_iterator iter = instructions.begin() + instr_index;

    iter_increment = 1;
    
    switch ( iter->id ) {
    case EVAL_ID_TYPE_PUSHD:
      // PUSH-DOUBLE <dval>
      //  0, -0, +1
      evaluation_stack.back().push_back( operand_data_type( iter->arg.d ) );
      break;

    // TODO. rename to PUSH-INT32
    case EVAL_ID_TYPE_PUSHI:
      // PUSH-INT <ival>
      //  0, -0, +1
      evaluation_stack.back().push_back( operand_data_type( iter->arg.i32 ) );
      break;

    // TODO. rename to PUSH-SIZET
    case EVAL_ID_TYPE_OP_PUSHADDR:
      // PUSH-ADDR <addr>
      //  0, -0, +1
      evaluation_stack.back().push_back( operand_data_type( iter->arg.sz ) );
      break;

    // TODO. replace with PUSH-INT32
    case EVAL_ID_TYPE_OP_PUSHSTACKOFFSET:
      // PUSH-OFFSET <offset>
      //  0, -0, +1
      evaluation_stack.back().push_back( operand_data_type( iter->arg.i32 ) );
      break;

    case EVAL_ID_TYPE_OP_NOT:
      // OP-NOT
      //  1, -1, +1 
      {
	if ( evaluation_stack.back().empty() ) {
	  return false;
	}
	  
	double value = evaluation_stack.back().back().value;
	  
	evaluation_stack.back().back().set_value( (value == 0.0) ? 1.0 : 0.0 );
      }
      break;

    case EVAL_ID_TYPE_OP_NEGATE:
      // OP-NEGATE
      //  1, -1, +1
      {
	if ( evaluation_stack.back().empty() ) {
	  return false;
	}
	  
	double value = evaluation_stack.back().back().value;
	  
	evaluation_stack.back().back().set_value( -1.0 * value );
      }
      break;

    case EVAL_ID_TYPE_OP_ADD:
      // OP-ADD
      //  2, -2, +1
      {
	if ( evaluation_stack.back().size() < 2 ) {
	  return false;
	}
	  
	double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
	double value2 = (evaluation_stack.back().rbegin()     )->value;

	double result = value1 + value2;
	evaluation_stack.back().pop_back();
	evaluation_stack.back().back().set_value( result );
      }
      break;

    case EVAL_ID_TYPE_OP_SUBTRACT:
      // OP-SUB
      //  2, -2, +1
      {
	if ( evaluation_stack.back().size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
	double value2 = (evaluation_stack.back().rbegin()     )->value;

	double result = value1 - value2;
	evaluation_stack.back().pop_back();
	evaluation_stack.back().back().set_value( result );

      }
      break;

    case EVAL_ID_TYPE_OP_DIVIDE:
      // OP-DIV
      //  2, -2, +1
      {
	if ( evaluation_stack.back().size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
	double value2 = (evaluation_stack.back().rbegin()     )->value;

	if ( value2 == 0.0 ) {
	  return false;
	}

	double result = value1 / value2;
	evaluation_stack.back().pop_back();
	evaluation_stack.back().back().set_value( result );
      }
      break;

    case EVAL_ID_TYPE_OP_MULTIPLY:
      // OP-MULT
      //  2, -2, +1
      {
	if ( evaluation_stack.back().size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
	double value2 = (evaluation_stack.back().rbegin()     )->value;

	double result = value1 * value2;
	evaluation_stack.back().pop_back();
	evaluation_stack.back().back().set_value( result );
      }
      break;

    case EVAL_ID_TYPE_OP_EQ:
      // OP-EQ
      //  2, -2, +1
      {
	if ( evaluation_stack.back().size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
	double value2 = (evaluation_stack.back().rbegin()     )->value;

	bool result = ( value1 == value2 );
	evaluation_stack.back().pop_back();
	evaluation_stack.back().back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case EVAL_ID_TYPE_OP_NEQ:
      // OP-NEQ
      //  2, -2, +1
      {
	if ( evaluation_stack.back().size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
	double value2 = (evaluation_stack.back().rbegin()     )->value;

	bool result = ( value1 != value2 );
	evaluation_stack.back().pop_back();
	evaluation_stack.back().back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case EVAL_ID_TYPE_OP_GE:
      // OP-GE
      //  2, -2, +1
      {
	if ( evaluation_stack.back().size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
	double value2 = (evaluation_stack.back().rbegin()     )->value;

	bool result = ( value1 >= value2 );
	evaluation_stack.back().pop_back();
	evaluation_stack.back().back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case EVAL_ID_TYPE_OP_GT:
      // OP-GT
      //  2, -2, +1
      {
	if ( evaluation_stack.back().size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
	double value2 = (evaluation_stack.back().rbegin()     )->value;

	bool result = ( value1 > value2 );
	evaluation_stack.back().pop_back();
	evaluation_stack.back().back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case EVAL_ID_TYPE_OP_LE:
      // OP-LE
      //  2, -2, +1
      {
	if ( evaluation_stack.back().size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
	double value2 = (evaluation_stack.back().rbegin()     )->value;

	bool result = ( value1 <= value2 );
	evaluation_stack.back().pop_back();
	evaluation_stack.back().back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case EVAL_ID_TYPE_OP_LT:
      // OP-LT
      //  2, -2, +1
      {
	if ( evaluation_stack.back().size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
	double value2 = (evaluation_stack.back().rbegin()     )->value;

	bool result = ( value1 < value2 );
	evaluation_stack.back().pop_back();
	evaluation_stack.back().back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case EVAL_ID_TYPE_OP_AND:
      // OP-AND
      //  2, -2, +1
      {
	if ( evaluation_stack.back().size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
	bool result = ( value1 != 0.0 );

	if ( result ) {
	  double value2 = (evaluation_stack.back().rbegin()     )->value;
	  result &= (value2 != 0.0);
	}

	evaluation_stack.back().pop_back();
	evaluation_stack.back().back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case EVAL_ID_TYPE_OP_OR:
      // OP-OR
      //  2, -2, +1
      {
	if ( evaluation_stack.back().size() < 2 ) {
	  return false;
	}

	double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
	bool result = ( value1 != 0.0 );

	if ( !result ) {
	  double value2 = (evaluation_stack.back().rbegin()     )->value;
	  result |= (value2 != 0.0);
	}

	evaluation_stack.back().pop_back();
	evaluation_stack.back().back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case EVAL_ID_TYPE_OP_ASSIGN:
      // OP-ASSIGN
      //  3, -3, +1
      {
	if ( evaluation_stack.back().size() < 3 ) {
	  return false;
	}


	double new_value = (evaluation_stack.back().rbegin())->value; // TODO. varible type
	char *src = reinterpret_cast<char*>( &new_value );

	int is_abs = (evaluation_stack.back().rbegin() + 1U)->ivalue;

	if ( is_abs ) {
	  size_t dst_idx = (evaluation_stack.back().rbegin() + 2U)->addr;
	  std::copy( src, src+8U, &(data[dst_idx]) );
	}
	else {
	  int32_t dst_offset = (evaluation_stack.back().rbegin() + 2U)->ivalue;
	  std::copy( src, src+8U, &(data[dst_offset + stack_frame_base]) );
	}

	evaluation_stack.back().pop_back();
	evaluation_stack.back().pop_back();
	evaluation_stack.back().back().value = new_value;
      }
      break;

    case EVAL_ID_TYPE_OP_CLEAR:
      // OP-CLEAR
      //  clears estack
      {
	if ( !evaluation_stack.back().empty() ) {
	  double value = (evaluation_stack.back().rbegin())->value;
	  std::cout << " => " << value << "\n";
	  if ( evaluation_stack.back().size() > 1 ) {
	    std::cout << "WARNING: final stack size is " << evaluation_stack.back().size() << "\n";
	  }
	}
	evaluation_stack.back().clear();
      }
      break;
      
    case EVAL_ID_TYPE_OP_POP:
      // OP-POP <narg>
      //  narg, -narg, +0
      {
	std::cout << "debug: pop\n";
	if ( evaluation_stack.back().size() < iter->arg.sz ) {
	  return false;
	}

	for ( size_t i=0; i<iter->arg.sz; ++i ) {
	  evaluation_stack.back().pop_back();
	}
      }
      break;

    case EVAL_ID_TYPE_OP_JNEZ:
      // OP-JNEZ <offset>
      //  1, -0, +0
      {
	if ( evaluation_stack.back().empty() ) {
	  return false;
	}

	double value = (evaluation_stack.back().rbegin())->value;

	if ( value != 0.0 ) {
	  iter_increment = iter->arg.i32;
	}
      }
      break;

    case EVAL_ID_TYPE_OP_JEQZ:
      // OP-JEQZ <offset>
      //  1, -0, +0
      {
	if ( evaluation_stack.back().empty() ) {
	  return false;
	}

	double value = (evaluation_stack.back().rbegin())->value;
	if ( value == 0.0 ) {
	  iter_increment = iter->arg.i32;
	}
      }
      break;

    case EVAL_ID_TYPE_OP_JCEQZ:
      // OP-JCEQZ <offset>
      //  1, -1, +0
      {
	if ( evaluation_stack.back().empty() ) {
	  return false;
	}

	double value = (evaluation_stack.back().rbegin())->value;

	if ( value == 0.0 ) {
	  iter_increment = iter->arg.i32;
	}
	evaluation_stack.back().pop_back();
      }
      break;

    case EVAL_ID_TYPE_OP_JMP:
      // OP-JMP <offset>
      //  0, -0, +0
      {
	iter_increment = iter->arg.i32;
      }
      break;

    case EVAL_ID_TYPE_OP_JMPA:
      // OP-JMPA <addr>
      //  0, -0, +0
      {
	instr_index   = iter->arg.i32; // TODO. type mismatch!
	jump_absolute = true;
      }
      break;

    case EVAL_ID_TYPE_OP_COPYFROMADDR:
      // OP-COPY-FROM-ADDR <addr>
      //  0, -0, +1
      {
	double new_value;
	char *src = &(data[iter->arg.sz]);
	char *dst = reinterpret_cast<char*>( &new_value );
	std::copy( src, src+8U, dst ); // TODO. variable-size copy

	evaluation_stack.back().emplace_back( operand_data_type( new_value ) );
      }
      break;

    case EVAL_ID_TYPE_OP_COPYFROMSTACKOFFSET:
      // OP-COPY-FROM-OFFSET <offset>
      //  0, -0, +1
      {
	double new_value;
	char *src = &(data[iter->arg.i32 + stack_frame_base]);
	char *dst = reinterpret_cast<char*>( &new_value );
	std::copy( src, src+8U, dst ); // TODO. variable-size copy

	evaluation_stack.back().emplace_back( operand_data_type( new_value ) );
      }
      break;

    case EVAL_ID_TYPE_OP_COPYTOADDR:
      // OP-COPY-TO-ADDR <addr>
      //  1, -0, +0
      {
	if ( evaluation_stack.back().empty() ) {
	  return false;
	}

	double value = (evaluation_stack.back().rbegin())->value;

	char *src = reinterpret_cast<char*>( &value );
	std::copy( src, src+8U, &(data[iter->arg.sz]) ); // TODO. variable-size copy
      }
      break;

    case EVAL_ID_TYPE_OP_COPYTOSTACKOFFSET:
      // OP-COPY-TO-STACK-OFFSET <offset>
      //  1, -0, +0
      {
	if ( evaluation_stack.back().empty() ) {
	  return false;
	}

	double value = (evaluation_stack.back().rbegin())->value;

	char *src = reinterpret_cast<char*>( &value );
	std::copy( src, src+8U, &(data[iter->arg.i32 + stack_frame_base]) ); // TODO. variable-size copy
      }
      break;

    case EVAL_ID_TYPE_OP_MOVE_END_OF_STACK:
      // OP-MOVE-END-OF-STACK
      //  0, -0, +0
      {
	size_t new_size = data.size() + iter->arg.i32;
	data.resize( new_size );
      }
      break;

    case EVAL_ID_TYPE_OP_CALL:
      {
	std::cout << "=====CALL=====\n";
	std::cout << "current stack frame base is " << stack_frame_base << "\n";
	std::cout << "data stack size is " << data.size() << "\n";

	// push address of next instruction onto stack
	data.resize( data.size() + 8U );
	*(reinterpret_cast<size_t*>( &(data[data.size()-8U]) )) = instr_index + 1U;
	std::cout << "pushing return addr : " << *(reinterpret_cast<size_t*>( &(data[data.size()-8U]) )) << "\n";
	
	// push current stack frame base onto stack
	data.resize( data.size() + 8U );
	*(reinterpret_cast<size_t*>( &(data[data.size()-8U]) )) = stack_frame_base;
	std::cout << "pushing current stack frame base : " << *(reinterpret_cast<size_t*>( &(data[data.size()-8U]) )) << "\n";

	// reset stack frame base to end-of-stack, in preparation for
	// function execution
	//
	stack_frame_base = data.size();
	std::cout << "new stack frame base will be " << stack_frame_base << "\n";

	// New evaluation stack for the function
	//
	evaluation_stack.push_back( std::vector<operand_data_type>() );

	// jump to function start
	//
	instr_index   = iter->arg.sz;
	std::cout << "jumping to " << instr_index << "\n";
	jump_absolute = true;
      }
      break;
      
    case EVAL_ID_TYPE_OP_RETURN:
      {
	std::cout << "=====RETURN=====\n";
	
	size_t old_stack_frame_base = *(reinterpret_cast<size_t*>(&(data[stack_frame_base -  8])));
	std::cout << "debug: setting stack frame base to " << old_stack_frame_base << "\n";
	size_t return_address       = *(reinterpret_cast<size_t*>(&(data[stack_frame_base - 16])));
	std::cout << "debug: jump back addr is " << return_address << "\n";
	data.resize( stack_frame_base - 16 );
	std::cout << "data stack size is now " << data.size() << "\n";

	// Remove the function's evaluation stack
	//
	evaluation_stack.pop_back();

	// Restore state before returning to caller
	//
	stack_frame_base = old_stack_frame_base;
	instr_index      = return_address;
	jump_absolute    = true;
      }
      break;

    case EVAL_ID_TYPE_OP_DEBUG_PRINT_STACK:
      // TODO.
      std::cout << "DEBUG: stack size is " << data.size() << "\n";
      {
	for ( size_t i=0U; i<data.size(); i += 8 ) {
	  std::cout << i << ": " << *(reinterpret_cast<double*>( &(data[i]) )) << ","
		    << *(reinterpret_cast<size_t*>( &(data[i]) )) << "\n";
	}
      }
      break;

    case EVAL_ID_TYPE_OP_COMMA:
    case EVAL_ID_TYPE_OP_FINALIZE:
    case EVAL_ID_TYPE_OP_FN:
    case EVAL_ID_TYPE_OP_LPARENS:
    case EVAL_ID_TYPE_OP_RPARENS:
      // NOTE. These should never occur...
      break;
    }

    if ( !jump_absolute ) {
      instr_index += iter_increment;
    }
  }

  return true;
}
