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
	      const std::vector<eval_data_type> &expression
	      ,std::vector<char>                 &data
	      )
{
  std::vector<operand_data_type> evaluation_stack;

  size_t  iter_increment           = 1U;


  for ( std::vector<eval_data_type>::const_iterator iter = expression.begin()
	  ; iter != expression.end()
	  ; iter += iter_increment ) {

    iter_increment = 1U;
    
    switch ( iter->id ) {
    case EVAL_ID_TYPE_PUSHD:
      evaluation_stack.push_back( operand_data_type( iter->value ) );
      break;

    case EVAL_ID_TYPE_OP_PUSHADDR:
      evaluation_stack.push_back( operand_data_type( iter->addr_arg ) );
      break;

    case EVAL_ID_TYPE_OP_NOT:
      {
	if ( evaluation_stack.empty() ) {
	  return false;
	}
	  
	double value = evaluation_stack.back().value;
	  
	evaluation_stack.back().set_value( (value == 0.0) ? 1.0 : 0.0 );
      }
      break;

    case EVAL_ID_TYPE_OP_NEGATE:
      {
	if ( evaluation_stack.empty() ) {
	  return false;
	}
	  
	double value = evaluation_stack.back().value;
	  
	evaluation_stack.back().set_value( -1.0 * value );
      }
      break;

    case EVAL_ID_TYPE_OP_ADD:
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
      {
	if ( evaluation_stack.size() < 2 ) {
	  return false;
	}

	size_t dst_idx = (evaluation_stack.rbegin() + 1U)->addr;

	double new_value = (evaluation_stack.rbegin())->value;

	char *src = reinterpret_cast<char*>( &new_value );
	std::copy( src, src+8U, &(data[dst_idx]) );

	evaluation_stack.pop_back();
	evaluation_stack.back().value = new_value;
      }
      break;

    case EVAL_ID_TYPE_OP_CLEAR:
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
      {
	iter_increment = iter->jump_arg;
      }
      break;

    case EVAL_ID_TYPE_OP_COPYFROMADDR:
      {
	double new_value;
	char *src = &(data[iter->addr_arg]);
	char *dst = reinterpret_cast<char*>( &new_value );
	std::copy( src, src+8U, dst );

	evaluation_stack.emplace_back( operand_data_type( new_value ) );
      }
      break;

    case EVAL_ID_TYPE_OP_COPYTOADDR:
      {
	if ( evaluation_stack.empty() ) {
	  return false;
	}

	double value = (evaluation_stack.rbegin())->value;

	char *src = reinterpret_cast<char*>( &value );
	std::copy( src, src+8U, &(data[iter->addr_arg]) );
      }
      break;
    }
      
  }

  return true;
}
