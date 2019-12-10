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
  enum data_type {
    DATA_TYPE_VALUE
    ,DATA_TYPE_NAME
  };

  
  struct operand_type {
    explicit operand_type( double in_value )
      :name()
      ,value( in_value )
      ,type( DATA_TYPE_VALUE )
    {}

    explicit operand_type( const std::string &in_name )
      :name( in_name )
      ,value( 0.0 )
      ,type( DATA_TYPE_NAME )
    {}

    bool get_value( const std::map<std::string,double> &variables, double *out_value )
    {
      if ( type == DATA_TYPE_VALUE ) {
	*out_value = value;
	return true;
      }
      else {
	std::map<std::string,double>::const_iterator iter = variables.find( name );
	if ( iter == variables.end() ) {
	  return false;
	}
	*out_value = iter->second;
	return true;
      }
    }

    void set_value( double in_value )
    {
      value = in_value;
      type = DATA_TYPE_VALUE;
    }
    
    std::string name;
    double      value;
    data_type   type;
  };
}


bool evaluate(
	      const std::vector<eval_data_type> &expression
	      ,std::map<std::string,double>      &variables
	      )
{
  std::vector<operand_type> evaluation_stack;

  size_t  iter_increment           = 1U;


  for ( std::vector<eval_data_type>::const_iterator iter = expression.begin()
	  ; iter != expression.end()
	  ; iter += iter_increment ) {

    iter_increment = 1U;
    
    switch ( iter->id ) {
    case EVAL_ID_TYPE_PUSHD:
      evaluation_stack.push_back( operand_type( iter->value ) );
      break;

    case EVAL_ID_TYPE_PUSHN:
      evaluation_stack.push_back( operand_type( iter->name ) );
      break;

    case EVAL_ID_TYPE_OP_NOT:
      {
	if ( evaluation_stack.empty() ) {
	  return false;
	}
	  
	double value;
	bool value_found = evaluation_stack.back().get_value( variables, &value );
	if ( !value_found ) {
	  return false;
	}
	  
	evaluation_stack.back().set_value( (value == 0.0) ? 1.0 : 0.0 );
      }
      break;

    case EVAL_ID_TYPE_OP_NEGATE:
      {
	if ( evaluation_stack.empty() ) {
	  return false;
	}
	  
	double value;
	bool value_found = evaluation_stack.back().get_value( variables, &value );
	if ( !value_found ) {
	  return false;
	}
	  
	evaluation_stack.back().set_value( -1.0 * value );
      }
      break;

    case EVAL_ID_TYPE_OP_ADD:
      {
	if ( evaluation_stack.size() < 2 ) {
	  return false;
	}
	  
	double value1;
	bool value1_found = (evaluation_stack.rbegin() + 1)->get_value( variables, &value1 );
	if ( !value1_found ) {
	  return false;
	}
	double value2;
	bool value2_found = (evaluation_stack.rbegin())->get_value( variables, &value2 );
	if ( !value2_found ) {
	  return false;
	}

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

	double value1;
	bool value1_found = (evaluation_stack.rbegin() + 1)->get_value( variables, &value1 );
	if ( !value1_found ) {
	  return false;
	}
	double value2;
	bool value2_found = (evaluation_stack.rbegin())->get_value( variables, &value2 );
	if ( !value2_found ) {
	  return false;
	}

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

	double value1;
	bool value1_found = (evaluation_stack.rbegin() + 1)->get_value( variables, &value1 );
	if ( !value1_found ) {
	  return false;
	}
	double value2;
	bool value2_found = (evaluation_stack.rbegin())->get_value( variables, &value2 );
	if ( !value2_found ) {
	  return false;
	}

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

	double value1;
	bool value1_found = (evaluation_stack.rbegin() + 1)->get_value( variables, &value1 );
	if ( !value1_found ) {
	  return false;
	}
	double value2;
	bool value2_found = (evaluation_stack.rbegin())->get_value( variables, &value2 );
	if ( !value2_found ) {
	  return false;
	}

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

	double value1;
	bool value1_found = (evaluation_stack.rbegin() + 1)->get_value( variables, &value1 );
	if ( !value1_found ) {
	  return false;
	}
	double value2;
	bool value2_found = (evaluation_stack.rbegin())->get_value( variables, &value2 );
	if ( !value2_found ) {
	  return false;
	}

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

	double value1;
	bool value1_found = (evaluation_stack.rbegin() + 1)->get_value( variables, &value1 );
	if ( !value1_found ) {
	  return false;
	}
	double value2;
	bool value2_found = (evaluation_stack.rbegin())->get_value( variables, &value2 );
	if ( !value2_found ) {
	  return false;
	}

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

	double value1;
	bool value1_found = (evaluation_stack.rbegin() + 1)->get_value( variables, &value1 );
	if ( !value1_found ) {
	  return false;
	}
	double value2;
	bool value2_found = (evaluation_stack.rbegin())->get_value( variables, &value2 );
	if ( !value2_found ) {
	  return false;
	}

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

	double value1;
	bool value1_found = (evaluation_stack.rbegin() + 1)->get_value( variables, &value1 );
	if ( !value1_found ) {
	  return false;
	}
	double value2;
	bool value2_found = (evaluation_stack.rbegin())->get_value( variables, &value2 );
	if ( !value2_found ) {
	  return false;
	}

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

	double value1;
	bool value1_found = (evaluation_stack.rbegin() + 1)->get_value( variables, &value1 );
	if ( !value1_found ) {
	  return false;
	}
	double value2;
	bool value2_found = (evaluation_stack.rbegin())->get_value( variables, &value2 );
	if ( !value2_found ) {
	  return false;
	}

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

	double value1;
	bool value1_found = (evaluation_stack.rbegin() + 1)->get_value( variables, &value1 );
	if ( !value1_found ) {
	  return false;
	}
	double value2;
	bool value2_found = (evaluation_stack.rbegin())->get_value( variables, &value2 );
	if ( !value2_found ) {
	  return false;
	}

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

	double value1;
	bool value1_found = (evaluation_stack.rbegin() + 1)->get_value( variables, &value1 );
	if ( !value1_found ) {
	  return false;
	}

	bool result = ( value1 != 0.0 );
	if ( result ) {
	  double value2;
	  bool value2_found = (evaluation_stack.rbegin())->get_value( variables, &value2 );
	  if ( !value2_found ) {
	    return false;
	  }

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

	double value1;
	bool value1_found = (evaluation_stack.rbegin() + 1)->get_value( variables, &value1 );
	if ( !value1_found ) {
	  return false;
	}

	bool result = ( value1 != 0.0 );
	if ( !result ) {
	  double value2;
	  bool value2_found = (evaluation_stack.rbegin())->get_value( variables, &value2 );
	  if ( !value2_found ) {
	    return false;
	  }

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
	if ( (evaluation_stack.rbegin() + 1)->type != DATA_TYPE_NAME ) {
	  return false;
	}
	std::map<std::string,double>::iterator iter = variables.find( (evaluation_stack.rbegin() + 1)->name );
	if ( iter == variables.end() ) {
	  return false;
	}

	double new_value;
	bool value_found = evaluation_stack.rbegin()->get_value( variables, &new_value );
	if ( !value_found ) {
	  return false;
	}
	evaluation_stack.pop_back();
	iter->second = new_value;
      }
      break;

    case EVAL_ID_TYPE_OP_CREATE_DOUBLE:
      {
	if ( evaluation_stack.empty() ) {
	  return false;
	}
	if ( evaluation_stack.back().type != DATA_TYPE_NAME ) {
	  return false;
	}

	std::map<std::string,double>::iterator iter = variables.find( evaluation_stack.back().name );
	if ( iter != variables.end() ) {
	  return false;
	}

	variables.insert( std::make_pair( evaluation_stack.back().name, 0.0 ) );
      }
      break;

    case EVAL_ID_TYPE_OP_CLEAR:
      {
	// DEBUG
	if ( !evaluation_stack.empty() ) {
	  double value;
	  bool value_found = evaluation_stack.back().get_value( variables, &value );
	  if ( !value_found ) {
	    return false;
	  }
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

	double value;
	bool value_found = evaluation_stack.back().get_value( variables, &value );
	if ( !value_found ) {
	  return false;
	}

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

	double value;
	bool value_found = evaluation_stack.back().get_value( variables, &value );
	if ( !value_found ) {
	  return false;
	}

	if ( value == 0.0 ) {
	  iter_increment = iter->jump_arg;
	}
      }
      break;

    }
      
  }

  #if 0
  if ( !evaluation_stack.empty() ) {
    double value;
    bool value_found = evaluation_stack.back().get_value( variables, &value );
    if ( !value_found ) {
      return false;
    }
    std::cout << " => " << value << "\n";
    if ( evaluation_stack.size() > 1 ) {
      std::cout << "WARNING: final stack size is " << evaluation_stack.size() << "\n";
    }
  }
  #endif

  return true;
}
