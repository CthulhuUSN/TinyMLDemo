#ifndef TENSORFLOW_LITE_EXPERIMENTAL_MICRO_EXAMPLES_HELLO_WORLD_OUTPUT_HANDLER_H_
#define TENSORFLOW_LITE_EXPERIMENTAL_MICRO_EXAMPLES_HELLO_WORLD_OUTPUT_HANDLER_H_

#include "tensorflow/lite/c/c_api_internal.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"

void HandleOutput(tflite::ErrorReporter* error_reporter, float y_value);

#endif  // TENSORFLOW_LITE_EXPERIMENTAL_MICRO_EXAMPLES_HELLO_WORLD_OUTPUT_HANDLER_H_
