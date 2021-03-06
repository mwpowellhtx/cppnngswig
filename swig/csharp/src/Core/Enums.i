/*
  Copyright (c) 2017 Michael W. Powell <mwpowellhtx@gmail.com> All rights reserved.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

// Include the file we want to wrap a first time.
%{
#include "cpp/src/core/enums.h"
%}

%module(directors="1") Core;

%include "typemaps.i"

namespace nng {

// TODO: TBD: will circle back for these enumerations...
%ignore duration_ms;
%ignore protocol_type;
%ignore error_code_type;

%typemap(csattributes) SocketFlag "[global::System.FlagsAttribute]"
%typemap(csbase) SocketFlag "int";

%rename("None") flag_none;
%rename("Alloc") flag_alloc;
%rename("NonBlock") flag_nonblock;

}

%include "cpp/src/core/enums.h"
