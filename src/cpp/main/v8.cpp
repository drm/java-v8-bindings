#include <iostream>
#include "include/nl_melp_v8_V8.h"

JNIEXPORT void JNICALL Java_nl_melp_v8_V8_sayHello
	(JNIEnv* env, jobject thisObject)
{
    std::cout << "Hello from C++ !!" << std::endl;
}
