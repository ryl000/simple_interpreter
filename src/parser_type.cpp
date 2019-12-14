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
    ,{ 0,  "jceqz" }
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


  enum grammar_mode_type {
    GRAMMAR_MODE_STATEMENT_START
    ,GRAMMAR_MODE_STATEMENT_END
    ,GRAMMAR_MODE_IF_CLAUSE
    ,GRAMMAR_MODE_IF_EXPRESSION
    ,GRAMMAR_MODE_IF_STATEMENT
    ,GRAMMAR_MODE_ELSE_CHECK
    ,GRAMMAR_MODE_ELSE_CLAUSE
    ,GRAMMAR_MODE_STATEMENT
    ,GRAMMAR_MODE_ERROR
  };


  struct grammar_state_type {
    size_t            block_depth;
    grammar_mode_type mode;
    size_t            jump_offset;

    grammar_state_type( grammar_mode_type in_mode, size_t in_block_depth )
      :block_depth( in_block_depth )
      ,mode( in_mode )
      ,jump_offset( 0U ) 
    {}
  };

  std::vector<grammar_state_type> grammar_state_;
  
}


bool parser_type::anchor_jump_here_( size_t jump_idx )
{
  // TODO. add checks?
  // TODO. guard against invalid index?
  // TODO. guard against invalid offset calc?
  statements_[ jump_idx ].jump_arg = statements_.size() - jump_idx;
  return true;
}


bool parser_type::statement_parser_( const token_type &last_token )
{
  switch ( parse_mode_ ) {

  case PARSE_MODE_START:
    {
      if ( last_token.id == TOKEN_ID_TYPE_NAME ) {
	if ( std::strcmp( "double", last_token.text.c_str() ) == 0 ) {
	  update_stacks_with_operator_( EVAL_ID_TYPE_OP_CREATE_DOUBLE );
	  parse_mode_ = PARSE_MODE_NAME_EXPECTED;
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
	  update_stacks_with_operator_( EVAL_ID_TYPE_OP_NEGATE );
	}
	else if ( last_token.id == TOKEN_ID_TYPE_NOT ) {
	  update_stacks_with_operator_( EVAL_ID_TYPE_OP_NOT );
	}
	else {
	  // Nothing needs to be done for unary +
	}
	parse_mode_ = PARSE_MODE_OPERAND_EXPECTED;
      }
      else if ( last_token.id == TOKEN_ID_TYPE_LPARENS ) {
	update_stacks_with_operator_( EVAL_ID_TYPE_OP_LPARENS );
	parse_mode_ = PARSE_MODE_OPERAND_EXPECTED;
      }
      else {
	parse_mode_ = PARSE_MODE_ERROR;
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
	update_stacks_with_operator_( EVAL_ID_TYPE_OP_NEGATE );
      }
      else if ( last_token.id == TOKEN_ID_TYPE_NOT ) {
	update_stacks_with_operator_( EVAL_ID_TYPE_OP_NOT );
      }
      else {
	// Nothing needs to be done for unary +
      }
      // stay in this parse mode
    }
    else if ( last_token.id == TOKEN_ID_TYPE_LPARENS ) {
      update_stacks_with_operator_( EVAL_ID_TYPE_OP_LPARENS );
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
	update_stacks_with_operator_( new_eval_id_type );
	parse_mode_ = PARSE_MODE_OPERAND_EXPECTED;
      }
    }
    else if ( last_token.id == TOKEN_ID_TYPE_RPARENS ) {
      if ( !update_stacks_with_operator_( EVAL_ID_TYPE_OP_RPARENS ) ) {
	parse_mode_ = PARSE_MODE_ERROR;
      }
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
    if ( last_token.id == TOKEN_ID_TYPE_ASSIGN ) {
      if ( !update_stacks_with_operator_( EVAL_ID_TYPE_OP_ASSIGN ) ) {
	parse_mode_ = PARSE_MODE_ERROR;
      }
      parse_mode_ = PARSE_MODE_OPERAND_EXPECTED;
    }
    else {
      parse_mode_ = PARSE_MODE_ERROR;
    }
    break;

  }

  if ( parse_mode_ == PARSE_MODE_ERROR ) {
    return false;
  }
  else {
    return true;
  }
}


bool parser_type::statement_parser_finalize_()
{
  if ( !update_stacks_with_operator_( EVAL_ID_TYPE_OP_FINALIZE ) ) {
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
					       eval_id_type                       eval_id
					       )
{
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
	
      statements_.emplace_back( operator_stack_.back() );

      // If && or || is pushed into the statements stack, we need to resolve any previously-pushed
      // JNEZ/JEQZ with the correct jump arg
      //
      if ( operator_stack_.back().id == EVAL_ID_TYPE_OP_AND
	   || operator_stack_.back().id == EVAL_ID_TYPE_OP_OR ) {
	// TODO. check return value?
	anchor_jump_here_( operator_stack_.back().jump_arg );
	statements_.back().jump_arg = 0U;
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
	
      operator_stack_.emplace_back( eval_data_type( eval_id ) );

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

      if ( grammar_state_.empty() ) {
	grammar_state_.emplace_back( grammar_state_type( GRAMMAR_MODE_STATEMENT_START, curly_braces_ ) );
      }

      const token_type &last_token = tokens_[tokens_parsed_];
      
      do {
	switch ( grammar_state_.back().mode ) {
	case GRAMMAR_MODE_STATEMENT_START:
	  if ( last_token.id == TOKEN_ID_TYPE_NAME ) {
	    if ( std::strcmp( "if", last_token.text.c_str() ) == 0 ) {
	      grammar_state_.back().mode = GRAMMAR_MODE_IF_STATEMENT;
	    }
	    else {
	      grammar_state_.back().mode = GRAMMAR_MODE_STATEMENT;
	      reprocess                  = true;
	    }
	  }
	  else if ( last_token.id == TOKEN_ID_TYPE_LCURLY_BRACE ) {
	    // do we need to push state?
	    ++curly_braces_;
	  }
	  else if ( last_token.id == TOKEN_ID_TYPE_RCURLY_BRACE ) {
	    // do we need to pop state?
	    if ( curly_braces_ ) {
	      --curly_braces_;
	      grammar_state_.back().mode = GRAMMAR_MODE_STATEMENT_END;
	      reprocess                  = true;
	    }
	    else {
	      grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
	    }
	  }
	  else {
	    grammar_state_.back().mode = GRAMMAR_MODE_STATEMENT;
	    reprocess                  = true;
	  }
	  break;
      
	case GRAMMAR_MODE_IF_STATEMENT:
	  if ( last_token.id == TOKEN_ID_TYPE_LPARENS ) {
	    grammar_state_.back().mode = GRAMMAR_MODE_IF_EXPRESSION;
	  }
	  else {
	    grammar_state_.back().mode = GRAMMAR_MODE_ERROR;
	  }
	  break;

	case GRAMMAR_MODE_IF_EXPRESSION:
	  if ( last_token.id == TOKEN_ID_TYPE_RPARENS && lparens_.empty() ) {
	    if ( !statement_parser_finalize_() ) {
	      std::cerr << "ERROR(1): parse error on character " << c << "\n";
	      return false;
	    }

	    // TODO. Is there an easy way to unify JCEQZ and JEQZ while maintaining
	    // our stack-based evaluation?
	    //
	    grammar_state_.back().jump_offset = statements_.size();
	    statements_.emplace_back( eval_data_type( EVAL_ID_TYPE_OP_JCEQZ ) );
	    grammar_state_.back().mode = GRAMMAR_MODE_IF_CLAUSE;
	    grammar_state_.emplace_back( grammar_state_type( GRAMMAR_MODE_STATEMENT_START, curly_braces_ ) );
	  }
	  else {
	    if ( !statement_parser_( last_token ) ) {
	      std::cerr << "ERROR(2): parse error on character " << c << "\n";
	      return false;
	    }
	  }
	  break;
	  
	case GRAMMAR_MODE_STATEMENT:
	  reprocess = false;
	  if ( last_token.id == TOKEN_ID_TYPE_SEMICOLON ) {
	    if ( !statement_parser_finalize_() ) {
	      std::cerr << "ERROR(3): parse error on character " << c << "\n";
	      return false;
	    }

	    // for semi-colon, we need to add an explicit command to clear the
	    //  evaluation stack. Might want to add size checking...
	    //  evaluation stack should either be down to 1 or 0 elements...
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
	    reprocess = false;
	    bool mode_set = false;
	    if ( (grammar_state_.size() > 1) ) {
	      if ( ((grammar_state_.rbegin() + 1U)->mode == GRAMMAR_MODE_IF_CLAUSE)
		   &&
		   ((grammar_state_.rbegin() )->block_depth == curly_braces_) ) {
		grammar_state_.pop_back();
		grammar_state_.back().mode = GRAMMAR_MODE_ELSE_CHECK;
		mode_set = true;

		// NOTE: Need to defer jump offset fix until after we know if there
		// is an "else" following or not...
	      }
	      else {

		// "unwind" if/else as applicable
		//
		while ( grammar_state_.size() > 1
			&&
			((grammar_state_.rbegin() + 1U)->mode == GRAMMAR_MODE_ELSE_CLAUSE)
			&&
			((grammar_state_.rbegin() )->block_depth == curly_braces_) ) {
		  grammar_state_.pop_back();

		  // TODO. check return value?
		  anchor_jump_here_( grammar_state_.back().jump_offset );
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
	    
	    grammar_state_.back().mode = GRAMMAR_MODE_ELSE_CLAUSE;
	    grammar_state_.emplace_back( grammar_state_type( GRAMMAR_MODE_STATEMENT_START, curly_braces_ ) );
	  }
	  else {
	    // TODO. check return value?
	    anchor_jump_here_( grammar_state_.back().jump_offset );
	    
	    grammar_state_.back().mode = GRAMMAR_MODE_STATEMENT_START;
	    reprocess                  = true;
	  }
	  break;
	  
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
    std::cerr << "ERROR: grammar error on characater " << c << "\n";
    return false;
  }


  // Catch errors when parser not in a valid final state
  //
  if ( c == '\0' ) {
    if ( !grammar_state_.empty() ) {
      if ( grammar_state_.back().mode == GRAMMAR_MODE_ELSE_CHECK ) {
	// TODO. check return value?
	anchor_jump_here_( grammar_state_.back().jump_offset );
      }
      else if ( grammar_state_.back().mode != GRAMMAR_MODE_STATEMENT_START ) {
	std::cerr << "ERROR: grammar not terminated correctly!\n";
	std::cerr << "grammar mode is " << grammar_state_.back().mode << "\n";
	return false;
      }
    }
    
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
      std::cout << "pushd " << iter->value << "\n";
    }
    else if ( iter->id == EVAL_ID_TYPE_PUSHN ) {
      std::cout << "pushn " << iter->name << "\n";
    }
    else if ( iter->id >= EVAL_ID_TYPE_OP_JNEZ &&
	      iter->id <= EVAL_ID_TYPE_OP_JMP ) {
      std::cout << operator_data[ iter->id ].text <<
	" " << iter->jump_arg <<
	"\n";
    }
    else {
      std::cout << operator_data[ iter->id ].text << "\n";
    }
  }
}

