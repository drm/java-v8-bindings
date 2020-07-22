#include <iostream>
#include "include/nl_melp_v8_V8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cassert>
#include <cstring>
#include <libplatform/libplatform.h>
#include <v8.h>

const char* HOME = "/home/gerard/git/j2v8-replacement";
bool report_exceptions = true;

void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch);

// We declare these 'globally', because they are reused across contexts.
v8::Isolate::CreateParams create_params;
std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
v8::Isolate* isolate;

// Temporary scoping fix for FunctionTemplate binding.
JNIEnv* g_env;

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

// Call java, see nl.melp.v8.V8.recv(String)
void javaCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
  JNIEnv* env = g_env;
  assert(env != NULL);

  jclass cls = env->FindClass("nl/melp/v8/V8");
  jmethodID mid = env->GetStaticMethodID(cls, "recv", "(Ljava/lang/String;)Ljava/lang/String;");

  v8::EscapableHandleScope handle_scope(args.GetIsolate());
  v8::String::Utf8Value str(args.GetIsolate(), args[0]);
  jstring result = (jstring)env->CallStaticObjectMethod(cls, mid, env->NewStringUTF(ToCString(str)));

  const char* cstr = env->GetStringUTFChars(result, 0);
  args.GetReturnValue().Set(handle_scope.Escape(v8::String::NewFromUtf8(isolate, cstr, v8::NewStringType::kNormal).ToLocalChecked()));
  env->ReleaseStringUTFChars(result, cstr);
}

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

void _v8_dispose() {
  // Dispose the isolate and tear down V8.
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  delete create_params.array_buffer_allocator;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
	_v8_init();
	return JNI_VERSION_1_6;
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {
	_v8_dispose();
}

/*
 * Class:     nl_melp_v8_V8
 * Method:    load
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT void JNICALL Java_nl_melp_v8_V8_run
  (JNIEnv* env, jclass cls, jstring jstr)
{
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);

	// TODO this is ugly as hell, but for now fixes scope binding to the JavaCall function
	g_env = env;

	// Create a template for the global object.
	v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
	// Bind the global 'print' function to the C++ Print callback.
	global->Set(
	  v8::String::NewFromUtf8(isolate, "__do_call", v8::NewStringType::kNormal)
		  .ToLocalChecked(),
	  v8::FunctionTemplate::New(isolate, javaCallback)
  	);

	v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, global);
    v8::Context::Scope context_scope(context);

	runScript(env->GetStringUTFChars(jstr, NULL), context, platform.get());

	// TODO fix scoping.
	g_env = NULL;
}

