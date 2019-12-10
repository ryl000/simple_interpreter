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

#include "parser_type.h"


namespace {

  
  struct operator_data_type {
    int         precedence;
    const char *text;
  };


  
  // NOTE: needs to match up with eval_id_type enum
  //
  const operator_data_type operator_data[] = {
    { 0, "push double" }
    ,{ 0, "push name" }

    ,{ 10, "not" }
    ,{ 10, "negate" }
   
    ,{ 2, "comma" }

    ,{ 1, "(" }
    ,{ 1, ")" }

    ,{ 0, ";" }

    ,{ 11, "create" }

    ,{ 0,  "clear-stack" }
    ,{ 0,  "pop" }
    ,{ 0,  "jnez" }
    ,{ 0,  "jeqz" }
    ,{ 0,  "jmp" }

    ,{ 8, "add" }
    ,{ 8, "subtract" }

    ,{ 9, "divide" }
    ,{ 9, "mutliply" }
   
    ,{ 6, "eq" }
    ,{ 6, "ne" }
    ,{ 7, "ge" }
    ,{ 7, "gt" }
    ,{ 7, "le" }
    ,{ 7, "lt" }
   
    ,{ 5, "and" }
    ,{ 4, "or" }
   
    ,{ 3, "assign" }
   
  };

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
					       std::vector<eval_data_type>       &statements
					       ,std::vector<eval_data_type>       &operator_stack
					       ,std::vector<size_t>               &lparens
					       ,eval_id_type                       eval_id
					       )
{
  // std::cout << "DEBUG: saw " << eval_id << "\n";

  if ( eval_id == EVAL_ID_TYPE_OP_LPARENS ) {
      
    //std::cout << "DEBUG: pushing " << operator_stack.size() << " onto lparens stack\n";
    lparens.push_back( operator_stack.size() );
      
  }
  else {

    // For a closing parens, make sure parenthesis is balanced
    //
    if ( (eval_id == EVAL_ID_TYPE_OP_RPARENS) && lparens.empty() ) {
      return false;
    }

    // For a semi-colon, make sure no un-matched left parens exist
    else if ( (eval_id == EVAL_ID_TYPE_OP_SEMICOLON) && !lparens.empty() ) {
      return false;
    }

    // The elements popped off the operator stack will be added to the
    //  statements stack
    //
	
    while ( !operator_stack.empty() ) {

      // For a closing parens, pop elements off operator stack until the
      //  matching opening parens is found
      //
      if ( eval_id == EVAL_ID_TYPE_OP_RPARENS ) {

	if ( operator_stack.size() == lparens.back() ) {
	  break;
	}

      }
      else {

	// Otherwise, pop elements off operator stack until either the
	// current parenthesis level has been exhausted...
	//
	if ( !lparens.empty() && (operator_stack.size() <= lparens.back()) ) {
	  break;
	}

	// ...Or the topmost operator in the operator stack has a < precedence
	//
	if ( operator_data[ eval_id ].precedence >= operator_data[ operator_stack.back().id ].precedence ) {
	  break;
	}
      }
	
      //std::cout << "DEBUG: putting " << operator_data[ operator_stack.back().id ].text << " into statement stack\n";
      statements.emplace_back( operator_stack.back() );

      // If && or || is pushed into the statements stack, we need to resolve any previously-pushed
      // JNEZ/JEQZ with the correct jump arg
      //
      if ( operator_stack.back().id == EVAL_ID_TYPE_OP_AND
	   || operator_stack.back().id == EVAL_ID_TYPE_OP_OR ) {
	size_t jump_idx = operator_stack.back().jump_arg;
	// TODO. guard against invalid index?
	// TODO. guard against invalid offset calc?
	statements[ jump_idx ].jump_arg = statements.size() - jump_idx;
	//std::cout << "DEBUG: fixing up index " << short_circuit_index << " offset to " << (statements.size() - short_circuit_index) << "\n";
	statements.back().jump_arg = 0U;
      }

      operator_stack.pop_back();

    }


    // If this was a closing parens, take care of the
    //  opening parens
    //
    if ( eval_id == EVAL_ID_TYPE_OP_RPARENS ) {
      lparens.pop_back();
      if ( lparens.empty() ) {
	if ( !if_parse_state_.empty() && if_parse_state_.back().mode == IF_PARSE_MODE_IF ) {
	  if_parse_state_.back().jump_offset = statements.size();
	  if_parse_state_.back().mode        = IF_PARSE_MODE_CLAUSE;
	  statements.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_JEQZ ) );
	  // TODO. tricky... because we are in if parse mode,
	  // need to drop back into start mode
	  parse_mode_ = PARSE_MODE_START;
	}
      }
    }

    // Otherwise, if this is not a comma or semi-colon, add it to the
    //  operator stack
    //
    else if ( eval_id != EVAL_ID_TYPE_OP_COMMA && eval_id != EVAL_ID_TYPE_OP_SEMICOLON ) {
	
      //std::cout << "DEBUG: putting " << operator_data[ item_data.id ].text << " into operator stack\n";
      operator_stack.emplace_back( eval_data_type( eval_id ) );

      // If && or ||, we need to add a JEQZ/JNEZ into the statements, to handle short-circuits
      // NOTE that we are using the "jump arg" field in the && or || to
      //  store the location of the associated JEQZ/JNEZ. This is faster than
      //  searching backwards at the time the && or || is pushed into the statements stack
      //
      if ( eval_id == EVAL_ID_TYPE_OP_AND ) {
	statements.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_JEQZ ) );
	operator_stack.back().jump_arg = statements.size() - 1U;
      }
      else if ( eval_id == EVAL_ID_TYPE_OP_OR ) {
	statements.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_JNEZ ) );
	operator_stack.back().jump_arg = statements.size() - 1U;
      }
	
    }

    // for semi-colon, we need to add an explicit command to clear the
    //  evaluation stack. Might want to add size checking...
    //  evaluation stack should either be down to 1 or 0 elements...
    //
    if ( eval_id == EVAL_ID_TYPE_OP_SEMICOLON && !statements.empty() ) {
      statements.push_back( eval_data_type( EVAL_ID_TYPE_OP_CLEAR ) );
    }

  }
    
  return true;
}


bool parser_type::parse_char( char c )
{
  ++char_no_;

  // Stage 1: Tokenize
  //
  
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
	// end of input, nothing needs
	// to be done
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

    case LEX_MODE_ERROR:
      // Do nothing
      //
      break;
    }
    
  } while ( reprocess );


  if ( lex_mode_ == LEX_MODE_ERROR ) {
    std::cerr << "ERROR: lex error on character " << c << "\n";
    return false;
  }

  
  // Stage 2: Parser
  //
  if ( tokens_.size() > tokens_parsed_ ) {

    while ( tokens_parsed_ < tokens_.size() ) {

      std::cout << "DEBUG: " << parse_mode_ << "\n";
	
      const token_type &last_token = tokens_[tokens_parsed_];
      switch ( parse_mode_ ) {

      case PARSE_MODE_START:
	{
	  bool entered_in_else_check = ( !if_parse_state_.empty() && if_parse_state_.back().mode == IF_PARSE_MODE_CHECK_ELSE );
	  bool else_seen = false;
	  
	  if ( last_token.id == TOKEN_ID_TYPE_NAME ) {
	    if ( std::strcmp( "double", last_token.text.c_str() ) == 0 ) {
	      update_stacks_with_operator_( statements_, operator_stack_, lparens_, EVAL_ID_TYPE_OP_CREATE_DOUBLE );
	      parse_mode_ = PARSE_MODE_NAME_EXPECTED;
	    }
	    else if ( std::strcmp( "if", last_token.text.c_str() ) == 0 ) {
	      if_parse_state_type new_if_parse_state;
	      new_if_parse_state.curly_braces = curly_braces_;
	      std::cout << "debug: if capturing curly braces " << curly_braces_ << "\n";
	      if_parse_state_.push_back( new_if_parse_state );
	      parse_mode_ = PARSE_MODE_LPARENS_EXPECTED;
	    }
	    else if ( std::strcmp( "else", last_token.text.c_str() ) == 0 ) {
	      std::cout << "DEBUG: else seen\n";
	      else_seen = true;
	      if ( if_parse_state_.empty() ) {
		std::cout << "DEBUG: trailing else with no if?\n";
		parse_mode_ = PARSE_MODE_ERROR;
	      }
	      else if ( if_parse_state_.back().mode != IF_PARSE_MODE_CHECK_ELSE ) {
		std::cout << "DEBUG: wrong if parse mode\n";
		parse_mode_ = PARSE_MODE_ERROR;
	      }
	      else {
		// saving the index of the unconditional jump which terminates the last if-clause,
		// then pushing it into the statements stack
		//
		size_t saved_index = statements_.size();
		statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_JMP ) );

		// fixing the jeqz associated with the previous if-check, to jump to here
		//
		size_t jump_start_idx = if_parse_state_.back().jump_offset;
		statements_[ jump_start_idx ].jump_arg = statements_.size() - jump_start_idx; // TODO. note this jump_arg doesn't exist YET
		std::cout << "debug: (A) setting " << jump_start_idx << " jump arg to " << statements_[ jump_start_idx ].jump_arg << "\n";

		// and saving the index of the jump that terminates the last if-clause
		//
		if_parse_state_.back().jump_offset = saved_index;

		if_parse_state_.back().mode = IF_PARSE_MODE_CLAUSE;

		parse_mode_ = PARSE_MODE_START;
	      }
	    }
	    else {
	      statements_.emplace_back( eval_data_type( last_token.text ) );
	      parse_mode_ = PARSE_MODE_OPERATOR_EXPECTED;
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
	      update_stacks_with_operator_( statements_, operator_stack_, lparens_, EVAL_ID_TYPE_OP_NEGATE );
	    }
	    else if ( last_token.id == TOKEN_ID_TYPE_NOT ) {
	      update_stacks_with_operator_( statements_, operator_stack_, lparens_, EVAL_ID_TYPE_OP_NOT );
	    }
	    else {
	      // Nothing needs to be done for unary +
	    }
	    parse_mode_ = PARSE_MODE_OPERAND_EXPECTED;
	  }
	  else if ( last_token.id == TOKEN_ID_TYPE_LPARENS ) {
	    update_stacks_with_operator_( statements_, operator_stack_, lparens_, EVAL_ID_TYPE_OP_LPARENS );
	    parse_mode_ = PARSE_MODE_OPERAND_EXPECTED;
	  }
	  else if ( last_token.id == TOKEN_ID_TYPE_SEMICOLON ) {
	    std::cout << "debug: semicolon\n";
	    if ( !if_parse_state_.empty() ) {
	      std::cout << "debug: current curly braces is " << curly_braces_ << "\n";
	      if ( if_parse_state_.back().curly_braces == curly_braces_ ) {
		std::cout << "if/else curly brace match!\n";
		if ( if_parse_state_.back().mode == IF_PARSE_MODE_CLAUSE ) {
		  if_parse_state_.back().mode = IF_PARSE_MODE_CHECK_ELSE;
		}
		else {
		  // TODO. is this possible?
		}
	      }
	    }
	  }
	  else if ( last_token.id == TOKEN_ID_TYPE_LCURLY_BRACE ) {
	    ++curly_braces_;
	  }
	  else if ( last_token.id == TOKEN_ID_TYPE_RCURLY_BRACE ) {
	    if ( curly_braces_ == 0U ) {
	      parse_mode_ = PARSE_MODE_ERROR;
	    }
	    else {
	      --curly_braces_;

	      if ( !if_parse_state_.empty() ) {
		if ( if_parse_state_.back().curly_braces == curly_braces_ ) {
		  if ( if_parse_state_.back().mode == IF_PARSE_MODE_CLAUSE ) {
		    if_parse_state_.back().mode = IF_PARSE_MODE_CHECK_ELSE;
		  }
		  else {
		    // TODO. is this possible?
		  }
		}
	      }

	    }
	  }
	  else {
	    parse_mode_ = PARSE_MODE_ERROR;
	  }

	  if ( parse_mode_ != PARSE_MODE_ERROR ) {
	    if ( entered_in_else_check && !else_seen ) {
	      // fixing the jeqz associated with the previous if-check, to jump to here
	      //
	      size_t jump_start_idx = if_parse_state_.back().jump_offset;
	      statements_[ jump_start_idx ].jump_arg = statements_.size() - jump_start_idx - 1; // TODO. note this jump_arg doesn't exist YET. And why -1 ?
	      std::cout << "debug: (B) setting " << jump_start_idx << " jump arg to " << statements_[ jump_start_idx ].jump_arg << "\n";

	      if_parse_state_.pop_back();
	    }
	  }

	}
	break;
      
      case PARSE_MODE_OPERAND_EXPECTED:
	if ( last_token.id == TOKEN_ID_TYPE_NUMBER
	     || last_token.id == TOKEN_ID_TYPE_NAME ) {
	  if ( last_token.id == TOKEN_ID_TYPE_NUMBER ) {
	    statements_.emplace_back( std::atof( last_token.text.c_str() ) );
	  }
	  else {
	    // TODO. guard against keywords
	    statements_.emplace_back( eval_data_type( last_token.text ) );
	  }
	  parse_mode_ = PARSE_MODE_OPERATOR_EXPECTED;
	}
	else if ( last_token.id == TOKEN_ID_TYPE_PLUS ||
		  last_token.id == TOKEN_ID_TYPE_MINUS ||
		  last_token.id == TOKEN_ID_TYPE_NOT ) {
	  if ( last_token.id == TOKEN_ID_TYPE_MINUS ) {
	    update_stacks_with_operator_( statements_, operator_stack_, lparens_, EVAL_ID_TYPE_OP_NEGATE );
	  }
	  else if ( last_token.id == TOKEN_ID_TYPE_NOT ) {
	    update_stacks_with_operator_( statements_, operator_stack_, lparens_, EVAL_ID_TYPE_OP_NOT );
	  }
	  else {
	    // Nothing needs to be done for unary +
	  }
	  // stay in this parse mode
	}
	else if ( last_token.id == TOKEN_ID_TYPE_LPARENS ) {
	  update_stacks_with_operator_( statements_, operator_stack_, lparens_, EVAL_ID_TYPE_OP_LPARENS );
	  // stay in this parse mode
	}
	// TODO. allow RPARENS here? Tricky...
	//  need to allow for (), but not (-)
	//  Perhaps only allow for this in a function context?
	else {
	  parse_mode_ = PARSE_MODE_ERROR;
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
	    update_stacks_with_operator_( statements_, operator_stack_, lparens_, new_eval_id_type );
	    parse_mode_ = PARSE_MODE_OPERAND_EXPECTED;
	  }
	}
	else if ( last_token.id == TOKEN_ID_TYPE_RPARENS ) {
	  if ( !update_stacks_with_operator_( statements_, operator_stack_, lparens_, EVAL_ID_TYPE_OP_RPARENS ) ) {
	    parse_mode_ = PARSE_MODE_ERROR;
	  }
	}
	else if ( last_token.id == TOKEN_ID_TYPE_SEMICOLON ) {
	  if ( !update_stacks_with_operator_( statements_, operator_stack_, lparens_, EVAL_ID_TYPE_OP_SEMICOLON ) ) {
	    parse_mode_ = PARSE_MODE_ERROR;
	  }
	  else {
	    std::cout << "debug: semicolon\n";
	    if ( !if_parse_state_.empty() ) {
	      std::cout << "debug: current curly braces is " << curly_braces_ << "\n";
	      if ( if_parse_state_.back().curly_braces == curly_braces_ ) {
		std::cout << "if/else curly brace match!\n";
		if ( if_parse_state_.back().mode == IF_PARSE_MODE_CLAUSE ) {
		  if_parse_state_.back().mode = IF_PARSE_MODE_CHECK_ELSE;
		}
		else {
		  // TODO. is this possible?
		}
	      }
	    }
	  }
	  parse_mode_ = PARSE_MODE_START;
	}
	else {
	  parse_mode_ = PARSE_MODE_ERROR;
	}
	break;


      case PARSE_MODE_NAME_EXPECTED:
	if ( last_token.id == TOKEN_ID_TYPE_NAME ) {
	  // TODO. guard against keywords
	  statements_.emplace_back( eval_data_type( last_token.text ) );
	  parse_mode_ = PARSE_MODE_VARIABLE_DEFINITION_START;
	}
	else {
	  parse_mode_ = PARSE_MODE_ERROR;
	}
	break;


      case PARSE_MODE_VARIABLE_DEFINITION_START:
	if ( last_token.id == TOKEN_ID_TYPE_SEMICOLON ) {
	  if ( !update_stacks_with_operator_( statements_, operator_stack_, lparens_, EVAL_ID_TYPE_OP_SEMICOLON ) ) {
	    parse_mode_ = PARSE_MODE_ERROR;
	  }
	  parse_mode_ = PARSE_MODE_START;
	}
	else if ( last_token.id == TOKEN_ID_TYPE_ASSIGN ) {
	  if ( !update_stacks_with_operator_( statements_, operator_stack_, lparens_, EVAL_ID_TYPE_OP_ASSIGN ) ) {
	    parse_mode_ = PARSE_MODE_ERROR;
	  }
	  parse_mode_ = PARSE_MODE_OPERAND_EXPECTED;
	}
	else {
	  parse_mode_ = PARSE_MODE_ERROR;
	}
	break;


      case PARSE_MODE_LPARENS_EXPECTED:
	if ( last_token.id == TOKEN_ID_TYPE_LPARENS ) {
	  update_stacks_with_operator_( statements_, operator_stack_, lparens_, EVAL_ID_TYPE_OP_LPARENS );
	  parse_mode_ = PARSE_MODE_OPERAND_EXPECTED;
	}
	else {
	  parse_mode_ = PARSE_MODE_ERROR;
	}
	break;

      }

      ++tokens_parsed_;
    }
  }

  
  // If we've parsed all tokens, clear everything
  //
  if ( tokens_.size() == tokens_parsed_ ) {
    tokens_.clear();
    tokens_parsed_ = 0U;
  }

  
  if ( parse_mode_ == PARSE_MODE_ERROR ) {
    std::cerr << "ERROR: parse error on character " << c << "\n";
    return false;
  }

  
  // Catch errors when parser not in a valid final state
  //
  if ( c == '\0' ) {
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
  for ( std::vector<eval_data_type>::const_iterator iter( statement.begin() )
	  ; iter != statement.end()
	  ; ++iter ) {
    if ( iter->id == EVAL_ID_TYPE_PUSHD ) {
      std::cout << iter->value << "\n";
    }
    else if ( iter->id == EVAL_ID_TYPE_PUSHN ) {
      std::cout << iter->name << "\n";
    }
    else {
      std::cout << operator_data[ iter->id ].text << "\n";
    }
  }
}

