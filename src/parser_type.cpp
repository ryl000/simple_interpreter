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
#include <map>

#include "parser_type.h"


// Function call interface
//
// Right before jump-to-function:
//      ...
//      space reserved for return value(s), if any
//      argument(s), if any
// sp->
//
// The call will:
//    push the addr of the next instruction onto the stack
//    push the current stack frame base onto the stack
//    set the stack frame base to the current stack pointer
//
// So, inside the function:
//      ...
//      space reserved for return value(s), if any
//      argument(s), if any
//      return address
//      old stack frame addr
// sp->
//
// When the function does a return, it will:
//   set the sp-> to the stack frame addr
//   pop the old stack frame addr off the stack and
//    use that to set the stack frame addr
//   pop the return address off and jump to that address
//
// So upon return to the original caller:
//      ...
//      return value(s), if any
//      argument(s), if any
// sp->
//
// At this point, the caller will do any stack cleanup
// required (popping arguments off the stack, then handling
// what to do with return value(s))
//


namespace {

  
  struct operator_data_type {
    int         precedence;
    const char *text;
  };


  
  // NOTE: needs to match up with eval_id_type enum
  //
  const operator_data_type operator_data[] = {
    { 0, "push double" }
    ,{ 0, "push int" }

    ,{ 10, "not" }
    ,{ 10, "negate" }
   
    ,{ 1,  "(" }
    ,{ 1,  ")" }

    ,{ 0,  ";" }

    ,{ 0,  "clear-stack" }
    ,{ 0,  "pop" }
    ,{ 0,  "jnez" }
    ,{ 0,  "jeqz" }
    ,{ 0,  "jceqz" }
    ,{ 0,  "jmp" }
    ,{ 0,  "jmp-absolute" }

    ,{ 0,  "push-addr" }
    ,{ 0,  "push-addr-stack" }
    ,{ 0,  "copy-to-addr" }
    ,{ 0,  "copy-from-addr" }
    ,{ 0,  "copy-to-stack-offset" }
    ,{ 0,  "copy-from-stack-offset" }

    ,{ 0,  "move-end-of-stack" }
    ,{ 0,  "set-set-frame-base-to-end-of-stack" }
    ,{ 0,  "call" }
    ,{ 0,  "return" }

    ,{ 0,  "print-dstack" }
    
    ,{ 9,  "fn" }

    ,{ 8,  "add" }
    ,{ 8,  "subtract" }

    ,{ 9,  "divide" }
    ,{ 9,  "mutliply" }
   
    ,{ 6,  "eq" }
    ,{ 6,  "ne" }
    ,{ 7,  "ge" }
    ,{ 7,  "gt" }
    ,{ 7,  "le" }
    ,{ 7,  "lt" }
   
    ,{ 5,  "and" }
    ,{ 4,  "or" }
   
    ,{ 2,  "comma" }

    ,{ 3,  "assign" }
   
  };


  bool is_keyword( const std::string &name )
  {
    if ( std::strcmp( "else", name.c_str() ) == 0 ) {
      return true;
    }
    else if ( std::strcmp( "fn", name.c_str() ) == 0 ) {
      return true;
    }
    else if ( std::strcmp( "if", name.c_str() ) == 0 ) {
      return true;
    }
    else if ( std::strcmp( "return", name.c_str() ) == 0 ) {
      return true;
    }
    else if ( std::strcmp( "while", name.c_str() ) == 0 ) {
      return true;
    }
    else if ( std::strcmp( "double", name.c_str() ) == 0 ) {
      return true;
    }

    return false;
  }
  
}


bool parser_type::anchor_jump_here_( size_t jump_idx )
{
  // TODO. add checks?
  // TODO. guard against invalid index?
  // TODO. guard against invalid offset calc?
  statements_[ jump_idx ].jump_arg = statements_.size() - jump_idx;
  return true;
}


// TODO. need to add symbol_table_ as a parameter?
bool parser_type::statement_parser_( const token_type &last_token )
{
  switch ( parse_mode_ ) {

  case PARSE_MODE_START:
    {
      if ( last_token.id == TOKEN_ID_TYPE_NAME ) {
	// TODO. unify name lookup code
	bool symbol_found = false;
	for ( auto st_iter = symbol_table_.rbegin()
		; !symbol_found && st_iter != symbol_table_.rend()
		; ++st_iter ) {
	  auto iter = st_iter->find( last_token.text );
	  if ( iter != st_iter->end() ) {
	    if ( iter->second.type == SYMBOL_TYPE_VARIABLE ) {
	      if ( !(iter->second.is_abs) ) {
		statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_COPYFROMSTACKOFFSET ) );
		statements_.back().offset_arg = iter->second.sfb_offset;
	      }
	      else {
		statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_COPYFROMADDR ) );
		statements_.back().addr_arg = iter->second.addr;
	      }
	      parse_mode_ = PARSE_MODE_OPERATOR_EXPECTED;
	      symbol_found = true;
	    }
	    else {
	      // TODO. if this fn returns void, it cannot be part of
	      // a "compound" expression
	      eval_data_type new_fn( EVAL_ID_TYPE_OP_FN );

	      // TODO. for now, only "absolute" addresses allowed
	      if ( !(iter->second.is_abs) ) {
		std::cout << "ERROR: nested functions not currently allowed\n";
		break;
	      }
	      new_fn.addr_arg = iter->second.addr;
	      new_fn.symbol_data = &(iter->second);
	      update_stacks_with_operator_( new_fn );
	      parse_mode_ = PARSE_MODE_FN_LPARENS_EXPECTED;
	      symbol_found = true;
	    }
	  }
	}

	if ( !symbol_found ) {
	  std::cout << "ERROR(A): symbol " << last_token.text << " cannot be found\n";
	  parse_mode_ = PARSE_MODE_ERROR;
	}
      }
      else if ( last_token.id == TOKEN_ID_TYPE_NUMBER ) {
	statements_.emplace_back( std::atof( last_token.text.c_str() ) );
	parse_mode_ = PARSE_MODE_OPERATOR_EXPECTED;
      }
      else if ( last_token.id == TOKEN_ID_TYPE_PLUS ||
		last_token.id == TOKEN_ID_TYPE_MINUS ||
		last_token.id == TOKEN_ID_TYPE_NOT ) {
	if ( last_token.id == TOKEN_ID_TYPE_MINUS ) {
	  update_stacks_with_operator_( eval_data_type( EVAL_ID_TYPE_OP_NEGATE ) );
	}
	else if ( last_token.id == TOKEN_ID_TYPE_NOT ) {
	  update_stacks_with_operator_( eval_data_type( EVAL_ID_TYPE_OP_NOT ) );
	}
	else {
	  // Nothing needs to be done for unary +
	}
	parse_mode_ = PARSE_MODE_OPERAND_EXPECTED;
      }
      else if ( last_token.id == TOKEN_ID_TYPE_LPARENS ) {
	update_stacks_with_operator_( eval_data_type( EVAL_ID_TYPE_OP_LPARENS ) );
	parse_mode_ = PARSE_MODE_OPERAND_EXPECTED;
      }
      else {
	parse_mode_ = PARSE_MODE_ERROR;
      }
    }
    break;
      
  case PARSE_MODE_OPERAND_EXPECTED:
    {
      if ( last_token.id == TOKEN_ID_TYPE_NAME ) {
	// TODO. unify name lookup code
	bool symbol_found = false;
	for ( auto st_iter = symbol_table_.rbegin()
		; !symbol_found && st_iter != symbol_table_.rend()
		; ++st_iter ) {
	  auto iter = st_iter->find( last_token.text );
	  if ( iter != st_iter->end() ) {
	    if ( iter->second.type == SYMBOL_TYPE_VARIABLE ) {
	      if ( !(iter->second.is_abs) ) {
		statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_COPYFROMSTACKOFFSET ) );
		statements_.back().offset_arg = iter->second.sfb_offset;
	      }
	      else {
		statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_COPYFROMADDR ) );
		statements_.back().addr_arg = iter->second.addr;
	      }
	      parse_mode_ = PARSE_MODE_OPERATOR_EXPECTED;
	      symbol_found = true;
	    }
	    else {
	      // TODO. if this fn returns void, it cannot be part of
	      // a "compound" expression
	      eval_data_type new_fn( EVAL_ID_TYPE_OP_FN );

	      // TODO. for now, only "absolute" addresses allowed
	      if ( !(iter->second.is_abs) ) {
		std::cout << "ERROR: nested functions not currently allowed\n";
		break;
	      }
	      new_fn.addr_arg = iter->second.addr;
	      new_fn.symbol_data = &(iter->second);
	      update_stacks_with_operator_( new_fn );
	      parse_mode_ = PARSE_MODE_FN_LPARENS_EXPECTED;
	      symbol_found = true;
	    }
	  }
	}

	if ( !symbol_found ) {
	  std::cout << "ERROR(A): symbol " << last_token.text << " cannot be found\n";
	  parse_mode_ = PARSE_MODE_ERROR;
	}
      }
      else if ( last_token.id == TOKEN_ID_TYPE_NUMBER ) {
	statements_.emplace_back( std::atof( last_token.text.c_str() ) );
	parse_mode_ = PARSE_MODE_OPERATOR_EXPECTED;
      }
      else if ( last_token.id == TOKEN_ID_TYPE_PLUS ||
		last_token.id == TOKEN_ID_TYPE_MINUS ||
		last_token.id == TOKEN_ID_TYPE_NOT ) {
	if ( last_token.id == TOKEN_ID_TYPE_MINUS ) {
	  update_stacks_with_operator_( eval_data_type( EVAL_ID_TYPE_OP_NEGATE ) );
	}
	else if ( last_token.id == TOKEN_ID_TYPE_NOT ) {
	  update_stacks_with_operator_( eval_data_type( EVAL_ID_TYPE_OP_NOT ) );
	}
	else {
	  // Nothing needs to be done for unary +
	}
	// stay in this parse mode
      }
      else if ( last_token.id == TOKEN_ID_TYPE_LPARENS ) {
	update_stacks_with_operator_( eval_data_type( EVAL_ID_TYPE_OP_LPARENS ) );
	// stay in this parse mode
      }
      #if 0
      else if ( last_token.id == TOKEN_ID_TYPE_RPARENS ) {
	// TODO. restrict this to function mode only!
	//  i.e., xyz() is allowed, but
	//  3 + () should not be
	//
	if ( !update_stacks_with_operator_( eval_data_type( EVAL_ID_TYPE_OP_RPARENS ) ) ) {
	  parse_mode_ = PARSE_MODE_ERROR;
	}
	// stay in this parse mode
      }
      #endif
      else {
	parse_mode_ = PARSE_MODE_ERROR;
      }
    }
    break;
      
  case PARSE_MODE_OPERATOR_EXPECTED:
    if ( last_token.id == TOKEN_ID_TYPE_PLUS ||
	 last_token.id == TOKEN_ID_TYPE_MINUS ||
	 last_token.id == TOKEN_ID_TYPE_DIVIDE ||
	 last_token.id == TOKEN_ID_TYPE_MULTIPLY ||
	 last_token.id == TOKEN_ID_TYPE_ASSIGN ||
	 last_token.id == TOKEN_ID_TYPE_COMMA ||
	 last_token.id == TOKEN_ID_TYPE_EQ ||
	 last_token.id == TOKEN_ID_TYPE_GE ||
	 last_token.id == TOKEN_ID_TYPE_GT ||
	 last_token.id == TOKEN_ID_TYPE_LE ||
	 last_token.id == TOKEN_ID_TYPE_LT ||
	 last_token.id == TOKEN_ID_TYPE_NEQ ||
	 last_token.id == TOKEN_ID_TYPE_AND ||
	 last_token.id == TOKEN_ID_TYPE_OR ) {
      eval_id_type new_eval_id_type;
      if ( !token_id_to_eval_id_( last_token.id, &new_eval_id_type ) ) {
	parse_mode_ = PARSE_MODE_ERROR;
      }
      else {
	
	if ( last_token.id == TOKEN_ID_TYPE_ASSIGN ) {
	  // If we've come across an assignment token, the
	  // most recent statement must be a copyfromaddress
	  // (i.e., a variable name). We will convert this to
	  // a push-addr, because the assign-op needs that addr
	  // for the assignment
	  //
	  // TODO. we need to distinguish assign-to-addr and
	  //  assign-to-stack-offset!
	  //
	  // TODO. instead of having
	  //  PUSHADDR x
	  //  ...
	  //  ASSIGN
	  // do
	  //  ...
	  //  ASSIGN x
	  // ?
	  //
	  // OR.
	  //  (absolute)
	  //  PUSHADDR x
	  //  PUSHD    1
	  //  ...
	  //  ASSIGN
	  //
	  //  (stack-offset)
	  //  PUSHADDR x
	  //  PUSHD    0
	  //  ...
	  //  ASSIGN
	  //
	  if ( statements_.empty() ) {
	    parse_mode_ = PARSE_MODE_ERROR;
	  }
	  else if ( statements_.back().id == EVAL_ID_TYPE_OP_COPYFROMSTACKOFFSET ) {
	    statements_.back().id = EVAL_ID_TYPE_OP_PUSHSTACKOFFSET;
	    statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_PUSHI ) );
	    statements_.back().ivalue = 0;
	  }
	  else if ( statements_.back().id == EVAL_ID_TYPE_OP_COPYFROMADDR ) {
	    statements_.back().id = EVAL_ID_TYPE_OP_PUSHADDR;
	    statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_PUSHI ) );
	    statements_.back().ivalue = 1;
	  }
	  else {
	    parse_mode_ = PARSE_MODE_ERROR;
	  }
	}

	if ( parse_mode_ != PARSE_MODE_ERROR ) {
	  if ( !update_stacks_with_operator_( eval_data_type( new_eval_id_type ) ) ) {
	    parse_mode_ = PARSE_MODE_ERROR;
	  }
	  else {
	    parse_mode_ = PARSE_MODE_OPERAND_EXPECTED;
	  }
	}
      }
    }
    else if ( last_token.id == TOKEN_ID_TYPE_RPARENS ) {
      if ( !update_stacks_with_operator_( eval_data_type( EVAL_ID_TYPE_OP_RPARENS ) ) ) {
	parse_mode_ = PARSE_MODE_ERROR;
      }
    }
    else {
      parse_mode_ = PARSE_MODE_ERROR;
    }
    break;

    
  case PARSE_MODE_VARIABLE_DEFINITION_START:
    if ( last_token.id == TOKEN_ID_TYPE_ASSIGN ) {
      if ( !update_stacks_with_operator_( eval_data_type( EVAL_ID_TYPE_OP_ASSIGN ) ) ) {
	parse_mode_ = PARSE_MODE_ERROR;
      }
      parse_mode_ = PARSE_MODE_OPERAND_EXPECTED;
    }
    else {
      parse_mode_ = PARSE_MODE_ERROR;
    }
    break;

  case PARSE_MODE_FN_LPARENS_EXPECTED:
    if ( last_token.id == TOKEN_ID_TYPE_LPARENS ) {
      update_stacks_with_operator_( eval_data_type( EVAL_ID_TYPE_OP_LPARENS ) );
      parse_mode_ = PARSE_MODE_OPERAND_EXPECTED;
    }
    else {
      parse_mode_ = PARSE_MODE_ERROR;
    }
    break;


  case PARSE_MODE_ERROR:
    // do nothing
    break;

  }

  return ( parse_mode_ != PARSE_MODE_ERROR );
}


bool parser_type::statement_parser_finalize_()
{
  if ( !update_stacks_with_operator_( eval_data_type( EVAL_ID_TYPE_OP_FINALIZE ) ) ) {
    return false;
  }
  parse_mode_ = PARSE_MODE_START;
  return true;
}


bool parser_type::token_id_to_eval_id_(
				       token_id_type token_id
				       ,eval_id_type *eval_id
				       )
{
  bool rv = false;

  if ( token_id >= TOKEN_ID_TYPE_PLUS ) {
    *eval_id = static_cast<eval_id_type>( token_id + EVAL_ID_FIRST_DIRECT_TOKEN_TO_CMD - TOKEN_ID_FIRST_DIRECT_TOKEN_TO_CMD );
    rv = true;
  }

  return rv;
}


bool parser_type::update_stacks_with_operator_(
					       eval_data_type                       eval_data
					       )
{
  eval_id_type eval_id = eval_data.id;
  
  if ( eval_id == EVAL_ID_TYPE_OP_LPARENS ) {
      
    lparens_.push_back( operator_stack_.size() );
      
  }
  else {

    // For a closing parens, make sure parenthesis is balanced
    //
    if ( (eval_id == EVAL_ID_TYPE_OP_RPARENS) && lparens_.empty() ) {
      return false;
    }

    // For finalize, make sure no un-matched left parens exist
    else if ( (eval_id == EVAL_ID_TYPE_OP_FINALIZE) && !lparens_.empty() ) {
      return false;
    }

    // The elements popped off the operator stack will be added to the
    //  statements stack
    //
	
    while ( !operator_stack_.empty() ) {

      // For a closing parens, pop elements off operator stack until the
      //  matching opening parens is found
      //
      if ( eval_id == EVAL_ID_TYPE_OP_RPARENS ) {

	if ( operator_stack_.size() == lparens_.back() ) {
	  break;
	}

      }
      else {

	// Otherwise, pop elements off operator stack until either the
	// current parenthesis level has been exhausted...
	//
	if ( !lparens_.empty() && (operator_stack_.size() <= lparens_.back()) ) {
	  break;
	}

	// ...Or the topmost operator in the operator stack has a < precedence
	//
	if ( operator_data[ eval_id ].precedence >= operator_data[ operator_stack_.back().id ].precedence ) {
	  break;
	}
      }

      if ( operator_stack_.back().id == EVAL_ID_TYPE_OP_FN ) {
	// TODO. allow for void returns
	size_t stack_space = 8U;

	if ( operator_stack_.back().symbol_data->fn_nargs > 0U ) {
	  stack_space += (operator_stack_.back().symbol_data->fn_nargs * 8U);
	}

	// reserve space on dstack for return value + args
	size_t ret_val_offset = current_offset_from_stack_frame_base_.back();
	statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_MOVE_END_OF_STACK ) );
	statements_.back().jump_arg = stack_space;
	current_offset_from_stack_frame_base_.back() += stack_space;

	// transfer any args from estack to dstack
	if ( operator_stack_.back().symbol_data->fn_nargs > 0U ) {
	  int32_t offset = -8;
	  for ( size_t i=0U; i<operator_stack_.back().symbol_data->fn_nargs; ++i, offset -= 8 ) {
	    statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_COPYTOSTACKOFFSET ) );
	    statements_.back().offset_arg = current_offset_from_stack_frame_base_.back() + offset;
	    statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_POP ) );
	    statements_.back().pop_arg = 1U;
	  }
	}
	
	statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_DEBUG_PRINT_STACK ) );
	
	statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_CALL ) );
	statements_.back().addr_arg = operator_stack_.back().addr_arg;

	// adjust stack to remove the args that were passed to the function
	if ( operator_stack_.back().symbol_data->fn_nargs > 0U ) {
	  statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_MOVE_END_OF_STACK ) );
	  statements_.back().jump_arg = operator_stack_.back().symbol_data->fn_nargs * -8;
	}

	statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_DEBUG_PRINT_STACK ) );

	// transfer return value to estack
	// TODO. allow for void returns!
	//
	statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_COPYFROMSTACKOFFSET ) );
	statements_.back().offset_arg = ret_val_offset;
      }
      else {
	statements_.emplace_back( operator_stack_.back() );
      }

      // If && or || is pushed into the statements stack, we need to resolve any previously-pushed
      // JNEZ/JEQZ with the correct jump arg
      //
      if ( operator_stack_.back().id == EVAL_ID_TYPE_OP_AND
	   || operator_stack_.back().id == EVAL_ID_TYPE_OP_OR ) {
	// TODO. check return value?
	anchor_jump_here_( operator_stack_.back().jump_arg );
	statements_.back().jump_arg = 0;
      }

      operator_stack_.pop_back();

    }


    // If this was a closing parens, take care of the
    //  opening parens
    //
    if ( eval_id == EVAL_ID_TYPE_OP_RPARENS ) {
      lparens_.pop_back();
    }

    // Otherwise, if this is not a comma or "finalize", add it to the
    //  operator stack
    //
    else if ( eval_id != EVAL_ID_TYPE_OP_COMMA && eval_id != EVAL_ID_TYPE_OP_FINALIZE ) {
	
      operator_stack_.emplace_back( eval_data );

      // If && or ||, we need to add a JEQZ/JNEZ into the statements, to handle short-circuits
      // NOTE that we are using the "jump arg" field in the && or || to
      //  store the location of the associated JEQZ/JNEZ. This is faster than
      //  searching backwards at the time the && or || is pushed into the statements stack
      //
      if ( eval_id == EVAL_ID_TYPE_OP_AND ) {
	statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_JEQZ ) );
	operator_stack_.back().jump_arg = statements_.size() - 1U;
      }
      else if ( eval_id == EVAL_ID_TYPE_OP_OR ) {
	statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_JNEZ ) );
	operator_stack_.back().jump_arg = statements_.size() - 1U;
      }
	
    }

  }
    
  return true;
}


bool parser_type::parse_char( char c )
{
  ++char_no_;

  // Stage 1: Tokenize
  //
  {
    bool reprocess  = false;

    do {
    
      switch ( lex_mode_ ) {

      case LEX_MODE_START:
	reprocess = false;
	if ( std::isdigit( c ) ) {
	  current_token_ += c;
	  lex_mode_ = LEX_MODE_NUMBER_START_DIGIT;
	}
	else if ( c == '.' ) {
	  current_token_ += c;
	  lex_mode_ = LEX_MODE_NUMBER_START_DECIMAL;
	}
	else if ( c == '_' || std::isalpha( c ) ) {
	  current_token_ += c;
	  lex_mode_ = LEX_MODE_NAME_START;
	}
	else if ( std::isspace( c ) ) {
	  // skip, and stay in this mode
	  if ( c == '\n' ) {
	    ++line_no_;
	    char_no_ = 0;
	  };
	}
	else if ( c == '+' ) {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_PLUS ) );
	}
	else if ( c == '-' ) {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_MINUS ) );
	}
	else if ( c == '/' ) {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_DIVIDE ) );
	}
	else if ( c == '*' ) {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_MULTIPLY ) );
	}
	else if ( c == '(' ) {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_LPARENS ) );
	}
	else if ( c == ')' ) {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_RPARENS ) );
	}
	else if ( c == ',' ) {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_COMMA ) );
	}
	else if ( c == ';' ) {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_SEMICOLON ) );
	}
	else if ( c == '{' ) {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_LCURLY_BRACE ) );
	}
	else if ( c == '}' ) {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_RCURLY_BRACE ) );
	}
	else if ( c == '=' ) {
	  lex_mode_ = LEX_MODE_EQ_CHECK;
	}
	else if ( c == '>' ) {
	  lex_mode_ = LEX_MODE_GT_CHECK;
	}
	else if ( c == '<' ) {
	  lex_mode_ = LEX_MODE_LT_CHECK;
	}
	else if ( c == '!' ) {
	  lex_mode_ = LEX_MODE_NOT_CHECK;
	}
	else if ( c == '&' ) {
	  lex_mode_ = LEX_MODE_AND_CHECK;
	}
	else if ( c == '|' ) {
	  lex_mode_ = LEX_MODE_OR_CHECK;
	}
	else if ( c == '#' ) {
	  lex_mode_ = LEX_MODE_COMMENT;
	}
	else if ( c == '\0' ) {
	  lex_mode_ = LEX_MODE_END_OF_INPUT;
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_END_OF_INPUT ) );
	}
	else {
	  lex_mode_ = LEX_MODE_ERROR;
	}
	break;

      case LEX_MODE_COMMENT:
	if ( c == '\n' ) {
	  lex_mode_ = LEX_MODE_START;
	}
	break;
      
      case LEX_MODE_NUMBER_START_DIGIT:
	if ( std::isdigit( c ) ) {
	  current_token_ += c;
	}
	else if ( c == '.' ) {
	  current_token_ += c;
	  lex_mode_     = LEX_MODE_NUMBER_DECIMAL;
	}
	else if ( c == 'e' || c == 'E' ) {
	  current_token_ += c;
	  lex_mode_     = LEX_MODE_NUMBER_EXPONENT;
	}
	else {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_NUMBER, current_token_ ) );

	  current_token_.clear();
	  lex_mode_ = LEX_MODE_START;
	  reprocess     = true;
	}
	break;
      
      case LEX_MODE_NUMBER_START_DECIMAL:
	if ( std::isdigit( c ) ) {
	  current_token_ += c;
	  lex_mode_     = LEX_MODE_NUMBER_FRACTION;
	}
	else {
	  lex_mode_ = LEX_MODE_ERROR;
	}
	break;
      
      case LEX_MODE_NUMBER_DECIMAL:
	if ( std::isdigit( c ) ) {
	  current_token_ += c;
	  lex_mode_     = LEX_MODE_NUMBER_FRACTION;
	}
	else {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_NUMBER, current_token_ ) );

	  current_token_.clear();
	  lex_mode_ = LEX_MODE_START;
	  reprocess     = true;
	}
	break;
      
      case LEX_MODE_NUMBER_FRACTION:
	if ( std::isdigit( c ) ) {
	  current_token_ += c;
	}
	else if ( c == 'e' || c == 'E' ) {
	  current_token_ += c;
	  lex_mode_     = LEX_MODE_NUMBER_EXPONENT;
	}
	else {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_NUMBER, current_token_ ) );

	  current_token_.clear();
	  lex_mode_ = LEX_MODE_START;
	  reprocess     = true;
	}
	break;
      
      case LEX_MODE_NUMBER_EXPONENT:
	if ( c == '+' || c == '-' ) {
	  current_token_ += c;
	  lex_mode_     = LEX_MODE_NUMBER_EXPONENT_SIGN;
	}
	else if ( std::isdigit( c ) ) {
	  current_token_ += c;
	  lex_mode_     = LEX_MODE_NUMBER_EXPONENT_DIGIT;
	}
	else {
	  lex_mode_ = LEX_MODE_ERROR;
	}
	break;
      
      case LEX_MODE_NUMBER_EXPONENT_SIGN:
	if ( std::isdigit( c ) ) {
	  current_token_ += c;
	  lex_mode_     = LEX_MODE_NUMBER_EXPONENT_DIGIT;
	}
	else {
	  lex_mode_ = LEX_MODE_ERROR;
	}
	break;
      
      case LEX_MODE_NUMBER_EXPONENT_DIGIT:
	if ( std::isdigit( c ) ) {
	  current_token_ += c;
	}
	else {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_NUMBER, current_token_ ) );
	
	  current_token_.clear();
	  lex_mode_ = LEX_MODE_START;
	  reprocess     = true;
	}
	break;
      
      case LEX_MODE_NAME_START:
	if ( c == '_' || std::isalpha( c ) || std::isdigit( c ) ) {
	  current_token_ += c;
	}
	else {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_NAME, current_token_ ) );
	
	  current_token_.clear();
	  lex_mode_ = LEX_MODE_START;
	  reprocess     = true;
	}
	break;

      case LEX_MODE_EQ_CHECK:
	if ( c == '=' ) {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_EQ ) );
	  lex_mode_ = LEX_MODE_START;
	}
	else {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_ASSIGN ) );

	  current_token_.clear();
	  lex_mode_ = LEX_MODE_START;
	  reprocess     = true;
	}
	break;

      case LEX_MODE_GT_CHECK:
	if ( c == '=' ) {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_GE ) );
	  lex_mode_ = LEX_MODE_START;
	}
	else {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_GT ) );

	  current_token_.clear();
	  lex_mode_ = LEX_MODE_START;
	  reprocess     = true;
	}
	break;
	
      case LEX_MODE_LT_CHECK:
	if ( c == '=' ) {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_LE ) );
	  lex_mode_ = LEX_MODE_START;
	}
	else {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_LT ) );

	  current_token_.clear();
	  lex_mode_ = LEX_MODE_START;
	  reprocess     = true;
	}
	break;

      case LEX_MODE_NOT_CHECK:
	if ( c == '=' ) {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_NEQ ) );
	  lex_mode_ = LEX_MODE_START;
	}
	else {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_NOT ) );

	  current_token_.clear();
	  lex_mode_ = LEX_MODE_START;
	  reprocess     = true;
	}
	break;

      case LEX_MODE_AND_CHECK:
	if ( c == '&' ) {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_AND ) );
	  lex_mode_ = LEX_MODE_START;
	}
	else {
	  lex_mode_ = LEX_MODE_ERROR;
	}
	break;

      case LEX_MODE_OR_CHECK:
	if ( c == '|' ) {
	  tokens_.emplace_back( token_type( TOKEN_ID_TYPE_OR ) );
	  lex_mode_ = LEX_MODE_START;
	}
	else {
	  lex_mode_ = LEX_MODE_ERROR;
	}
	break;

      case LEX_MODE_END_OF_INPUT:
      case LEX_MODE_ERROR:
	// Do nothing
	//
	break;
      }
    
    } while ( reprocess );

  }

  
  if ( lex_mode_ == LEX_MODE_ERROR ) {
    std::cerr << "ERROR: lex error on character " << c << "\n";
    return false;
  }


  // Stage 2: Grammar
  //
  if ( tokens_.size() > tokens_parsed_ ) {
    
    while ( tokens_parsed_ < tokens_.size() ) {

      bool reprocess = false;

      const token_type &last_token = tokens_[tokens_parsed_];

      do {
	reprocess = false;

	switch ( grammar_state_.back().mode ) {
	case GRAMMAR_MODE_STATEMENT_START:
	  if ( last_token.id == TOKEN_ID_TYPE_NAME ) {
	    if ( std::strcmp( "if", last_token.text.c_str() ) == 0 ) {
	      grammar_state_.back().mode = GRAMMAR_MODE_BRANCH_STATEMENT;
	      grammar_state_.back().branching_mode = BRANCHING_MODE_IF;
	    }
	    else if ( std::strcmp( "while", last_token.text.c_str() ) == 0 ) {
	      grammar_state_.back().mode = GRAMMAR_MODE_BRANCH_STATEMENT;
	      grammar_state_.back().branching_mode = BRANCHING_MODE_WHILE;
	      grammar_state_.back().loopback_offset = statements_.size();
	    }
	    // TODO. instead of allowing
	    //  double x;
	    //  int y;
	    // do this instead?
	    //  var double x;
	    //  var int y;
	    // to be consistent with
	    //  fn double x() {}
	    //  fn int y() {}
	    //
	    else if ( std::strcmp( "double", last_token.text.c_str() ) == 0 ) {
	      grammar_state_.back().mode = GRAMMAR_MODE_DEFINE_VARIABLE;
	    }
	    else if ( std::strcmp( "fn", last_token.text.c_str() ) == 0 ) {
	      // TODO. disallow "fn" inside fn...
	      grammar_state_.back().mode = GRAMMAR_MODE_DEFINE_FUNCTION_START;
	      
	      // add a jump, that will be fixed at end-of-function to jump
	      // past function contents
	      //
	      size_t new_jmp_idx = statements_.size();
	      statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_JMP ) );
	      grammar_state_.back().jump_offset = new_jmp_idx;
	    }
	    else if ( std::strcmp( "return", last_token.text.c_str() ) == 0 ) {
	      // TODO. only allow inside a function
	      grammar_state_.back().return_mode = true;
	      grammar_state_.back().mode        = GRAMMAR_MODE_STATEMENT;
	    }
	    // TODO. add int?
	    else {
	      grammar_state_.back().mode = GRAMMAR_MODE_STATEMENT;
	      reprocess                  = true;
	    }
	  }
	  else if ( last_token.id == TOKEN_ID_TYPE_LCURLY_BRACE ) {
	    // NOTE: This is a block start
	    ++curly_braces_;
	    symbol_table_.push_back( std::map<std::string,symbol_table_data_type>() );
	    // TODO. the following two are related to stack frame, not curly brace level!
	    // Instead of being initialized to zero, they should be set to top-most value
	    //
	    // ("global")
	    // stack-base 0
	    //
	    // fn ()
	    // {
	    //   stack-base is non-zero
	    //   push new symbol_table_
	    //   push new current_new_var_idx_ (value = 0)
	    //   push new new_variable_index_ (value = 0)
	    //   {
	    //     push new symbol_table_
	    //     push new current_new_var_idx_ (value = topmost current_new_var_idx_)
	    //     push new new_variable_index_ (value = topmost new_variable_index_)
	    //   }
	    //   pop symboL_table_
	    //   pop current_new_var_idx_
	    //   pop new_variable_index_
	    //
	    if ( !current_new_var_idx_.empty() ) {
	      size_t prev = current_new_var_idx_.back();
	      current_new_var_idx_.push_back( prev );
	    }
	    if ( !new_variable_index_.empty() ) {
	      size_t prev = new_variable_index_.back();
	      new_variable_index_.push_back( prev );
	    }
	    if ( !current_offset_from_stack_frame_base_.empty() ) {
	      size_t prev = current_offset_from_stack_frame_base_.back();
	      current_offset_from_stack_frame_base_.push_back( prev );
	    }
	  }
	  else if ( last_token.id == TOKEN_ID_TYPE_RCURLY_BRACE ) {
	    if ( curly_braces_ ) {
	      --curly_braces_;
	      symbol_table_.pop_back();
	      current_new_var_idx_.pop_back();
	      current_offset_from_stack_frame_base_.pop_back();
	      if ( !new_variable_index_.empty() ) {
		size_t end_of_prev_block_new_variable_index = new_variable_index_.back();
		new_variable_index_.pop_back();
		// If variables were defined in the most recent block, we need to 'pop'
		// them off the d-stack
		//
		if ( end_of_prev_block_new_variable_index > new_variable_index_.back() ) {
		  statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_MOVE_END_OF_STACK ) );
		  statements_.back().jump_arg = new_variable_index_.back() - end_of_prev_block_new_variable_index;
		}
	      }
	      grammar_state_.back().mode = GRAMMAR_MODE_STATEMENT_END;
	      reprocess                  = true;
	    }
	    else {
	      grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
	    }
	  }
	  else if ( last_token.id == TOKEN_ID_TYPE_END_OF_INPUT ) {
	    grammar_state_.back().mode = GRAMMAR_MODE_END_OF_INPUT;
	  }
	  else {
	    grammar_state_.back().mode = GRAMMAR_MODE_STATEMENT;
	    reprocess                  = true;
	  }
	  break;

	case GRAMMAR_MODE_DEFINE_VARIABLE:
	  if ( last_token.id == TOKEN_ID_TYPE_NAME ) {
	    if ( is_keyword( last_token.text ) ) {
	      std::cout << "ERROR: keyword found\n";
	      grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
	      break;
	    }
	    
	    // TODO. scoping
	    auto iter = symbol_table_.back().find( last_token.text );
	    if ( iter != symbol_table_.back().end() ) {
	      std::cout << "ERROR: symbol already defined\n";
	      grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
	      break;
	    }

	    // TODO. always add doubles on 8-byte boundary,
	    // always add int32's on 4-byte boundary
	    //
	    current_new_var_idx_.back() = new_variable_index_.back();
	    new_variable_index_.back() += 8U; // size of double

	    statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_MOVE_END_OF_STACK ) );
	    statements_.back().jump_arg = 8; // size of double
	    current_offset_from_stack_frame_base_.back() += 8U;

	    symbol_table_data_type new_variable;
	    if ( symbol_table_.size() == 1 ) {
	      new_variable.is_abs = true;
	      new_variable.addr   = current_new_var_idx_.back();
	    }
	    else {
	      new_variable.sfb_offset  = current_new_var_idx_.back();
	    }
	    new_variable.type        = SYMBOL_TYPE_VARIABLE;
	    // TODO. more efficient insert, using find_lower_bound
	    symbol_table_.back().insert( std::make_pair( last_token.text, new_variable ) );
	    grammar_state_.back().mode = GRAMMAR_MODE_CHECK_FOR_ASSIGN;
	  }
	  else {
	    grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
	  }
	  break;

	case GRAMMAR_MODE_CHECK_FOR_ASSIGN:
	  if ( last_token.id == TOKEN_ID_TYPE_ASSIGN ) {
	    grammar_state_.back().mode = GRAMMAR_MODE_NEW_VARIABLE_ASSIGNMENT;
	  }
	  else if ( last_token.id == TOKEN_ID_TYPE_SEMICOLON ) {
	    grammar_state_.back().mode = GRAMMAR_MODE_STATEMENT_END;
	    reprocess = true;
	  }
	  else {
	    grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
	  }
	  break;

	case GRAMMAR_MODE_NEW_VARIABLE_ASSIGNMENT:
	  if ( last_token.id == TOKEN_ID_TYPE_SEMICOLON ) {
	    if ( !statement_parser_finalize_() ) {
	      grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
	    }
	    else {
	      // TODO. distinguish between copy-to-absolute (for globals)
	      //  vs copy-to-stack (for stack-local)
	      //
	      if ( symbol_table_.size() == 1 ) {
		statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_COPYTOADDR ) );
		statements_.back().addr_arg = current_new_var_idx_.back();
	      }
	      else {
		statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_COPYTOSTACKOFFSET ) );
		statements_.back().offset_arg = current_new_var_idx_.back();
	      }
	      statements_.push_back( eval_data_type( EVAL_ID_TYPE_OP_CLEAR ) );
	      grammar_state_.back().mode = GRAMMAR_MODE_STATEMENT_START;
	    }
	  }
	  else {
	    if ( !statement_parser_( last_token ) ) {
	      grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
	    }
	  }
	  break;
	  
	case GRAMMAR_MODE_BRANCH_STATEMENT:
	  if ( last_token.id == TOKEN_ID_TYPE_LPARENS ) {
	    grammar_state_.back().mode = GRAMMAR_MODE_BRANCH_EXPRESSION;
	  }
	  else {
	    grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
	  }
	  break;

	case GRAMMAR_MODE_BRANCH_EXPRESSION:
	  if ( last_token.id == TOKEN_ID_TYPE_RPARENS && lparens_.empty() ) {
	    if ( !statement_parser_finalize_() ) {
	      std::cerr << "ERROR(1): parse error on character " << c << "\n";
	      grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
	      break;
	    }

	    // TODO. Is there an easy way to unify JCEQZ and JEQZ while maintaining
	    // our stack-based evaluation?
	    //
	    grammar_state_.back().jump_offset = statements_.size();
	    statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_JCEQZ ) );
	    grammar_state_.back().mode = GRAMMAR_MODE_BRANCH_CLAUSE;
	    grammar_state_.emplace_back( grammar_state_type( GRAMMAR_MODE_STATEMENT_START, curly_braces_ ) );
	  }
	  else if ( !statement_parser_( last_token ) ) {
	    std::cerr << "ERROR(2): parse error on character " << c << "\n";
	    grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
	  }
	  break;
	  
	case GRAMMAR_MODE_STATEMENT:
	  if ( last_token.id == TOKEN_ID_TYPE_SEMICOLON ) {
	    if ( !statement_parser_finalize_() ) {
	      std::cerr << "ERROR(3): parse error on character " << c << "\n";
	      return false;
	    }

	    if ( grammar_state_.back().return_mode ) {
	      // pop estack value and place in return location
	      int32_t offset = -24;
	      offset -= (current_fn_iter_->second.fn_nargs * 8);
	      statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_COPYTOSTACKOFFSET ) );
	      statements_.back().offset_arg = offset;
	      
	      statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_POP ) );
	      statements_.back().pop_arg = 1U;

	      // return
	      statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_RETURN ) );

	      grammar_state_.back().return_mode = false;
	    }
	    // for semi-colon, we need to add an explicit command to clear the
	    //  evaluation stack. Might want to add size checking...
	    //  evaluation stack should either be down to 1 or 0 elements...
	    //
	    // TODO. only do this if we actually pushed elements into statements_
	    //  stack (versus previous "end")
	    //
	    statements_.push_back( eval_data_type( EVAL_ID_TYPE_OP_CLEAR ) );
	    
	    grammar_state_.back().mode = GRAMMAR_MODE_STATEMENT_END;
	    reprocess = true;
	  }
	  else if ( !statement_parser_( last_token ) ) {
	    std::cerr << "ERROR(4): parse error on character " << c << " (" << line_no_ << "," << char_no_ << ")\n";
	    return false;
	  }
	  
	  break;

	case GRAMMAR_MODE_STATEMENT_END:
	  {
	    bool mode_set = false;
	    if ( (grammar_state_.size() > 1) ) {

	      // "unwind" if/else as applicable
	      //
	      while ( grammar_state_.size() > 1
		      &&
		      (
		        ( (grammar_state_.rbegin() + 1U)->mode == GRAMMAR_MODE_BRANCH_CLAUSE)
		        ||
		        ( (grammar_state_.rbegin() + 1U)->mode == GRAMMAR_MODE_DEFINE_FUNCTION_BODY)
		      )
		      &&
		      ((grammar_state_.rbegin() )->block_depth == curly_braces_) ) {

		grammar_state_.pop_back();

		if ( (grammar_state_.rbegin())->mode == GRAMMAR_MODE_DEFINE_FUNCTION_BODY ) {

		  // TODO. need to detect if this function body was properly terminated with
		  // an ending return...
		  // All "terminating" code paths must end in a return x (if function
		  // does not return void)
		  //
		  
		  //  Fix-up the jump that was placed before the function
		  //    definition, so we jump past the function definition
		  //
		  anchor_jump_here_( grammar_state_.back().jump_offset );
		  grammar_state_.back().jump_offset = 0U;
		}
		else {

		  if ( (grammar_state_.rbegin())->branching_mode == BRANCHING_MODE_WHILE ) {
		    // Put an unconditional jmp at the end of the previous while clause,
		    // to go back to conditional check
		    size_t new_jmp_idx = statements_.size();
		    statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_JMP ) );
		    statements_.back().jump_arg = grammar_state_.rbegin()->loopback_offset - new_jmp_idx;  // TODO. unsigned subtract!
		    grammar_state_.rbegin()->loopback_offset = 0U;
		  }

		  if ( (grammar_state_.rbegin())->branching_mode != BRANCHING_MODE_IF ) {
		    // TODO. check return value?
		    anchor_jump_here_( grammar_state_.back().jump_offset );
		    grammar_state_.back().jump_offset = 0U;
		  }
		  else {
		    grammar_state_.back().mode = GRAMMAR_MODE_ELSE_CHECK;
		    mode_set = true;
		    break;
		  }
		}
		
	      }
	      
	    }
	    
	    if ( !mode_set ) {
	      grammar_state_.back().mode = GRAMMAR_MODE_STATEMENT_START;
	    }
	  }
	  break;

	case GRAMMAR_MODE_ELSE_CHECK:
	  if ( last_token.id == TOKEN_ID_TYPE_NAME && ( std::strcmp( "else", last_token.text.c_str() ) == 0 ) ) {
	    // Put an unconditional jmp at the end of the previous if clause
	    size_t new_jmp_idx = statements_.size();
	    statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_JMP ) );

	    // Fix-up the jump from the if expression
	    // TODO. check return value?
	    anchor_jump_here_( grammar_state_.back().jump_offset );

	    // Save this new jmp to fix later
	    grammar_state_.back().jump_offset = new_jmp_idx;
	    
	    grammar_state_.back().mode = GRAMMAR_MODE_BRANCH_CLAUSE;
	    grammar_state_.back().branching_mode = BRANCHING_MODE_ELSE;
	    grammar_state_.emplace_back( grammar_state_type( GRAMMAR_MODE_STATEMENT_START, curly_braces_ ) );
	  }
	  else {
	    anchor_jump_here_( grammar_state_.back().jump_offset );
	    
	    // "unwind" if/else as applicable
	    //
	    while ( grammar_state_.size() > 1
		    &&
		    ((grammar_state_.rbegin() + 1U)->mode == GRAMMAR_MODE_BRANCH_CLAUSE)
		    &&
		    ((grammar_state_.rbegin() )->block_depth == curly_braces_) ) {
	      grammar_state_.pop_back();

	      // TODO. check return value?
	      anchor_jump_here_( grammar_state_.back().jump_offset );
	    }

	    grammar_state_.back().mode = GRAMMAR_MODE_STATEMENT_START;
	    reprocess                  = true;
	  }
	  break;

	case GRAMMAR_MODE_BRANCH_CLAUSE:
	  // TODO. do we need this as an enum?
	  break;

	case GRAMMAR_MODE_DEFINE_FUNCTION_START:
	  {
	    bool fn_type_found = false;

	    if ( last_token.id == TOKEN_ID_TYPE_NAME ) {
	      if ( std::strcmp( "double", last_token.text.c_str() ) == 0 ) {
		grammar_state_.back().mode = GRAMMAR_MODE_EXPECT_FUNCTION_NAME;
		fn_type_found = true;
	      }
	    }

	    if ( !fn_type_found ) {
	      grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
	    }
	  }
	  break;
	  
	case GRAMMAR_MODE_EXPECT_FUNCTION_NAME:
	  {
	    if ( last_token.id == TOKEN_ID_TYPE_NAME ) {

	      if ( is_keyword( last_token.text ) ) {
		std::cout << "ERROR: keyword found\n";
		grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
		break;
	      }

	      auto iter = symbol_table_.back().find( last_token.text );
	      if ( iter != symbol_table_.back().end() ) {
		std::cout << "ERROR: symbol already defined\n";
		grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
		break;
	      }

	      symbol_table_data_type new_function;
	      // TODO. for now, functions always "absolute" adddresses
	      //  Maybe later, we can have nested functions...
	      new_function.is_abs     = true;
	      new_function.addr       = statements_.size();
	      new_function.type       = SYMBOL_TYPE_FUNCTION;
	      new_function.fn_nargs   = 0U;
	      // TODO. more efficient insert, using find_lower_bound
	      auto rv = symbol_table_.back().insert( std::make_pair( last_token.text, new_function ) );
	      current_fn_iter_ = rv.first;

	      symbol_table_.push_back( std::map<std::string,symbol_table_data_type>() );
	      new_variable_index_.push_back( 0U );
	      current_new_var_idx_.push_back( 0U );
	      current_offset_from_stack_frame_base_.push_back( 0U );
	      
	      grammar_state_.back().mode = GRAMMAR_MODE_EXPECT_FUNCTION_OPEN_PARENS;
	    }
	    else {
	      grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
	    }
	  }
	  break;

	case GRAMMAR_MODE_EXPECT_FUNCTION_OPEN_PARENS:
	  {
	    if ( last_token.id == TOKEN_ID_TYPE_LPARENS ) {
	      grammar_state_.back().mode = GRAMMAR_MODE_EXPECT_FUNCTION_ARG_TYPE;
	    }
	    else {
	      grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
	    }
	  }
	  break;

	case GRAMMAR_MODE_EXPECT_FUNCTION_ARG_TYPE:
	  {
	    if ( last_token.id == TOKEN_ID_TYPE_RPARENS ) {
	      grammar_state_.back().mode = GRAMMAR_MODE_EXPECT_FUNCTION_BODY_START;
	    }
	    else {
	      bool arg_type_found = false;
	      if ( last_token.id == TOKEN_ID_TYPE_NAME ) {
		if ( std::strcmp( "double", last_token.text.c_str() ) == 0 ) {
		  grammar_state_.back().mode = GRAMMAR_MODE_EXPECT_FUNCTION_ARG_NAME;
		  arg_type_found = true;
		}
	      }

	      if ( !arg_type_found ) {
		grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
	      }
	    }
	  }
	  break;

	case GRAMMAR_MODE_EXPECT_FUNCTION_ARG_NAME:
	  {
	    if ( last_token.id == TOKEN_ID_TYPE_NAME ) {

	      if ( is_keyword( last_token.text ) ) {
		grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
		break;
	      }

	      auto iter = symbol_table_.back().find( last_token.text );
	      if ( iter != symbol_table_.back().end() ) {
		std::cout << "ERROR: symbol already defined\n";
		grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
		break;
	      }
	      current_new_var_idx_.back() = new_variable_index_.back();
	      new_variable_index_.back() += 8U; // size of double
	      
	      symbol_table_data_type new_variable;
	      new_variable.sfb_offset = current_new_var_idx_.back();
	      new_variable.type       = SYMBOL_TYPE_VARIABLE;
	      // TODO. more efficient insert, using find_lower_bound
	      symbol_table_.back().insert( std::make_pair( last_token.text, new_variable ) );

	      ++(current_fn_iter_->second.fn_nargs);
	      grammar_state_.back().mode = GRAMMAR_MODE_FUNCTION_ARG_END;
	    }
	    else {
	      grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
	    }
	  }
	  break;

	case GRAMMAR_MODE_FUNCTION_ARG_END:
	  if ( last_token.id == TOKEN_ID_TYPE_COMMA ) {
	    grammar_state_.back().mode = GRAMMAR_MODE_EXPECT_FUNCTION_ARG_TYPE;
	  }
	  else if ( last_token.id == TOKEN_ID_TYPE_RPARENS ) {
	    grammar_state_.back().mode = GRAMMAR_MODE_EXPECT_FUNCTION_BODY_START;
	  }
	  else {
	    grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
	  }
	  break;

	case GRAMMAR_MODE_EXPECT_FUNCTION_BODY_START:
	  {
	    
	    // In this mode, the symbol table that constitutes the arguments
	    // must be adjusted for the new stack frame base
	    //
	    for ( auto iter = symbol_table_.back().begin()
		    ; iter != symbol_table_.back().end()
		    ; ++iter ) {
	      iter->second.sfb_offset -= (16 + (current_fn_iter_->second.fn_nargs) * 8);
	    }
	  
	    if ( last_token.id == TOKEN_ID_TYPE_LCURLY_BRACE ) {
	      grammar_state_.back().mode = GRAMMAR_MODE_DEFINE_FUNCTION_BODY;
	      grammar_state_.emplace_back( grammar_state_type( GRAMMAR_MODE_STATEMENT_START, curly_braces_ ) );
	      ++curly_braces_;
	      
	      statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_DEBUG_PRINT_STACK ) );

	      // NOTE: symbol table/variable setup is in FUNCTION_NAME state
	      //
	    }
	    else {
	      grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
	    }
	    
	  }
	  break;

	case GRAMMAR_MODE_END_OF_INPUT:
	case GRAMMAR_MODE_ERROR:
	  // Do nothing
	  //
	  break;
	}
      } while ( reprocess );

      ++tokens_parsed_;
      
    }
    
  }


  // If we've parsed all tokens, clear everything
  //
  if ( tokens_.size() == tokens_parsed_ ) {
    tokens_.clear();
    tokens_parsed_ = 0U;
  }


  if ( !grammar_state_.empty() && grammar_state_.back().mode == GRAMMAR_MODE_ERROR ) {
    std::cerr << "ERROR: grammar error on characater " << c << " (" << line_no_ << "," << char_no_ << ")\n";
    return false;
  }


  // Catch errors when parser not in a valid final state
  //
  if ( lex_mode_ == LEX_MODE_END_OF_INPUT ) {
    if ( !grammar_state_.empty() && ( grammar_state_.back().mode != GRAMMAR_MODE_END_OF_INPUT ) ) {
      std::cerr << "ERROR: grammar not terminated correctly!\n";
      std::cerr << "grammar mode is " << grammar_state_.back().mode << "\n";
      return false;
    }

    // TODO. merge PARSE_MODE_START and PARSE_MODE_OPERAND_EXPECTED,
    //  but need to maintain the below check, to prevent:
    //   3 +
    //  from being accepted
    //
    if ( parse_mode_ != PARSE_MODE_START ) {
      std::cerr << "ERROR: parse not terminated correctly\n";
      return false;
    }

    if ( curly_braces_ ) {
      std::cerr << "ERROR: mismatched curly braces\n";
      return false;
    }

    if ( !lparens_.empty() ) {
      std::cerr << "ERROR: mismatched parenthesis\n";
      return false;
    }
  }


  return true;
}


void print_statements( const std::vector<eval_data_type> &statement )
{
  size_t i = 0U;
  for ( std::vector<eval_data_type>::const_iterator iter( statement.begin() )
	  ; iter != statement.end()
	  ; ++iter, ++i ) {
    if ( iter->id == EVAL_ID_TYPE_PUSHD ) {
      std::cout << i << ": pushd " << iter->value << "\n";
    }
    else if ( iter->id == EVAL_ID_TYPE_PUSHI ) {
      std::cout << i << ": pushi " << iter->ivalue << "\n";
    }
    else if ( iter->id == EVAL_ID_TYPE_OP_POP ) {
      std::cout << i << ": pop " << iter->pop_arg << "\n";
    }
    else if ( iter->id >= EVAL_ID_TYPE_OP_JNEZ &&
	      iter->id <= EVAL_ID_TYPE_OP_JMP ) {
      std::cout << i << ": " << operator_data[ iter->id ].text <<
	" " << iter->jump_arg <<
	"\n";
    }
    else if ( iter->id == EVAL_ID_TYPE_OP_COPYTOADDR ||
	      iter->id == EVAL_ID_TYPE_OP_COPYFROMADDR ) {
      std::cout << i << ": " << operator_data[ iter->id ].text <<
	" " << iter->addr_arg <<
	"\n";
    }
    else if ( iter->id == EVAL_ID_TYPE_OP_COPYTOSTACKOFFSET ||
	      iter->id == EVAL_ID_TYPE_OP_COPYFROMSTACKOFFSET ) {
      std::cout << i << ": " << operator_data[ iter->id ].text <<
	" " << iter->offset_arg <<
	"\n";
    }
    else if ( iter->id == EVAL_ID_TYPE_OP_PUSHADDR ||
	      iter->id == EVAL_ID_TYPE_OP_PUSHSTACKOFFSET ) {
      std::cout << i << ": " << operator_data[ iter->id ].text <<
	" " << iter->addr_arg <<
	"\n";
    }
    else if ( iter->id == EVAL_ID_TYPE_OP_MOVE_END_OF_STACK ) {
      std::cout << i << ": " << operator_data[ iter->id ].text <<
	" " << iter->jump_arg <<
	"\n";
    }
    else {
      std::cout << i << ": " << operator_data[ iter->id ].text << "\n";
    }
  }
}
