#include <TensorFlowLite_ESP32.h>

#include "main_functions.h"

#include "output_handler.h"
#include "model_data.h"
#include "tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

// Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
String data_rcvd = "";
// Create an area of memory to use for input, output, and intermediate arrays.
// Minimum arena size, at the time of writing. After allocating tensors
// you can retrieve this value by invoking interpreter.arena_used_bytes().
const int kModelArenaSize = 6360;
// Extra headroom for model + alignment + future interpreter changes.
const int kExtraArenaSize = 560 + 16 + 100;
const int kTensorArenaSize = kModelArenaSize + kExtraArenaSize;
uint8_t tensor_arena[kTensorArenaSize];
int norm = 1000000;
}  // namespace


void setup() {
  //Setting up the UART serial communications
  Serial.begin(115200);
  pinMode(12, OUTPUT); //Blue LED
  
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. 
  model = tflite::GetModel(g_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report(
        "Model provided is schema version %d not equal "
        "to supported version %d.",
        model->version(), TFLITE_SCHEMA_VERSION);
        Serial.println("Model provided is schema version no equal to supported version");
    return;
  }

  // This pulls in all the operation implementations we need.
  static tflite::ops::micro::AllOpsResolver resolver;

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    error_reporter->Report("AllocateTensors() failed");
    Serial.println("AllocateTensors() failed");
    return;
  }

  // Obtain pointers to the model's input and output tensors.
  input = interpreter->input(0);
  output = interpreter->output(0);
}

void loop() {
   if (Serial.available() > 0) {
    int inChar = Serial.read();
    if (inChar == '!') {
      float inputData = static_cast<float>(data_rcvd.toInt()) /
                   static_cast<float>(norm);
      input->data.f[0] = inputData;
      Serial.println(inputData,6);
      data_rcvd = ""; //clears variable for new input
      digitalWrite(12, HIGH);
      // Run inference, and report any error
      Serial.println("Running the interpreter");
      TfLiteStatus invoke_status = interpreter->Invoke();
      if (invoke_status != kTfLiteOk) {
        error_reporter->Report("Invoke failed on input.");
        Serial.println("Invoke failed on input.");
        return;
      }
      digitalWrite(12, LOW);
    
      // Read the predicted y value from the model's output tensor
      float y_val = output->data.f[0];
    
      // Output the results.
      Serial.print("y_val is;");
      Serial.println(y_val);
      HandleOutput(error_reporter, y_val);
    } else {
      if (isDigit(inChar)) {
      // convert the incoming byte to a char and add it to the string:
      data_rcvd += (char)inChar;
      }
    }
  }
}
