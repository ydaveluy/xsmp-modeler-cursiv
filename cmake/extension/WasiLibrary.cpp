// What a WASI library needs of the C++ exceptions and cannot take from the server:
// `__wasm_lpad_context` is a thread-local, and a library cannot reach one. So it
// has its own, which its landing pads fill. The personality of the server only
// reads the context it is given.
#include <unwind.h>

#include <cstdint>

struct LandingPadContext {
  uintptr_t lpad_index;
  uintptr_t lsda;
  uintptr_t selector;
};

extern "C" {

thread_local LandingPadContext __wasm_lpad_context;

_Unwind_Reason_Code __gxx_personality_wasm0(int, _Unwind_Action, uint64_t,
                                            _Unwind_Exception*,
                                            _Unwind_Context*);

uintptr_t _Unwind_CallPersonality(void* exception) {
  auto* object = static_cast<_Unwind_Exception*>(exception);
  __wasm_lpad_context.selector = 0;
  return __gxx_personality_wasm0(
      1, _UA_SEARCH_PHASE, object->exception_class, object,
      reinterpret_cast<_Unwind_Context*>(&__wasm_lpad_context));
}
}
