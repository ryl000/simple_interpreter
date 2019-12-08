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
#include <string>
#include <vector>

#include "evaluate.h"
#include "parser_type.h"


// test driver
//
int main( int argc, char* argv[] )
{
  if ( argc < 2 ) {
    std::cerr << "ERROR: missing argument\n";
    return 1;
  }
  
  size_t i = 0U;
  size_t in_length = std::strlen( argv[1] );
  bool process_ok = false;

  parser_type parser;
  
  while ( i < in_length && (process_ok = parser.parse_char( argv[1][i] )) ) {
    ++i;
  }

  if ( process_ok ) {
    // TODO. create an explicit "finalize" call
    //  (which underneath the covers senda a NULL char to process())
    //
    process_ok = parser.parse_char( '\0' );
  }

  if ( process_ok ) {

    std::map<std::string,double> variables;

    // print out expressions to be evaluated
    //
    for ( std::vector<std::vector<eval_data_type>>::const_iterator iter( parser.statements().begin() )
	    ; iter != parser.statements().end()
	    ; ++iter ) {

      //print_statements( *iter );
      
      if ( !evaluate( *iter, variables ) ) {
	std::cerr << "ERROR: evaluation error\n";
      }
    }
  }


  return 0;
}
