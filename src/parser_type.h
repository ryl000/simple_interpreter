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

#include <cstring>
#include <string>
#include <vector>

#include "eval_data_type.h"


class parser_type {
 public:
 parser_type()
   :current_token_()
    ,statements_()
    ,lparens_()
    ,operator_stack_()
    ,tokens_()
    ,grammar_state_()
    ,char_no_(0U)
    ,curly_braces_(0U)
    ,line_no_(0U)
    ,tokens_parsed_(0U)
    ,lex_mode_( LEX_MODE_START )
    ,parse_mode_( PARSE_MODE_START )
  {
    grammar_state_.emplace_back( grammar_state_type( GRAMMAR_MODE_STATEMENT_START, curly_braces_ ) );
  }

  bool parse_char( char c );

  const std::vector<eval_data_type> &statements() { return statements_; }

  
 private:

  enum grammar_mode_type {
    GRAMMAR_MODE_STATEMENT_START
    ,GRAMMAR_MODE_STATEMENT_END
    ,GRAMMAR_MODE_DEFINE_VARIABLE
    ,GRAMMAR_MODE_NEW_VARIABLE_ASSIGNMENT
    ,GRAMMAR_MODE_CHECK_FOR_ASSIGN
    ,GRAMMAR_MODE_IF_CLAUSE
    ,GRAMMAR_MODE_IF_EXPRESSION
    ,GRAMMAR_MODE_IF_STATEMENT
    ,GRAMMAR_MODE_ELSE_CHECK
    ,GRAMMAR_MODE_ELSE_CLAUSE
    ,GRAMMAR_MODE_STATEMENT
    ,GRAMMAR_MODE_ERROR
  };


  enum lex_mode_type {
    LEX_MODE_ERROR
  
    ,LEX_MODE_START

    ,LEX_MODE_COMMENT
  
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
    TOKEN_ID_TYPE_NUMBER = 0
    ,TOKEN_ID_TYPE_NAME

    ,TOKEN_ID_TYPE_NOT

    ,TOKEN_ID_TYPE_COMMA

    ,TOKEN_ID_TYPE_LPARENS
    ,TOKEN_ID_TYPE_RPARENS

    ,TOKEN_ID_TYPE_SEMICOLON

    ,TOKEN_ID_TYPE_LCURLY_BRACE
    ,TOKEN_ID_TYPE_RCURLY_BRACE

    // NOTE: from this point down,
    //  it is assumed these enums match up
    //  with eval_id_type.
    //
    ,TOKEN_ID_FIRST_DIRECT_TOKEN_TO_CMD
    
    ,TOKEN_ID_TYPE_PLUS = TOKEN_ID_FIRST_DIRECT_TOKEN_TO_CMD
    ,TOKEN_ID_TYPE_MINUS

    ,TOKEN_ID_TYPE_DIVIDE
    ,TOKEN_ID_TYPE_MULTIPLY
  
    ,TOKEN_ID_TYPE_EQ
    ,TOKEN_ID_TYPE_NEQ
    ,TOKEN_ID_TYPE_GE
    ,TOKEN_ID_TYPE_GT
    ,TOKEN_ID_TYPE_LE
    ,TOKEN_ID_TYPE_LT

    ,TOKEN_ID_TYPE_AND
    ,TOKEN_ID_TYPE_OR

    ,TOKEN_ID_TYPE_ASSIGN

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


  struct token_type {
    explicit token_type( token_id_type in_id )
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
    ,PARSE_MODE_NAME_EXPECTED
    ,PARSE_MODE_VARIABLE_DEFINITION_START
  };


  bool anchor_jump_here_( size_t idx );

  bool statement_parser_( const token_type &last_token );

  bool statement_parser_finalize_();
  
  bool token_id_to_eval_id_(
			    token_id_type token_id
			    ,eval_id_type *eval_id
			    );

  
  bool update_stacks_with_operator_(
				    eval_id_type                       eval_id
				   );

  std::string                               current_token_;
  std::vector<eval_data_type>               statements_;
  std::vector<size_t>                       lparens_; // TODO. convert this to a deque?
  std::vector<eval_data_type>               operator_stack_; // TODO. convert this to a deque?
  std::vector<token_type>                   tokens_;
  std::vector<grammar_state_type>           grammar_state_;

  size_t                                    char_no_;
  size_t                                    curly_braces_;
  size_t                                    line_no_;
  size_t                                    tokens_parsed_;

  lex_mode_type                             lex_mode_;
  parse_mode_type                           parse_mode_;
};


void print_statements( const std::vector<eval_data_type> &statement );

