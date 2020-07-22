#include <iostream>
#include "include/nl_melp_v8_V8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cassert>
#include <libplatform/libplatform.h>
#include <v8.h>

const char* HOME = "/home/gerard/git/j2v8-replacement";
bool report_exceptions = true;

void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch);

// We declare these 'globally', because they are reused across contexts.
v8::Isolate::CreateParams create_params;
std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
v8::Isolate* isolate;

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

// The callback that is invoked by v8 whenever the JavaScript 'print'
// function is called.  Prints its arguments on stdout separated by
// spaces and ending with a newline.
void Print(const v8::FunctionCallbackInfo<v8::Value>& args) {
  bool first = true;
  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope(args.GetIsolate());
    if (first) {
      first = false;
    } else {
      printf(" ");
    }
    v8::String::Utf8Value str(args.GetIsolate(), args[i]);
    const char* cstr = ToCString(str);
    printf("%s", cstr);
  }
  printf("\n");
  fflush(stdout);
}

//// Executes a string within the current v8 context.
//bool ExecuteString(v8::Isolate* isolate, v8::Local<v8::String> source,
//                   v8::Local<v8::Value> name, bool print_result,
//                   bool report_exceptions) {
//  v8::HandleScope handle_scope(isolate);
//  v8::TryCatch try_catch(isolate);
//  v8::ScriptOrigin origin(name);
//  v8::Local<v8::Context> context(isolate->GetCurrentContext());
//  v8::Local<v8::Script> script;
//  if (!v8::Script::Compile(context, source, &origin).ToLocal(&script)) {
//    // Print errors that happened during compilation.
//    if (report_exceptions)
//      ReportException(isolate, &try_catch);
//    return false;
//  } else {
//    v8::Local<v8::Value> result;
//    if (!script->Run(context).ToLocal(&result)) {
//      assert(try_catch.HasCaught());
//      // Print errors that happened during execution.
//      if (report_exceptions)
//        ReportException(isolate, &try_catch);
//      return false;
//    } else {
//      assert(!try_catch.HasCaught());
//      if (print_result && !result->IsUndefined()) {
//        // If all went well and the result wasn't undefined then print
//        // the returned value.
//        v8::String::Utf8Value str(isolate, result);
//        const char* cstr = ToCString(str);
//        printf("%s\n", cstr);
//      }
//      return true;
//    }
//  }
//}

void runScript(const char *scriptSource, v8::Local<v8::Context> context, v8::Platform* platform) {
	// Enter the execution environment before evaluating any code.
	v8::Isolate* isolate = context->GetIsolate();
	v8::Context::Scope context_scope(context);
	v8::Local<v8::String> name(
	  v8::String::NewFromUtf8(isolate, "(shell)",
							  v8::NewStringType::kNormal).ToLocalChecked());
	{
		  v8::HandleScope handle_scope(isolate);
          v8::TryCatch try_catch(isolate);
          v8::ScriptOrigin origin(name);
          v8::Local<v8::Context> context(isolate->GetCurrentContext());
          v8::Local<v8::Script> script;
          if (!v8::Script::Compile(context, v8::String::NewFromUtf8(isolate, scriptSource,
                                            									v8::NewStringType::kNormal).ToLocalChecked(), &origin).ToLocal(&script)) {
            // Print errors that happened during compilation.
            if (report_exceptions)
              ReportException(isolate, &try_catch);
          } else {
            v8::Local<v8::Value> result;
            if (!script->Run(context).ToLocal(&result)) {
              assert(try_catch.HasCaught());
              // Print errors that happened during execution.
		    if (report_exceptions)
                ReportException(isolate, &try_catch);
            } else {
              assert(!try_catch.HasCaught());
            }
          }
		while (v8::platform::PumpMessageLoop(platform, context->GetIsolate()))
		  continue;
	}
	fprintf(stderr, "\n");
}

//void *init_isolate() {
	// Creates a new execution environment containing the built-in
	// functions.
	// Create a template for the global object.
	// Bind the global 'print' function to the C++ Print callback.
//	global->Set(isolate, "print", v8::FunctionTemplate::New(isolate, Print));
	// Bind the global 'read' function to the C++ Read callback.
//	global->Set(isolate, "read", v8::FunctionTemplate::New(isolate, Read));
	// Bind the global 'load' function to the C++ Load callback.
//	global->Set(isolate, "load", v8::FunctionTemplate::New(isolate, Load));
	// Bind the 'quit' function
//	global->Set(isolate, "quit", v8::FunctionTemplate::New(isolate, Quit));
	// Bind the 'version' function
//	global->Set(isolate, "version", v8::FunctionTemplate::New(isolate, Version));
//}

// Initializes v8
void _v8_init() {
	v8::V8::InitializeICUDefaultLocation(HOME);
	v8::V8::InitializeExternalStartupData(HOME);
	v8::V8::InitializePlatform(platform.get());
	v8::V8::Initialize();
	create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();

	isolate = v8::Isolate::New(create_params);
}

void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch) {
  v8::HandleScope handle_scope(isolate);
  v8::String::Utf8Value exception(isolate, try_catch->Exception());
  const char* exception_string = ToCString(exception);
  v8::Local<v8::Message> message = try_catch->Message();
  if (message.IsEmpty()) {
    // V8 didn't provide any extra information about this error; just
    // print the exception.
    fprintf(stderr, "%s\n", exception_string);
  } else {
    // Print (filename):(line number): (message).
    v8::String::Utf8Value filename(isolate,
                                   message->GetScriptOrigin().ResourceName());
    v8::Local<v8::Context> context(isolate->GetCurrentContext());
    const char* filename_string = ToCString(filename);
    int linenum = message->GetLineNumber(context).FromJust();
    fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
    // Print line of source code.
    v8::String::Utf8Value sourceline(
        isolate, message->GetSourceLine(context).ToLocalChecked());
    const char* sourceline_string = ToCString(sourceline);
    fprintf(stderr, "%s\n", sourceline_string);
    // Print wavy underline (GetUnderline is deprecated).
    int start = message->GetStartColumn(context).FromJust();
    for (int i = 0; i < start; i++) {
      fprintf(stderr, " ");
    }
    int end = message->GetEndColumn(context).FromJust();
    for (int i = start; i < end; i++) {
      fprintf(stderr, "^");
    }
    fprintf(stderr, "\n");
    v8::Local<v8::Value> stack_trace_string;
    if (try_catch->StackTrace(context).ToLocal(&stack_trace_string) &&
        stack_trace_string->IsString() &&
        v8::Local<v8::String>::Cast(stack_trace_string)->Length() > 0) {
      v8::String::Utf8Value stack_trace(isolate, stack_trace_string);
      const char* stack_trace_string = ToCString(stack_trace);
      fprintf(stderr, "%s\n", stack_trace_string);
    }
  }
}


// Creates a new execution environment containing the built-in
// functions.
v8::Local<v8::Context> CreateShellContext(v8::Isolate* isolate) {
  // Create a template for the global object.
  v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
  // Bind the global 'print' function to the C++ Print callback.
  global->Set(
      v8::String::NewFromUtf8(isolate, "print", v8::NewStringType::kNormal)
          .ToLocalChecked(),
      v8::FunctionTemplate::New(isolate, Print));
//  // Bind the global 'read' function to the C++ Read callback.
//  global->Set(v8::String::NewFromUtf8(
//                  isolate, "read", v8::NewStringType::kNormal).ToLocalChecked(),
//              v8::FunctionTemplate::New(isolate, Read));
//  // Bind the global 'load' function to the C++ Load callback.
//  global->Set(v8::String::NewFromUtf8(
//                  isolate, "load", v8::NewStringType::kNormal).ToLocalChecked(),
//              v8::FunctionTemplate::New(isolate, Load));
//  // Bind the 'quit' function
//  global->Set(v8::String::NewFromUtf8(
//                  isolate, "quit", v8::NewStringType::kNormal).ToLocalChecked(),
//              v8::FunctionTemplate::New(isolate, Quit));
//  // Bind the 'version' function
//  global->Set(
//      v8::String::NewFromUtf8(isolate, "version", v8::NewStringType::kNormal)
//          .ToLocalChecked(),
//      v8::FunctionTemplate::New(isolate, Version));

  return v8::Context::New(isolate, NULL, global);
}

void helloworld()
{
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = CreateShellContext(isolate);
    v8::Context::Scope context_scope(context);

	runScript("java('Hello');", context, platform.get());
}


void _v8_dispose() {
  // Dispose the isolate and tear down V8.
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  delete create_params.array_buffer_allocator;
}

JNIEXPORT void JNICALL Java_nl_melp_v8_V8_sayHello__
	(JNIEnv* env, jobject thisObject)
{
	helloworld();
	helloworld();
}

JNIEXPORT void JNICALL Java_nl_melp_v8_V8_sayHello__Ljava_lang_String_2
	(JNIEnv* env, jobject thisObject, jstring name)
{
    std::cout << "Hello from C++, " << env->GetStringUTFChars(name, NULL) << "!!" << std::endl;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
	_v8_init();
	return JNI_VERSION_1_6;
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {
	_v8_dispose();
}

//
//int main(int argc, char* argv[]) {
//  // Initialize V8.
//  // Create a new Isolate and make it the current one.
//
//}

// helloworld.cc:
//
//  // Initialize V8.
//  v8::V8::InitializeICUDefaultLocation(argv[0]);
//  v8::V8::InitializeExternalStartupData(argv[0]);
//  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
//  v8::V8::InitializePlatform(platform.get());
//  v8::V8::Initialize();
//
//  // { body }

//  // Dispose the isolate and tear down V8.
//  isolate->Dispose();
//  v8::V8::Dispose();
//  v8::V8::ShutdownPlatform();
//  delete create_params.array_buffer_allocator;
