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
    ,current_statement_()
    ,statements_()
    ,lparens_()
    ,operator_stack_()
    ,tokens_()
    ,char_no_(0U)
    ,line_no_(0U)
    ,tokens_parsed_(0U)
    ,lex_mode_( LEX_MODE_START )
    ,parse_mode_( PARSE_MODE_START )
  {}

  bool parse_char( char c );

  const std::vector<std::vector<eval_data_type>> &statements() { return statements_; }

  
 private:
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

    ,TOKEN_ID_TYPE_PLUS
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

    ,TOKEN_ID_TYPE_COMMA

    ,TOKEN_ID_TYPE_LPARENS
    ,TOKEN_ID_TYPE_RPARENS

    ,TOKEN_ID_TYPE_SEMICOLON
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


  bool token_id_to_eval_id_(
			    token_id_type token_id
			    ,eval_id_type *eval_id
			    );

  
  std::string                               current_token_;
  std::vector<eval_data_type>               current_statement_;
  std::vector<std::vector<eval_data_type>>  statements_;
  std::vector<size_t>                       lparens_; // TODO. convert this to a deque?
  std::vector<eval_data_type>               operator_stack_; // TODO. convert this to a deque?
  std::vector<token_type>                   tokens_;

  size_t                                    char_no_;
  size_t                                    line_no_;
  size_t                                    tokens_parsed_;

  lex_mode_type                             lex_mode_;
  parse_mode_type                           parse_mode_;
};
