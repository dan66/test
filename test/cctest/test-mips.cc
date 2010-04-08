// Copyright 2010 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "v8.h"
#include "execution.h"

#include "cctest.h"

using ::v8::Local;
using ::v8::String;
using ::v8::Script;

namespace i = ::v8::internal;

TEST(MIPSFunctionCalls) {
  // Disable compilation of natives.
  i::FLAG_disable_native_files = true;
  i::FLAG_full_compiler = false;

  v8::HandleScope scope;
  LocalContext env;  // from cctest.h

  const char* c_source_1 =
    "function foo() {"
    "  return 0xabcd;"
    "}"
    "foo();";
  Local<String> source_1 = ::v8::String::New(c_source_1);
  Local<Script> script_1 = ::v8::Script::Compile(source_1);
  CHECK_EQ(0xabcd, script_1->Run()->Int32Value());


  const char* c_source_2 =
    "function foo1(arg1, arg2, arg3, arg4, arg5) {"
    "  return foo2(arg1, foo2(arg3, arg4));"
    "}"
    ""
    "function foo2(arg1, arg2) {"
    "  return arg2;"
    "}"
    "foo1(1, 2, 3, 4, 5);";

  Local<String> source_2 = ::v8::String::New(c_source_2);
  Local<Script> script_2 = ::v8::Script::Compile(source_2);
  CHECK_EQ(4, script_2->Run()->Int32Value());
}

