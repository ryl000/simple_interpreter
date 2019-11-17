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

#include <cctype>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>


enum lex_mode_type {
  LEX_MODE_ERROR
  
  ,LEX_MODE_START
  
  ,LEX_MODE_NUMBER_START_DIGIT
  ,LEX_MODE_NUMBER_START_DECIMAL
  ,LEX_MODE_NUMBER_DECIMAL
  ,LEX_MODE_NUMBER_FRACTION
  ,LEX_MODE_NUMBER_EXPONENT
  ,LEX_MODE_NUMBER_EXPONENT_SIGN
  ,LEX_MODE_NUMBER_EXPONENT_DIGIT

  ,LEX_MODE_NAME_START

  ,LEX_MODE_EQ_CHECK

  ,LEX_MODE_GT_CHECK
  
  ,LEX_MODE_LT_CHECK

  ,LEX_MODE_NOT_CHECK

  ,LEX_MODE_AND_CHECK

  ,LEX_MODE_OR_CHECK
};


enum token_id_type {
  TOKEN_ID_TYPE_NUMBER
  ,TOKEN_ID_TYPE_NAME

  ,TOKEN_ID_TYPE_NOT
  ,TOKEN_ID_TYPE_LPARENS
  ,TOKEN_ID_TYPE_RPARENS

  ,TOKEN_ID_TYPE_PLUS
  ,TOKEN_ID_TYPE_MINUS

  ,TOKEN_ID_TYPE_DIVIDE
  ,TOKEN_ID_TYPE_MULTIPLY
  ,TOKEN_ID_TYPE_ASSIGN
  ,TOKEN_ID_TYPE_COMMA
  ,TOKEN_ID_TYPE_EQ
  ,TOKEN_ID_TYPE_GE
  ,TOKEN_ID_TYPE_GT
  ,TOKEN_ID_TYPE_LE
  ,TOKEN_ID_TYPE_LT
  ,TOKEN_ID_TYPE_NE
  ,TOKEN_ID_TYPE_AND
  ,TOKEN_ID_TYPE_OR
};


struct token_type {
  token_type( token_id_type in_id )
    :text()
    ,id( in_id )
  {}

  token_type( token_id_type in_id, const std::string& in_text )
    :text( in_text )
    ,id( in_id )
  {}
  
  std::string   text;
  token_id_type id;
};


enum parse_mode_type {
  PARSE_MODE_ERROR
  ,PARSE_MODE_START
  ,PARSE_MODE_OPERAND_EXPECTED
  ,PARSE_MODE_OPERATOR_EXPECTED
};



// Lexer/parser state
//
std::string              current_token_;
lex_mode_type            lex_mode_ = LEX_MODE_START;
parse_mode_type          parse_mode_ = PARSE_MODE_START;
size_t                   parser_parens_depth_ = 0U;
std::vector<token_type>  tokens_;
size_t                   tokens_parsed_ = 0U;


bool process( char c )
{
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
      else if ( c == '\0' ) {
	// end of input, nothing needs
	// to be done
      }
      else {
	lex_mode_ = LEX_MODE_ERROR;
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
	tokens_.emplace_back( token_type( TOKEN_ID_TYPE_NE ) );
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

      const token_type &last_token = tokens_[tokens_parsed_];
      switch ( parse_mode_ ) {
	
      case PARSE_MODE_START:
	if ( last_token.id == TOKEN_ID_TYPE_NUMBER
	     || last_token.id == TOKEN_ID_TYPE_NAME ) {
	  parse_mode_ = PARSE_MODE_OPERATOR_EXPECTED;
	}
	else if ( last_token.id == TOKEN_ID_TYPE_PLUS ||
		  last_token.id == TOKEN_ID_TYPE_MINUS ||
		  last_token.id == TOKEN_ID_TYPE_NOT ) {
	  parse_mode_ = PARSE_MODE_OPERAND_EXPECTED;
	}
	else if ( last_token.id == TOKEN_ID_TYPE_LPARENS ) {
	  parse_mode_ = PARSE_MODE_OPERAND_EXPECTED;
	  ++parser_parens_depth_;
	}
	else {
	  parse_mode_ = PARSE_MODE_ERROR;
	}
	break;
      
      case PARSE_MODE_OPERAND_EXPECTED:
	if ( last_token.id == TOKEN_ID_TYPE_NUMBER
	     || last_token.id == TOKEN_ID_TYPE_NAME ) {
	  parse_mode_ = PARSE_MODE_OPERATOR_EXPECTED;
	}
	else if ( last_token.id == TOKEN_ID_TYPE_PLUS ||
		  last_token.id == TOKEN_ID_TYPE_MINUS ||
		  last_token.id == TOKEN_ID_TYPE_NOT ) {
	  // stay in this parse mode
	}
	else if ( last_token.id == TOKEN_ID_TYPE_LPARENS ) {
	  // stay in this parse mode
	  ++parser_parens_depth_;
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
	     last_token.id == TOKEN_ID_TYPE_NE ||
	     last_token.id == TOKEN_ID_TYPE_AND ||
	     last_token.id == TOKEN_ID_TYPE_OR ) {
	  parse_mode_ = PARSE_MODE_OPERAND_EXPECTED;
	}
	else if ( last_token.id == TOKEN_ID_TYPE_RPARENS ) {
	  if ( parser_parens_depth_ == 0U ) {
	    parse_mode_ = PARSE_MODE_ERROR;
	  }
	  else {
	    // stay in this parse mode
	    --parser_parens_depth_;
	  }
	}
	else {
	  parse_mode_ = PARSE_MODE_ERROR;
	}
	break;

      }

      ++tokens_parsed_;
    }
  }

  if ( parse_mode_ == PARSE_MODE_ERROR ) {
    std::cerr << "ERROR: parse error on character " << c << "\n";
    return false;
  }

  
  // Catch errors when parser not in a valid final state
  //
  if ( c == '\0' ) {
    if ( parse_mode_ == PARSE_MODE_OPERAND_EXPECTED ) {
      std::cerr << "ERROR: parse not terminated correctly\n";
    }
    else if ( parser_parens_depth_ != 0U ) {
      std::cerr << "ERROR: mismatched parens\n";
    }
  }

  return true;
}


// test driver
//
int main( int argc, char* argv[] )
{
  size_t i = 0U;
  size_t in_length = std::strlen( argv[1] );
  bool process_ok = false;
  while ( i < in_length && (process_ok = process( argv[1][i] )) ) {
    ++i;
  }
  if ( process_ok ) {
    // TODO. create an explicit "finalize" call
    //  (which underneath the covers senda a NULL char to process())
    //
    process( '\0' );
  }

  return 0;
}
