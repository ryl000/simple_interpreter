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
#include <deque>
#include <map>
#include <string>
#include <vector>

#include "instruction_type.h"
#include "symbol_table_data_type.h"


class parser_type {
 public:
 parser_type()
   :current_token_()
    ,statements_()
    ,lparens_()
    ,operator_stack_()
    ,tokens_()
    ,grammar_state_()
    ,function_parse_state_()
    ,symbol_table_( 1U )
    ,current_new_var_idx_( 1U )
    ,new_variable_index_( 1U )
    ,current_offset_from_stack_frame_base_( 1U )
    ,char_no_(0U)
    ,curly_braces_(0U)
    ,line_no_(0U)
    ,tokens_parsed_(0U)
    ,lex_mode_( LEX_MODE_START )
    ,parse_mode_( PARSE_MODE_START )
  {
    grammar_state_.emplace_back( grammar_state_type( GRAMMAR_MODE_STATEMENT_START, curly_braces_, false ) );
    current_new_var_idx_[0] = 0U;
    new_variable_index_[0] = 0U;
    current_offset_from_stack_frame_base_[0] = 0U;
  }

  bool parse_char( char c );

  size_t data_size() { return new_variable_index_.front(); }

  const std::vector<instruction_type> &statements() { return statements_; }

  
 private:

  enum grammar_mode_type {
    GRAMMAR_MODE_STATEMENT_START
    ,GRAMMAR_MODE_STATEMENT_END
    ,GRAMMAR_MODE_DEFINE_VARIABLE
    ,GRAMMAR_MODE_NEW_VARIABLE_ASSIGNMENT
    ,GRAMMAR_MODE_CHECK_FOR_ASSIGN
    ,GRAMMAR_MODE_BRANCH_CLAUSE
    ,GRAMMAR_MODE_BRANCH_EXPRESSION
    ,GRAMMAR_MODE_BRANCH_STATEMENT
    ,GRAMMAR_MODE_ELSE_CHECK
    ,GRAMMAR_MODE_STATEMENT
    ,GRAMMAR_MODE_DEFINE_FUNCTION_START
    ,GRAMMAR_MODE_EXPECT_FUNCTION_NAME
    ,GRAMMAR_MODE_EXPECT_FUNCTION_OPEN_PARENS
    ,GRAMMAR_MODE_EXPECT_FUNCTION_ARG_TYPE
    ,GRAMMAR_MODE_EXPECT_FUNCTION_ARG_NAME
    ,GRAMMAR_MODE_FUNCTION_ARG_END
    ,GRAMMAR_MODE_EXPECT_FUNCTION_BODY_START
    ,GRAMMAR_MODE_DEFINE_FUNCTION_BODY
    ,GRAMMAR_MODE_END_OF_INPUT
    ,GRAMMAR_MODE_ERROR
  };


  enum lex_mode_type {
    LEX_MODE_ERROR
  
    ,LEX_MODE_START

    ,LEX_MODE_COMMENT
  
    ,LEX_MODE_NUMBER_START_DIGIT
    ,LEX_MODE_NUMBER_START_DECIMAL
    ,LEX_MODE_NUMBER_DECIMAL
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

    ,LEX_MODE_END_OF_INPUT
  };


  
  enum token_id_type {
    TOKEN_ID_TYPE_NUMBER = 0
    ,TOKEN_ID_TYPE_NAME

    ,TOKEN_ID_TYPE_NOT

    ,TOKEN_ID_TYPE_LPARENS
    ,TOKEN_ID_TYPE_RPARENS

    ,TOKEN_ID_TYPE_SEMICOLON

    ,TOKEN_ID_TYPE_LCURLY_BRACE
    ,TOKEN_ID_TYPE_RCURLY_BRACE

    ,TOKEN_ID_TYPE_END_OF_INPUT

    // NOTE: from this point down,
    //  it is assumed these enums match up
    //  with eval_id_type.
    //
    ,TOKEN_ID_TYPE_FIRST_DIRECT_INSTRUCTION
    
    ,TOKEN_ID_TYPE_PLUS = TOKEN_ID_TYPE_FIRST_DIRECT_INSTRUCTION
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

    ,TOKEN_ID_TYPE_COMMA

    ,TOKEN_ID_TYPE_ASSIGN

  };


  enum branching_mode_type {
    BRANCHING_MODE_IF
    ,BRANCHING_MODE_ELSE
    ,BRANCHING_MODE_WHILE
  };


  struct grammar_state_type {
    size_t               block_depth;
    grammar_mode_type    mode;
    size_t               jump_offset; // TODO. add multiple jump offsets, to allow for break statement inside loop?
    size_t               loopback_offset;
    branching_mode_type  branching_mode;
    bool                 return_mode;
    bool                 unreachable_code;

    grammar_state_type( grammar_mode_type in_mode, size_t in_block_depth, bool in_unreachable_code )
      :block_depth( in_block_depth )
      ,mode( in_mode )
      ,jump_offset( 0U )
      ,loopback_offset( 0U )
      ,branching_mode( BRANCHING_MODE_IF )
      ,return_mode( false )
      ,unreachable_code( in_unreachable_code )
    {}
  };


  struct function_parse_state_type {
    size_t return_size;
    bool   code_path_inactive;

    function_parse_state_type()
      :return_size( 8U ) /* TODO. allow int, void returns */
      ,code_path_inactive( false )
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
    ,PARSE_MODE_FN_LPARENS_EXPECTED
  };


  bool anchor_jump_here_( size_t idx );

  bool statement_parser_( const token_type &last_token );

  bool statement_parser_finalize_();
  
  bool token_id_to_instruction_id_(
                                   token_id_type token_id
                                   ,instruction_id_type *instruction_id
                                   );

  
  bool update_stacks_with_operator_(
                                    instruction_type                       eval_data
                                   );

  std::string                                                current_token_;
  std::vector<instruction_type>                              statements_;
  std::vector<size_t>                                        lparens_; // TODO. convert this to a deque?
  std::vector<instruction_type>                              operator_stack_; // TODO. convert this to a deque?
  std::vector<token_type>                                    tokens_;
  std::vector<grammar_state_type>                            grammar_state_;
  std::vector<function_parse_state_type>                     function_parse_state_;
  
  std::deque<std::map<std::string,symbol_table_data_type>>   symbol_table_;
  std::vector<size_t>                                        current_new_var_idx_;
  std::vector<size_t>                                        new_variable_index_;
  std::vector<size_t>                                        current_offset_from_stack_frame_base_;

  std::map<std::string,symbol_table_data_type>::iterator     current_fn_iter_;

  size_t                                                     char_no_;
  size_t                                                     curly_braces_;
  size_t                                                     line_no_;
  size_t                                                     tokens_parsed_;

  lex_mode_type                                              lex_mode_;
  parse_mode_type                                            parse_mode_;
};


void print_statements( const std::vector<instruction_type> &statement );
