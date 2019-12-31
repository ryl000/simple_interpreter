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

#include <cstring>
#include <fstream>
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

  bool cmd_line_mode = false;
  // handle command-line options
  int iarg = 1;
  for ( ; iarg < argc; ++iarg ) {
    if ( argv[iarg][0] != '-' ) {
      break;
    }
    if ( std::strcmp( argv[iarg], "--" ) == 0U ) {
      break;
    }
    if ( std::strcmp( argv[iarg], "-c" ) == 0U ) {
      cmd_line_mode = true;
    }
  }

  if ( iarg >= argc ) {
    std::cerr << "ERROR: missing argument(s)\n";
    return 1;
  }
  
  bool process_ok = false;

  parser_type parser;

  if ( cmd_line_mode ) {
    size_t in_length = std::strlen( argv[iarg] );
    size_t i = 0U;
    while ( i < in_length && (process_ok = parser.parse_char( argv[iarg][i] )) ) {
      ++i;
    }
  }
  else {
    std::ifstream infile( argv[iarg] );
    if ( !infile ) {
      std::cerr << "ERROR: could not open file " << argv[iarg] << "\n";
      return 1;
    }

    char c;
    while (infile.get(c) && (process_ok = parser.parse_char(c))) {
      ;
    }
  }

  if ( process_ok ) {
    // TODO. create an explicit "finalize" call
    //  (which underneath the covers senda a NULL char to process())
    //
    process_ok = parser.parse_char( '\0' );
  }

  if ( process_ok ) {

    print_statements( parser.statements() );

    std::vector<char> data;
    if ( !evaluate( parser.statements(), data ) ) {
      std::cerr << "ERROR: evaluation error\n";
    }

  }


  return 0;
}
