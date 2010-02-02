// Copyright 2006-2008 the V8 project authors. All rights reserved.
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
#include "bootstrapper.h"
#include "codegen-inl.h"
#include "debug.h"
#include "parser.h"
#include "register-allocator-inl.h"
#include "runtime.h"
#include "scopes.h"
#include "compiler.h"

#include "mips/simulator-mips.h"

#include "test-interface-mips.h"


namespace v8 {
namespace internal {

#define __ ACCESS_MASM(masm)


} }  // namespace v8::internal

using namespace v8::internal;

// --------------------------------------------------------------------------------
// TestMIPS1

int TestMIPS1(int argc, char* argv[]) {
  typedef unsigned char byte;

  // Entering JavaScript.
  VMState state(JS);

  // Placeholder for return value.
  Object* value = reinterpret_cast<Object*>(kZapValue);

  typedef Object* (*JSEntryFunction)(
    byte* entry,
    Object* function,
    Object* receiver,
    int argc,
    Object*** args);

  Handle<Code> code;
  TestMIPSStub stub;
  code = stub.GetCode();

  {
    // Save and restore context around invocation and block the
    // allocation of handles without explicit handle scopes.
    SaveContext save;
    NoHandleAllocation na;
    JSEntryFunction entry = FUNCTION_CAST<JSEntryFunction>(code->entry());

    // Call TestMIPSStub generated code.
    byte* entry_address= NULL;
    JSFunction* function = NULL;
    Object* receiver_pointer = NULL;
    int argc = 0;
    Object*** args =  NULL;
    value = CALL_GENERATED_CODE(entry, entry_address, function,
                                receiver_pointer, argc, args);
  }

  // Check the value returned (in v0).
  return reinterpret_cast<int>(value);
}

void TestMIPSStub::GenerateBody(MacroAssembler* masm) {
  // The stub is called as a function, and expected to preserve callee-saved
  // registers. If you don't v8 will raise an error when leaving the simulator.
  // To do so you can use the following multi_push/pop pseudo-instructions.
//  __ multi_push(kCalleeSaved | ra.bit());
//  __ multi_pop(kCalleeSaved | ra.bit());

  // ----- Save callee-saved registers on the stack
  __ multi_push(kCalleeSaved | ra.bit());

  for (int i = 0; i < 5; i++) { __ nop(); }  // visual helper when disasembling

  // ----- Test all instructions.

  // Test lui, ori, and addiu, used in the li pseudo-instruction.
  // This way we can then safely load registers with chosen values.

  __ break_(0x1);
  __ ori(t0, zero_reg, Operand(0));
  __ lui(t0, 0x1234);
  __ ori(t0, t0, Operand(0));
  __ ori(t0, t0, Operand(0x0f0f));
  __ ori(t0, t0, Operand(0xf0f0));
  __ addiu(t1, t0, Operand(1));
  __ addiu(t2, t1, Operand(-0x10));

  // Load values in temporary registers
  __ li(t0, Operand(0x1248));
  __ li(t1, Operand(0x1248ffff));
  __ li(t2, Operand(0x4));
  __ li(t3, Operand(0x8));
  __ li(t4, Operand(0xfffff));
  __ li(t5, Operand(0x8000000f));
  __ li(t6, Operand(0x7fffffff));
  __ li(t7, Operand(0xffffffff));


  ///// SPECIAL class
  __ sll(v0, t0, 4);
  __ sll(v0, t0, 28);
  __ srl(v0, t0, 4);
  __ srl(v0, t1, 4);
  __ sra(v0, t0, 4);
  __ sra(v0, t1, 4);
  __ sllv(v0, t1, t2);
  __ srlv(v0, t0, t2);
  __ srlv(v0, t1, t2);
  __ srav(v0, t0, t2);
  __ srav(v0, t1, t2);

  __ mfhi(v1);
  __ mflo(v0);

//  __ mult();
//  __ multu();
  __ div(t4, Operand(t0));
  __ div(t5, Operand(t7));
  __ divu(t4, Operand(t0));
  __ divu(t5, Operand(t7));

  __ add(v0, t0, t2);
//  __ add(v0, t0, t6);   // overflow
//  __ add(v0, t7, t1);   // overflow
  __ addu(v0, t0, t2);
  __ addu(v0, t1, t4);
  __ sub(v1, t0, Operand(t2));
//  __ sub(v1, t7, Operand(t0));   // underflow
//  __ sub(v1, t6, Operand(t7));   // underflow

  __ and_(v0, t0, Operand(t1));
  __ or_(v0, t0, Operand(t1));
  __ xor_(v0, t0, Operand(t1));
  __ nor(v0, t0, Operand(t1));

  __ slt(v0, t7, t6);
  __ slt(v0, t1, t1);
  __ sltu(v0, t7, t6);
  __ sltu(v0, t1, t1);

  // Implement and test coprocessor instructions.

  __ addi(v0, t1, 1);
  __ addiu(v0, t7, 0xb);
  __ slti(v0, t0, Operand(0xffff));
  __ slti(v0, t0, Operand(0x1234));
  __ sltiu(v0, t0, Operand(0xffff));
  __ sltiu(v0, t0, Operand(0x1234));


  // Test
  Label L, C;

  __ li(a0, Operand(100));

  __ mov(a1, a0);
  __ li(v0, Operand(0));
  __ b(&C);
  __ nop();

  __ bind(&L);
  __ add(v0, v0, Operand(a1));
  __ addiu(a1, a1, -1);

  __ bind(&C);
  __ xor_(v1, a1, Operand(0));
  __ bcond(ne, &L, v1, Operand(0));
  __ nop();

  __ break_(0x9);   // v0 should be 5050


  // Return

  for (int i = 0; i < 5; i++) { __ nop(); }  // visual helper when disasembling

  // ----- Restore callee-saved registers on the stack
  __ multi_pop(kCalleeSaved | ra.bit());

  __ Jump(ra);
  __ nop();  // NOP_ADDED
}


// --------------------------------------------------------------------------------
// TestMIPS2
// Code adapted from samples/shell.cc

void LRunShell(v8::Handle<v8::Context> context);
bool LExecuteString(v8::Handle<v8::String> source,
                   v8::Handle<v8::Value> name,
                   bool print_result,
                   bool report_exceptions);
v8::Handle<v8::String> LReadFile(const char* name);
void LReportException(v8::TryCatch* handler);

// Extracts a C string from a V8 Utf8Value.
const char* LToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}


int TestMIPS2(int argc, char* argv[]) {
  v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
  v8::HandleScope handle_scope;
  // Create a template for the global object.
  v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();
  // Create a new execution environment containing the built-in functions.
  v8::Handle<v8::Context> context = v8::Context::New();
  // Enter the newly created execution environment.
  v8::Context::Scope context_scope(context);

  bool run_shell = (argc == 1);
  for (int i = 1; i < argc; i++) {
    const char* str = argv[i];
    if (strcmp(str, "--shell") == 0) {
      run_shell = true;
    } else if (strcmp(str, "-f") == 0) {
      // Ignore any -f flags for compatibility with the other stand-
      // alone JavaScript engines.
      continue;
    } else if (strncmp(str, "--", 2) == 0) {
      printf("Warning: unknown flag %s.\nTry --help for options\n", str);
    } else if (strcmp(str, "-e") == 0 && i + 1 < argc) {
      // Execute argument given to -e option directly
      v8::HandleScope handle_scope;
      v8::Handle<v8::String> file_name = v8::String::New("unnamed");
      v8::Handle<v8::String> source = v8::String::New(argv[i + 1]);
      if (!LExecuteString(source, file_name, false, true))
        return 1;
      i++;
    } else {
      // Use all other arguments as names of files to load and run.
      v8::HandleScope handle_scope;
      v8::Handle<v8::String> file_name = v8::String::New(str);
      v8::Handle<v8::String> source = LReadFile(str);
      if (source.IsEmpty()) {
        printf("Error reading '%s'\n", str);
        return 1;
      }
      if (!LExecuteString(source, file_name, false, true))
        return 1;
    }
  }
  if (run_shell) LRunShell(context);
  return 0;
}

// The read-eval-execute loop of the shell.
void LRunShell(v8::Handle<v8::Context> context) {
  printf("V8 version %s\n", v8::V8::GetVersion());
  static const int kBufferSize = 256;
  while (true) {
    char buffer[kBufferSize];
    printf("> ");
    char* str = fgets(buffer, kBufferSize, stdin);
    if (str == NULL) break;
    v8::HandleScope handle_scope;
    LExecuteString(v8::String::New(str),
                  v8::String::New("(shell)"),
                  true,
                  true);
  }
  printf("\n");
}


// Executes a string within the current v8 context.
bool LExecuteString(v8::Handle<v8::String> source,
                   v8::Handle<v8::Value> name,
                   bool print_result,
                   bool report_exceptions) {
  v8::HandleScope handle_scope;
  v8::TryCatch try_catch;
  v8::Handle<v8::Script> script = v8::Script::Compile(source, name);
  if (script.IsEmpty()) {
    // Print errors that happened during compilation.
    if (report_exceptions)
      LReportException(&try_catch);
    return false;
  } else {
    v8::Handle<v8::Value> result = script->Run();
    if (result.IsEmpty()) {
      // Print errors that happened during execution.
      if (report_exceptions)
        LReportException(&try_catch);
      return false;
    } else {
      if (print_result && !result->IsUndefined()) {
        printf("\nresult: 0x%x\n", *reinterpret_cast<int*>(*result));
      }
      return true;
    }
  }
}

void LReportException(v8::TryCatch* try_catch) {
  v8::HandleScope handle_scope;
  v8::String::Utf8Value exception(try_catch->Exception());
  const char* exception_string = LToCString(exception);
  v8::Handle<v8::Message> message = try_catch->Message();
  if (message.IsEmpty()) {
    // V8 didn't provide any extra information about this error; just
    // print the exception.
    printf("%s\n", exception_string);
  } else {
    // Print (filename):(line number): (message).
    v8::String::Utf8Value filename(message->GetScriptResourceName());
    const char* filename_string = LToCString(filename);
    int linenum = message->GetLineNumber();
    printf("%s:%i: %s\n", filename_string, linenum, exception_string);
    // Print line of source code.
    v8::String::Utf8Value sourceline(message->GetSourceLine());
    const char* sourceline_string = LToCString(sourceline);
    printf("%s\n", sourceline_string);
    // Print wavy underline (GetUnderline is deprecated).
    int start = message->GetStartColumn();
    for (int i = 0; i < start; i++) {
      printf(" ");
    }
    int end = message->GetEndColumn();
    for (int i = start; i < end; i++) {
      printf("^");
    }
    printf("\n");
  }
}

// Reads a file into a v8 string.
v8::Handle<v8::String> LReadFile(const char* name) {
  FILE* file = fopen(name, "rb");
  if (file == NULL) return v8::Handle<v8::String>();

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind(file);

  char* chars = new char[size + 1];
  chars[size] = '\0';
  for (int i = 0; i < size;) {
    int read = fread(&chars[i], 1, size - i, file);
    i += read;
  }
  fclose(file);
  v8::Handle<v8::String> result = v8::String::New(chars, size);
  delete[] chars;
  return result;
}

