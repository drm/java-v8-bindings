#include <iostream>
#include "include/nl_melp_v8_V8.h"

JNIEXPORT void JNICALL Java_nl_melp_v8_V8_sayHello__
	(JNIEnv* env, jobject thisObject)
{
    std::cout << "Hello from C++ !!" << std::endl;
}

JNIEXPORT void JNICALL Java_nl_melp_v8_V8_sayHello__Ljava_lang_String_2
	(JNIEnv* env, jobject thisObject, jstring name)
{
    std::cout << "Hello from C++, " << env->GetStringUTFChars(name, NULL) << "!!" << std::endl;
}
