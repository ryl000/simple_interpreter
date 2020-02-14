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

enum symbol_type {
  SYMBOL_TYPE_VARIABLE
  ,SYMBOL_TYPE_FUNCTION
};

struct symbol_table_data_type {
  symbol_table_data_type()
    :addr( 0U )
    ,fn_nargs( 0U )
    ,fn_ret_size( 0U )
    ,sfb_offset( 0 )
    ,is_abs( false )
    ,type( SYMBOL_TYPE_VARIABLE )
  {}

  size_t      addr;
  size_t      fn_nargs;
  size_t      fn_ret_size;
  int32_t     sfb_offset;
  bool        is_abs;
  symbol_type type;
};
