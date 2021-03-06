 /*
 * Copyright 2019-2020 Ray Li
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

#include "evaluate.h"


namespace {

  enum operand_type {
     OPERAND_TYPE_DOUBLE
    ,OPERAND_TYPE_INT32
    ,OPERAND_TYPE_SIZET
  };

  struct operand_data_type {
    explicit operand_data_type( double in_value )
      :value{ in_value }
      ,ivalue{}
      ,addr{}
      ,type{ OPERAND_TYPE_DOUBLE }
    {}

    explicit operand_data_type( int32_t in_ivalue )
      :value{}
      ,ivalue{ in_ivalue }
      ,addr{}
      ,type{ OPERAND_TYPE_INT32 }
    {}

    explicit operand_data_type( size_t in_addr )
      :value{}
      ,ivalue{}
      ,addr{ in_addr }
      ,type{ OPERAND_TYPE_SIZET }
    {}

    void set_value( double in_value )
    {
      value = in_value;
      type  = OPERAND_TYPE_DOUBLE;
    }

    double       value;
    int          ivalue;
    size_t       addr;
    operand_type type;
  };

}


bool evaluate(
              const std::vector<instruction_type> &instructions
             ,std::vector<char>                   &data
             )
{
  // instructions is the sequence of operands to execute
  // data is the "data stack" (d-stack)

  // this is the evaluation stack, which holds the "working" state of
  //  any computations
  std::vector<std::vector<operand_data_type>> evaluation_stack{ std::vector<operand_data_type> };
  size_t                                      stack_frame_base{};

  size_t instr_index = 0U;
  while ( instr_index < instructions.size() ) {

    bool    jump_absolute  = false;
    // TODO. make this int16_t for 32-bit?
    int32_t iter_increment = 1;

    std::vector<instruction_type>::const_iterator iter = instructions.begin() + instr_index;

    switch ( iter->id ) {
    case INSTRUCTION_ID_TYPE_PUSHDOUBLE:
      // PUSH-DOUBLE <double>
      //  (reqd min size of e-stack, e-stack # of elems popped, e-stack # of elems pushed)
      //  0, -0, +1
      evaluation_stack.back().push_back( operand_data_type( iter->arg.d ) );
      break;

    case INSTRUCTION_ID_TYPE_PUSHINT32:
      // PUSH-INT32 <int32>
      //  0, -0, +1
      evaluation_stack.back().push_back( operand_data_type( iter->arg.i32 ) );
      break;

    case INSTRUCTION_ID_TYPE_PUSHSIZET:
      // PUSH-SIZET <sizet>
      //  0, -0, +1
      evaluation_stack.back().push_back( operand_data_type( iter->arg.sz ) );
      break;

    case INSTRUCTION_ID_TYPE_NOT:
      // OP-NOT
      //  1, -1, +1 
      {
        if ( evaluation_stack.back().empty() ) {
          return false;
        }

        // TODO. type-aware not
        double value = evaluation_stack.back().back().value;

        evaluation_stack.back().back().set_value( (value == 0.0) ? 1.0 : 0.0 );
      }
      break;

    case INSTRUCTION_ID_TYPE_NEGATE:
      // OP-NEGATE
      //  1, -1, +1
      {
        if ( evaluation_stack.back().empty() ) {
          return false;
        }

        // TODO. type-aware negate
        double value = evaluation_stack.back().back().value;

        evaluation_stack.back().back().set_value( -1.0 * value );
      }
      break;

    case INSTRUCTION_ID_TYPE_ADD:
      // OP-ADD
      //  2, -2, +1
      {
        if ( evaluation_stack.back().size() < 2 ) {
          return false;
        }

        // TODO. type-aware add
        double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
        double value2 = (evaluation_stack.back().rbegin()     )->value;

        double result = value1 + value2;
        evaluation_stack.back().pop_back();
        evaluation_stack.back().back().set_value( result );
      }
      break;

    case INSTRUCTION_ID_TYPE_SUBTRACT:
      // OP-SUB
      //  2, -2, +1
      {
        if ( evaluation_stack.back().size() < 2 ) {
          return false;
        }

        // TODO. type-aware subtract
        double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
        double value2 = (evaluation_stack.back().rbegin()     )->value;

        double result = value1 - value2;
        evaluation_stack.back().pop_back();
        evaluation_stack.back().back().set_value( result );

      }
      break;

    case INSTRUCTION_ID_TYPE_DIVIDE:
      // OP-DIV
      //  2, -2, +1
      {
        if ( evaluation_stack.back().size() < 2 ) {
          return false;
        }

        // TODO. type-aware divide
        double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
        double value2 = (evaluation_stack.back().rbegin()     )->value;

        if ( value2 == 0.0 ) {
          return false;
        }

        double result = value1 / value2;
        evaluation_stack.back().pop_back();
        evaluation_stack.back().back().set_value( result );
      }
      break;

    case INSTRUCTION_ID_TYPE_MULTIPLY:
      // OP-MULT
      //  2, -2, +1
      {
        if ( evaluation_stack.back().size() < 2 ) {
          return false;
        }

        // TODO. type-aware multiply
        double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
        double value2 = (evaluation_stack.back().rbegin()     )->value;

        double result = value1 * value2;
        evaluation_stack.back().pop_back();
        evaluation_stack.back().back().set_value( result );
      }
      break;

    case INSTRUCTION_ID_TYPE_EQ:
      // OP-EQ
      //  2, -2, +1
      {
        if ( evaluation_stack.back().size() < 2 ) {
          return false;
        }

        // TODO. type-aware equality check
        double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
        double value2 = (evaluation_stack.back().rbegin()     )->value;

        bool result = ( value1 == value2 );
        evaluation_stack.back().pop_back();
        evaluation_stack.back().back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case INSTRUCTION_ID_TYPE_NEQ:
      // OP-NEQ
      //  2, -2, +1
      {
        if ( evaluation_stack.back().size() < 2 ) {
          return false;
        }

        // TODO. type-aware !equality check
        double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
        double value2 = (evaluation_stack.back().rbegin()     )->value;

        bool result = ( value1 != value2 );
        evaluation_stack.back().pop_back();
        evaluation_stack.back().back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case INSTRUCTION_ID_TYPE_GE:
      // OP-GE
      //  2, -2, +1
      {
        if ( evaluation_stack.back().size() < 2 ) {
          return false;
        }

        // TODO. type-aware >= check
        double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
        double value2 = (evaluation_stack.back().rbegin()     )->value;

        bool result = ( value1 >= value2 );
        evaluation_stack.back().pop_back();
        evaluation_stack.back().back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case INSTRUCTION_ID_TYPE_GT:
      // OP-GT
      //  2, -2, +1
      {
        if ( evaluation_stack.back().size() < 2 ) {
          return false;
        }

        // TODO. type-aware > check
        double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
        double value2 = (evaluation_stack.back().rbegin()     )->value;

        bool result = ( value1 > value2 );
        evaluation_stack.back().pop_back();
        evaluation_stack.back().back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case INSTRUCTION_ID_TYPE_LE:
      // OP-LE
      //  2, -2, +1
      {
        if ( evaluation_stack.back().size() < 2 ) {
          return false;
        }

        // TODO. type-aware <= check
        double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
        double value2 = (evaluation_stack.back().rbegin()     )->value;

        bool result = ( value1 <= value2 );
        evaluation_stack.back().pop_back();
        evaluation_stack.back().back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case INSTRUCTION_ID_TYPE_LT:
      // OP-LT
      //  2, -2, +1
      {
        if ( evaluation_stack.back().size() < 2 ) {
          return false;
        }

        // TODO. type-aware < check
        double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
        double value2 = (evaluation_stack.back().rbegin()     )->value;

        bool result = ( value1 < value2 );
        evaluation_stack.back().pop_back();
        evaluation_stack.back().back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case INSTRUCTION_ID_TYPE_AND:
      // OP-AND
      //  2, -2, +1
      {
        if ( evaluation_stack.back().size() < 2 ) {
          return false;
        }

        // TODO. type-aware && check
        double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
        bool result = ( value1 != 0.0 );

        if ( result ) {
          double value2 = (evaluation_stack.back().rbegin()     )->value;
          result &= (value2 != 0.0);
        }

        evaluation_stack.back().pop_back();
        evaluation_stack.back().back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case INSTRUCTION_ID_TYPE_OR:
      // OP-OR
      //  2, -2, +1
      {
        if ( evaluation_stack.back().size() < 2 ) {
          return false;
        }

        // TODO. type-aware || check
        double value1 = (evaluation_stack.back().rbegin() + 1U)->value;
        bool result = ( value1 != 0.0 );

        if ( !result ) {
          double value2 = (evaluation_stack.back().rbegin()     )->value;
          result |= (value2 != 0.0);
        }

        evaluation_stack.back().pop_back();
        evaluation_stack.back().back().set_value( result ? 1.0 : 0.0 );
      }
      break;

    case INSTRUCTION_ID_TYPE_ASSIGN:
      // OP-ASSIGN
      //  3, -3, +1
      {
        if ( evaluation_stack.back().size() < 3 ) {
          return false;
        }


        double new_value = (evaluation_stack.back().rbegin())->value; // TODO. varible type
        char *src = reinterpret_cast<char*>( &new_value );

        int is_abs = (evaluation_stack.back().rbegin() + 1U)->ivalue;

        if ( is_abs ) {
          size_t dst_idx = (evaluation_stack.back().rbegin() + 2U)->addr;
          std::copy( src, src+8U, &(data[dst_idx]) ); // TODO. variable type
        }
        else {
          int32_t dst_offset = (evaluation_stack.back().rbegin() + 2U)->ivalue;
          std::copy( src, src+8U, &(data[dst_offset + stack_frame_base]) ); // TODO. variable type
        }

        evaluation_stack.back().pop_back();
        evaluation_stack.back().pop_back();
        evaluation_stack.back().back().value = new_value;
      }
      break;

    case INSTRUCTION_ID_TYPE_CLEAR:
      // OP-CLEAR
      //  clears estack
      {
        if ( !evaluation_stack.back().empty() ) {
          double value = (evaluation_stack.back().rbegin())->value;
          std::cout << " => " << value << "\n";
          if ( evaluation_stack.back().size() > 1 ) {
            std::cout << "WARNING: final stack size is " << evaluation_stack.back().size() << "\n";
          }
        }
        evaluation_stack.back().clear();
      }
      break;

    case INSTRUCTION_ID_TYPE_POP:
      // OP-POP <narg>
      //  narg, -narg, +0
      {
        std::cout << "debug: pop\n";
        if ( evaluation_stack.back().size() < iter->arg.sz ) {
          return false;
        }

        for ( size_t i=0; i<iter->arg.sz; ++i ) {
          evaluation_stack.back().pop_back();
        }
      }
      break;

    case INSTRUCTION_ID_TYPE_JNEZ:
      // OP-JNEZ <offset>
      //  1, -0, +0
      {
        if ( evaluation_stack.back().empty() ) {
          return false;
        }

        // TODO. type-aware JNEZ
        double value = (evaluation_stack.back().rbegin())->value;

        if ( value != 0.0 ) {
          iter_increment = iter->arg.i32;
        }
      }
      break;

    case INSTRUCTION_ID_TYPE_JEQZ:
      // OP-JEQZ <offset>
      //  1, -0, +0
      {
        if ( evaluation_stack.back().empty() ) {
          return false;
        }

        // TODO. type-aware JEQZ
        double value = (evaluation_stack.back().rbegin())->value;
        if ( value == 0.0 ) {
          iter_increment = iter->arg.i32;
        }
      }
      break;

    case INSTRUCTION_ID_TYPE_JCEQZ:
      // OP-JCEQZ <offset>
      //  1, -1, +0
      {
        if ( evaluation_stack.back().empty() ) {
          return false;
        }

        // TODO. type-aware JCEQZ
        double value = (evaluation_stack.back().rbegin())->value;

        if ( value == 0.0 ) {
          iter_increment = iter->arg.i32;
        }
        evaluation_stack.back().pop_back();
      }
      break;

    case INSTRUCTION_ID_TYPE_JMP:
      // OP-JMP <offset>
      //  0, -0, +0
      {
        iter_increment = iter->arg.i32;
      }
      break;

    case INSTRUCTION_ID_TYPE_JMPA:
      // OP-JMPA <addr>
      //  0, -0, +0
      {
        instr_index   = iter->arg.sz;
        jump_absolute = true;
      }
      break;

    case INSTRUCTION_ID_TYPE_COPYFROMADDR:
      // OP-COPY-FROM-ADDR <addr>
      //  0, -0, +1
      {
        double new_value;
        char *src = &(data[iter->arg.sz]);
        char *dst = reinterpret_cast<char*>( &new_value );
        std::copy( src, src+8U, dst ); // TODO. variable-size copy

        evaluation_stack.back().emplace_back( operand_data_type( new_value ) );
      }
      break;

    case INSTRUCTION_ID_TYPE_COPYFROMSTACKOFFSET:
      // OP-COPY-FROM-OFFSET <offset>
      //  0, -0, +1
      {
        double new_value;
        char *src = &(data[iter->arg.i32 + stack_frame_base]);
        char *dst = reinterpret_cast<char*>( &new_value );
        std::copy( src, src+8U, dst ); // TODO. variable-size copy

        evaluation_stack.back().emplace_back( operand_data_type( new_value ) );
      }
      break;

    case INSTRUCTION_ID_TYPE_COPYTOADDR:
      // OP-COPY-TO-ADDR <addr>
      //  1, -0, +0
      {
        if ( evaluation_stack.back().empty() ) {
          return false;
        }

        double value = (evaluation_stack.back().rbegin())->value;

        char *src = reinterpret_cast<char*>( &value );
        std::copy( src, src+8U, &(data[iter->arg.sz]) ); // TODO. variable-size copy
      }
      break;

    case INSTRUCTION_ID_TYPE_COPYTOSTACKOFFSET:
      // OP-COPY-TO-STACK-OFFSET <offset>
      //  1, -0, +0
      {
        if ( evaluation_stack.back().empty() ) {
          return false;
        }

        double value = (evaluation_stack.back().rbegin())->value;

        char *src = reinterpret_cast<char*>( &value );
        std::copy( src, src+8U, &(data[iter->arg.i32 + stack_frame_base]) ); // TODO. variable-size copy
      }
      break;

    case INSTRUCTION_ID_TYPE_MOVE_END_OF_STACK:
      // OP-MOVE-END-OF-STACK
      //  0, -0, +0
      {
        size_t new_size = data.size() + iter->arg.i32;
        data.resize( new_size );
      }
      break;

    case INSTRUCTION_ID_TYPE_CALL:
      {
        std::cout << "=====CALL=====\n";
        std::cout << "current stack frame base is " << stack_frame_base << "\n";
        std::cout << "data stack size is " << data.size() << "\n";

        // push address of next instruction onto stack
        data.resize( data.size() + 8U );
        *(reinterpret_cast<size_t*>( &(data[data.size()-8U]) )) = instr_index + 1U;
        std::cout << "pushing return addr : " << *(reinterpret_cast<size_t*>( &(data[data.size()-8U]) )) << "\n";

        // push current stack frame base onto stack
        data.resize( data.size() + 8U );
        *(reinterpret_cast<size_t*>( &(data[data.size()-8U]) )) = stack_frame_base;
        std::cout << "pushing current stack frame base : " << *(reinterpret_cast<size_t*>( &(data[data.size()-8U]) )) << "\n";

        // reset stack frame base to end-of-stack, in preparation for
        // function execution
        //
        stack_frame_base = data.size();
        std::cout << "new stack frame base will be " << stack_frame_base << "\n";

        // New evaluation stack for the function
        //
        evaluation_stack.push_back( std::vector<operand_data_type>() );

        // jump to function start
        //
        instr_index   = iter->arg.sz;
        std::cout << "jumping to " << instr_index << "\n";
        jump_absolute = true;
      }
      break;

    case INSTRUCTION_ID_TYPE_RETURN:
      {
        std::cout << "=====RETURN=====\n";

        size_t old_stack_frame_base = *(reinterpret_cast<size_t*>(&(data[stack_frame_base -  8])));
        std::cout << "debug: setting stack frame base to " << old_stack_frame_base << "\n";
        size_t return_address       = *(reinterpret_cast<size_t*>(&(data[stack_frame_base - 16])));
        std::cout << "debug: jump back addr is " << return_address << "\n";
        data.resize( stack_frame_base - 16 );
        std::cout << "data stack size is now " << data.size() << "\n";

        // Remove the function's evaluation stack
        //
        evaluation_stack.pop_back();

        // Restore state before returning to caller
        //
        stack_frame_base = old_stack_frame_base;
        instr_index      = return_address;
        jump_absolute    = true;
      }
      break;

    case INSTRUCTION_ID_TYPE_DEBUG_PRINT_STACK:
      // TODO.
      std::cout << "DEBUG: stack size is " << data.size() << "\n";
      {
        for ( size_t i=0U; i<data.size(); i += 8 ) {
          std::cout << i << ": " << *(reinterpret_cast<double*>( &(data[i]) )) << ","
                    << *(reinterpret_cast<size_t*>( &(data[i]) )) << "\n";
        }
      }
      break;

    case INSTRUCTION_ID_TYPE_COMMA:
    case INSTRUCTION_ID_TYPE_FINALIZE:
    case INSTRUCTION_ID_TYPE_FN:
    case INSTRUCTION_ID_TYPE_LPARENS:
    case INSTRUCTION_ID_TYPE_RPARENS:
      // NOTE. These should never occur...
      break;
    }

    if ( !jump_absolute ) {
      instr_index += iter_increment;
    }
  }

  return true;
}
