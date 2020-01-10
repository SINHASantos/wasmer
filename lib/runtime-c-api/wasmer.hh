
#if !defined(WASMER_H_MACROS)

#define WASMER_H_MACROS

// Define the `ARCH_X86_X64` constant.
#if defined(MSVC) && defined(_M_AMD64)
#  define ARCH_X86_64
#elif (defined(GCC) || defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__)
#  define ARCH_X86_64
#endif

// Compatibility with non-Clang compilers.
#if !defined(__has_attribute)
#  define __has_attribute(x) 0
#endif

// Compatibility with non-Clang compilers.
#if !defined(__has_declspec_attribute)
#  define __has_declspec_attribute(x) 0
#endif

// Define the `DEPRECATED` macro.
#if defined(GCC) || defined(__GNUC__) || __has_attribute(deprecated)
#  define DEPRECATED(message) __attribute__((deprecated(message)))
#elif defined(MSVC) || __has_declspec_attribute(deprecated)
#  define DEPRECATED(message) __declspec(deprecated(message))
#endif

#define WASMER_WASI_ENABLED
#endif // WASMER_H_MACROS


#ifndef WASMER_H
#define WASMER_H

#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <new>

#if defined(WASMER_WASI_ENABLED)
enum class Version : uint8_t {
  /// Version cannot be detected or is unknown.
  Unknown = 0,
  /// Latest version. See `wasmer_wasi::WasiVersion::Latest` to
  /// learn more.
  Latest = 1,
  /// `wasi_unstable`.
  Snapshot0 = 2,
  /// `wasi_snapshot_preview1`.
  Snapshot1 = 3,
};
#endif

/// List of export/import kinds.
enum class wasmer_import_export_kind : uint32_t {
  WASM_FUNCTION = 0,
  WASM_GLOBAL = 1,
  WASM_MEMORY = 2,
  WASM_TABLE = 3,
};

enum class wasmer_result_t {
  WASMER_OK = 1,
  WASMER_ERROR = 2,
};

enum class wasmer_value_tag : uint32_t {
  WASM_I32,
  WASM_I64,
  WASM_F32,
  WASM_F64,
};

struct wasmer_module_t {

};

struct wasmer_instance_t {

};

struct wasmer_byte_array {
  const uint8_t *bytes;
  uint32_t bytes_len;
};

#if defined(WASMER_EMSCRIPTEN_ENABLED)
/// Type used to construct an import_object_t with Emscripten imports.
struct wasmer_emscripten_globals_t {

};
#endif

struct wasmer_import_object_t {

};

/// Opaque pointer to `NamedExportDescriptor`.
struct wasmer_export_descriptor_t {

};

/// Opaque pointer to `NamedExportDescriptors`.
struct wasmer_export_descriptors_t {

};

/// Opaque pointer to `wasmer_export_t`.
struct wasmer_export_func_t {

};

union wasmer_value {
  int32_t I32;
  int64_t I64;
  float F32;
  double F64;
};

struct wasmer_value_t {
  wasmer_value_tag tag;
  wasmer_value value;
};

/// Opaque pointer to `NamedExport`.
struct wasmer_export_t {

};

struct wasmer_memory_t {

};

/// Opaque pointer to `NamedExports`.
struct wasmer_exports_t {

};

struct wasmer_global_t {

};

struct wasmer_global_descriptor_t {
  bool mutable_;
  wasmer_value_tag kind;
};

struct wasmer_import_descriptor_t {

};

struct wasmer_import_descriptors_t {

};

struct wasmer_import_func_t {

};

struct wasmer_table_t {

};

/// Union of import/export value.
union wasmer_import_export_value {
  const wasmer_import_func_t *func;
  const wasmer_table_t *table;
  const wasmer_memory_t *memory;
  const wasmer_global_t *global;
};

struct wasmer_import_t {
  wasmer_byte_array module_name;
  wasmer_byte_array import_name;
  wasmer_import_export_kind tag;
  wasmer_import_export_value value;
};

struct wasmer_import_object_iter_t {

};

struct wasmer_instance_context_t {

};

struct wasmer_limit_option_t {
  bool has_some;
  uint32_t some;
};

struct wasmer_limits_t {
  uint32_t min;
  wasmer_limit_option_t max;
};

struct wasmer_serialized_module_t {

};

#if (!defined(_WIN32) && defined(ARCH_X86_64))
struct wasmer_trampoline_buffer_builder_t {

};
#endif

#if (!defined(_WIN32) && defined(ARCH_X86_64))
struct wasmer_trampoline_callable_t {

};
#endif

#if (!defined(_WIN32) && defined(ARCH_X86_64))
struct wasmer_trampoline_buffer_t {

};
#endif

#if defined(WASMER_WASI_ENABLED)
/// Opens a directory that's visible to the WASI module as `alias` but
/// is backed by the host file at `host_file_path`
struct wasmer_wasi_map_dir_entry_t {
  /// What the WASI module will see in its virtual root
  wasmer_byte_array alias;
  /// The backing file that the WASI module will interact with via the alias
  wasmer_byte_array host_file_path;
};
#endif

extern "C" {

/// Creates a new Module from the given wasm bytes.
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_compile(wasmer_module_t **module,
                               uint8_t *wasm_bytes,
                               uint32_t wasm_bytes_len);

#if defined(WASMER_EMSCRIPTEN_ENABLED)
/// Convenience function for setting up arguments and calling the Emscripten
/// main function.
///
/// WARNING:
///
/// Do not call this function on untrusted code when operating without
/// additional sandboxing in place.
/// Emscripten has access to many host system calls and therefore may do very
/// bad things.
wasmer_result_t wasmer_emscripten_call_main(wasmer_instance_t *instance,
                                            const wasmer_byte_array *args,
                                            unsigned int args_len);
#endif

#if defined(WASMER_EMSCRIPTEN_ENABLED)
/// Destroy `wasmer_emscrpten_globals_t` created by
/// `wasmer_emscripten_get_emscripten_globals`.
void wasmer_emscripten_destroy_globals(wasmer_emscripten_globals_t *globals);
#endif

#if defined(WASMER_EMSCRIPTEN_ENABLED)
/// Create a `wasmer_import_object_t` with Emscripten imports, use
/// `wasmer_emscripten_get_emscripten_globals` to get a
/// `wasmer_emscripten_globals_t` from a `wasmer_module_t`.
///
/// WARNING:
///1
/// This `import_object_t` contains thin-wrappers around host system calls.
/// Do not use this to execute untrusted code without additional sandboxing.
wasmer_import_object_t *wasmer_emscripten_generate_import_object(wasmer_emscripten_globals_t *globals);
#endif

#if defined(WASMER_EMSCRIPTEN_ENABLED)
/// Create a `wasmer_emscripten_globals_t` from a Wasm module.
wasmer_emscripten_globals_t *wasmer_emscripten_get_globals(const wasmer_module_t *module);
#endif

#if defined(WASMER_EMSCRIPTEN_ENABLED)
/// Execute global constructors (required if the module is compiled from C++)
/// and sets up the internal environment.
///
/// This function sets the data pointer in the same way that
/// [`wasmer_instance_context_data_set`] does.
wasmer_result_t wasmer_emscripten_set_up(wasmer_instance_t *instance,
                                         wasmer_emscripten_globals_t *globals);
#endif

/// Gets export descriptor kind
wasmer_import_export_kind wasmer_export_descriptor_kind(wasmer_export_descriptor_t *export_);

/// Gets name for the export descriptor
wasmer_byte_array wasmer_export_descriptor_name(wasmer_export_descriptor_t *export_descriptor);

/// Gets export descriptors for the given module
///
/// The caller owns the object and should call `wasmer_export_descriptors_destroy` to free it.
void wasmer_export_descriptors(const wasmer_module_t *module,
                               wasmer_export_descriptors_t **export_descriptors);

/// Frees the memory for the given export descriptors
void wasmer_export_descriptors_destroy(wasmer_export_descriptors_t *export_descriptors);

/// Gets export descriptor by index
wasmer_export_descriptor_t *wasmer_export_descriptors_get(wasmer_export_descriptors_t *export_descriptors,
                                                          int idx);

/// Gets the length of the export descriptors
int wasmer_export_descriptors_len(wasmer_export_descriptors_t *exports);

/// Calls a `func` with the provided parameters.
/// Results are set using the provided `results` pointer.
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_export_func_call(const wasmer_export_func_t *func,
                                        const wasmer_value_t *params,
                                        unsigned int params_len,
                                        wasmer_value_t *results,
                                        unsigned int results_len);

/// Sets the params buffer to the parameter types of the given wasmer_export_func_t
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_export_func_params(const wasmer_export_func_t *func,
                                          wasmer_value_tag *params,
                                          uint32_t params_len);

/// Sets the result parameter to the arity of the params of the wasmer_export_func_t
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_export_func_params_arity(const wasmer_export_func_t *func, uint32_t *result);

/// Sets the returns buffer to the parameter types of the given wasmer_export_func_t
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_export_func_returns(const wasmer_export_func_t *func,
                                           wasmer_value_tag *returns,
                                           uint32_t returns_len);

/// Sets the result parameter to the arity of the returns of the wasmer_export_func_t
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_export_func_returns_arity(const wasmer_export_func_t *func,
                                                 uint32_t *result);

/// Gets wasmer_export kind
wasmer_import_export_kind wasmer_export_kind(wasmer_export_t *export_);

/// Gets name from wasmer_export
wasmer_byte_array wasmer_export_name(wasmer_export_t *export_);

/// Gets export func from export
const wasmer_export_func_t *wasmer_export_to_func(const wasmer_export_t *export_);

/// Gets a memory pointer from an export pointer.
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_export_to_memory(const wasmer_export_t *export_, wasmer_memory_t **memory);

/// Frees the memory for the given exports
void wasmer_exports_destroy(wasmer_exports_t *exports);

/// Gets wasmer_export by index
wasmer_export_t *wasmer_exports_get(wasmer_exports_t *exports, int idx);

/// Gets the length of the exports
int wasmer_exports_len(wasmer_exports_t *exports);

/// Frees memory for the given Global
void wasmer_global_destroy(wasmer_global_t *global);

/// Gets the value stored by the given Global
wasmer_value_t wasmer_global_get(wasmer_global_t *global);

/// Returns a descriptor (type, mutability) of the given Global
wasmer_global_descriptor_t wasmer_global_get_descriptor(wasmer_global_t *global);

/// Creates a new Global and returns a pointer to it.
/// The caller owns the object and should call `wasmer_global_destroy` to free it.
wasmer_global_t *wasmer_global_new(wasmer_value_t value, bool mutable_);

/// Sets the value stored by the given Global
void wasmer_global_set(wasmer_global_t *global, wasmer_value_t value);

/// Gets export descriptor kind
wasmer_import_export_kind wasmer_import_descriptor_kind(wasmer_import_descriptor_t *export_);

/// Gets module name for the import descriptor
wasmer_byte_array wasmer_import_descriptor_module_name(wasmer_import_descriptor_t *import_descriptor);

/// Gets name for the import descriptor
wasmer_byte_array wasmer_import_descriptor_name(wasmer_import_descriptor_t *import_descriptor);

/// Gets import descriptors for the given module
///
/// The caller owns the object and should call `wasmer_import_descriptors_destroy` to free it.
void wasmer_import_descriptors(const wasmer_module_t *module,
                               wasmer_import_descriptors_t **import_descriptors);

/// Frees the memory for the given import descriptors
void wasmer_import_descriptors_destroy(wasmer_import_descriptors_t *import_descriptors);

/// Gets import descriptor by index
wasmer_import_descriptor_t *wasmer_import_descriptors_get(wasmer_import_descriptors_t *import_descriptors,
                                                          unsigned int idx);

/// Gets the length of the import descriptors
unsigned int wasmer_import_descriptors_len(wasmer_import_descriptors_t *exports);

/// Frees memory for the given Func
void wasmer_import_func_destroy(wasmer_import_func_t *func);

/// Creates new func
///
/// The caller owns the object and should call `wasmer_import_func_destroy` to free it.
wasmer_import_func_t *wasmer_import_func_new(void (*func)(void *data),
                                             const wasmer_value_tag *params,
                                             unsigned int params_len,
                                             const wasmer_value_tag *returns,
                                             unsigned int returns_len);

/// Sets the params buffer to the parameter types of the given wasmer_import_func_t
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_import_func_params(const wasmer_import_func_t *func,
                                          wasmer_value_tag *params,
                                          unsigned int params_len);

/// Sets the result parameter to the arity of the params of the wasmer_import_func_t
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_import_func_params_arity(const wasmer_import_func_t *func, uint32_t *result);

/// Sets the returns buffer to the parameter types of the given wasmer_import_func_t
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_import_func_returns(const wasmer_import_func_t *func,
                                           wasmer_value_tag *returns,
                                           unsigned int returns_len);

/// Sets the result parameter to the arity of the returns of the wasmer_import_func_t
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_import_func_returns_arity(const wasmer_import_func_t *func,
                                                 uint32_t *result);

/// Frees memory of the given ImportObject
void wasmer_import_object_destroy(wasmer_import_object_t *import_object);

/// Extends an existing import object with new imports
wasmer_result_t wasmer_import_object_extend(wasmer_import_object_t *import_object,
                                            const wasmer_import_t *imports,
                                            unsigned int imports_len);

/// Gets an entry from an ImportObject at the name and namespace.
/// Stores `name`, `namespace`, and `import_export_value` in `import`.
/// Thus these must remain valid for the lifetime of `import`.
///
/// The caller owns all data involved.
/// `import_export_value` will be written to based on `tag`.
wasmer_result_t wasmer_import_object_get_import(const wasmer_import_object_t *import_object,
                                                wasmer_byte_array namespace_,
                                                wasmer_byte_array name,
                                                wasmer_import_t *import,
                                                wasmer_import_export_value *import_export_value,
                                                uint32_t tag);

/// Frees the memory allocated in `wasmer_import_object_iter_next`
///
/// This function does not free the memory in `wasmer_import_object_t`;
/// it only frees memory allocated while querying a `wasmer_import_object_t`.
void wasmer_import_object_imports_destroy(wasmer_import_t *imports, uint32_t imports_len);

/// Returns true if further calls to `wasmer_import_object_iter_next` will
/// not return any new data
bool wasmer_import_object_iter_at_end(wasmer_import_object_iter_t *import_object_iter);

/// Frees the memory allocated by `wasmer_import_object_iterate_functions`
void wasmer_import_object_iter_destroy(wasmer_import_object_iter_t *import_object_iter);

/// Writes the next value to `import`.  `WASMER_ERROR` is returned if there
/// was an error or there's nothing left to return.
///
/// To free the memory allocated here, pass the import to `wasmer_import_object_imports_destroy`.
/// To check if the iterator is done, use `wasmer_import_object_iter_at_end`.
wasmer_result_t wasmer_import_object_iter_next(wasmer_import_object_iter_t *import_object_iter,
                                               wasmer_import_t *import);

/// Create an iterator over the functions in the import object.
/// Get the next import with `wasmer_import_object_iter_next`
/// Free the iterator with `wasmer_import_object_iter_destroy`
wasmer_import_object_iter_t *wasmer_import_object_iterate_functions(const wasmer_import_object_t *import_object);

/// Creates a new empty import object.
/// See also `wasmer_import_object_append`
wasmer_import_object_t *wasmer_import_object_new();

/// Hello
unsigned int wasmer_import_trap(const wasmer_instance_context_t *ctx, const char *error_message);

/// Calls an instances exported function by `name` with the provided parameters.
/// Results are set using the provided `results` pointer.
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_instance_call(wasmer_instance_t *instance,
                                     const char *name,
                                     const wasmer_value_t *params,
                                     uint32_t params_len,
                                     wasmer_value_t *results,
                                     uint32_t results_len);

/// Gets the `data` field within the context.
void *wasmer_instance_context_data_get(const wasmer_instance_context_t *ctx);

/// Sets the `data` field of the instance context. This context will be
/// passed to all imported function for instance.
void wasmer_instance_context_data_set(wasmer_instance_t *instance, void *data_ptr);

/// Extracts the instance's context and returns it.
const wasmer_instance_context_t *wasmer_instance_context_get(wasmer_instance_t *instance);

/// Gets the memory within the context at the index `memory_idx`.
/// The index is always 0 until multiple memories are supported.
const wasmer_memory_t *wasmer_instance_context_memory(const wasmer_instance_context_t *ctx,
                                                      uint32_t _memory_idx);

/// Frees memory for the given Instance
void wasmer_instance_destroy(wasmer_instance_t *instance);

/// Gets Exports for the given instance
///
/// The caller owns the object and should call `wasmer_exports_destroy` to free it.
void wasmer_instance_exports(wasmer_instance_t *instance, wasmer_exports_t **exports);

/// Creates a new Instance from the given wasm bytes and imports.
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_instantiate(wasmer_instance_t **instance,
                                   uint8_t *wasm_bytes,
                                   uint32_t wasm_bytes_len,
                                   wasmer_import_t *imports,
                                   int imports_len);

/// Gets the length in bytes of the last error.
/// This can be used to dynamically allocate a buffer with the correct number of
/// bytes needed to store a message.
///
/// # Example
///
/// ```c
/// int error_len = wasmer_last_error_length();
/// char *error_str = malloc(error_len);
/// ```
int wasmer_last_error_length();

/// Stores the last error message into the provided buffer up to the given `length`.
/// The `length` parameter must be large enough to store the last error message.
///
/// Returns the length of the string in bytes.
/// Returns `-1` if an error occurs.
///
/// # Example
///
/// ```c
/// int error_len = wasmer_last_error_length();
/// char *error_str = malloc(error_len);
/// wasmer_last_error_message(error_str, error_len);
/// printf("Error str: `%s`\n", error_str);
/// ```
int wasmer_last_error_message(char *buffer, int length);

/// Gets the start pointer to the bytes within a Memory
uint8_t *wasmer_memory_data(const wasmer_memory_t *mem);

/// Gets the size in bytes of a Memory
uint32_t wasmer_memory_data_length(wasmer_memory_t *mem);

/// Frees memory for the given Memory
void wasmer_memory_destroy(wasmer_memory_t *memory);

/// Grows a Memory by the given number of pages.
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_memory_grow(wasmer_memory_t *memory, uint32_t delta);

/// Returns the current length in pages of the given memory
uint32_t wasmer_memory_length(const wasmer_memory_t *memory);

/// Creates a new Memory for the given descriptor and initializes the given
/// pointer to pointer to a pointer to the new memory.
///
/// The caller owns the object and should call `wasmer_memory_destroy` to free it.
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_memory_new(wasmer_memory_t **memory, wasmer_limits_t limits);

/// Deserialize the given serialized module.
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_module_deserialize(wasmer_module_t **module,
                                          const wasmer_serialized_module_t *serialized_module);

/// Frees memory for the given Module
void wasmer_module_destroy(wasmer_module_t *module);

/// Given:
/// * A prepared `wasmer` import-object
/// * A compiled wasmer module
///
/// Instantiates a wasmer instance
wasmer_result_t wasmer_module_import_instantiate(wasmer_instance_t **instance,
                                                 const wasmer_module_t *module,
                                                 const wasmer_import_object_t *import_object);

/// Creates a new Instance from the given module and imports.
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_module_instantiate(const wasmer_module_t *module,
                                          wasmer_instance_t **instance,
                                          wasmer_import_t *imports,
                                          int imports_len);

/// Serialize the given Module.
///
/// The caller owns the object and should call `wasmer_serialized_module_destroy` to free it.
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_module_serialize(wasmer_serialized_module_t **serialized_module,
                                        const wasmer_module_t *module);

/// Get bytes of the serialized module.
wasmer_byte_array wasmer_serialized_module_bytes(const wasmer_serialized_module_t *serialized_module);

/// Frees memory for the given serialized Module.
void wasmer_serialized_module_destroy(wasmer_serialized_module_t *serialized_module);

/// Transform a sequence of bytes into a serialized module.
///
/// The caller owns the object and should call `wasmer_serialized_module_destroy` to free it.
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_serialized_module_from_bytes(wasmer_serialized_module_t **serialized_module,
                                                    const uint8_t *serialized_module_bytes,
                                                    uint32_t serialized_module_bytes_length);

/// Frees memory for the given Table
void wasmer_table_destroy(wasmer_table_t *table);

/// Grows a Table by the given number of elements.
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_table_grow(wasmer_table_t *table, uint32_t delta);

/// Returns the current length of the given Table
uint32_t wasmer_table_length(wasmer_table_t *table);

/// Creates a new Table for the given descriptor and initializes the given
/// pointer to pointer to a pointer to the new Table.
///
/// The caller owns the object and should call `wasmer_table_destroy` to free it.
///
/// Returns `wasmer_result_t::WASMER_OK` upon success.
///
/// Returns `wasmer_result_t::WASMER_ERROR` upon failure. Use `wasmer_last_error_length`
/// and `wasmer_last_error_message` to get an error message.
wasmer_result_t wasmer_table_new(wasmer_table_t **table, wasmer_limits_t limits);

#if (!defined(_WIN32) && defined(ARCH_X86_64))
/// Adds a callinfo trampoline to the builder.
uintptr_t wasmer_trampoline_buffer_builder_add_callinfo_trampoline(wasmer_trampoline_buffer_builder_t *builder,
                                                                   const wasmer_trampoline_callable_t *func,
                                                                   const void *ctx,
                                                                   uint32_t num_params);
#endif

#if (!defined(_WIN32) && defined(ARCH_X86_64))
/// Adds a context trampoline to the builder.
uintptr_t wasmer_trampoline_buffer_builder_add_context_trampoline(wasmer_trampoline_buffer_builder_t *builder,
                                                                  const wasmer_trampoline_callable_t *func,
                                                                  const void *ctx);
#endif

#if (!defined(_WIN32) && defined(ARCH_X86_64))
/// Finalizes the trampoline builder into an executable buffer.
wasmer_trampoline_buffer_t *wasmer_trampoline_buffer_builder_build(wasmer_trampoline_buffer_builder_t *builder);
#endif

#if (!defined(_WIN32) && defined(ARCH_X86_64))
/// Creates a new trampoline builder.
wasmer_trampoline_buffer_builder_t *wasmer_trampoline_buffer_builder_new();
#endif

#if (!defined(_WIN32) && defined(ARCH_X86_64))
/// Destroys the trampoline buffer if not null.
void wasmer_trampoline_buffer_destroy(wasmer_trampoline_buffer_t *buffer);
#endif

#if (!defined(_WIN32) && defined(ARCH_X86_64))
/// Returns the callable pointer for the trampoline with index `idx`.
const wasmer_trampoline_callable_t *wasmer_trampoline_buffer_get_trampoline(const wasmer_trampoline_buffer_t *buffer,
                                                                            uintptr_t idx);
#endif

#if (!defined(_WIN32) && defined(ARCH_X86_64))
/// Returns the context added by `add_context_trampoline`, from within the callee function.
void *wasmer_trampoline_get_context();
#endif

/// Returns true for valid wasm bytes and false for invalid bytes
bool wasmer_validate(const uint8_t *wasm_bytes, uint32_t wasm_bytes_len);

#if defined(WASMER_WASI_ENABLED)
/// Convenience function that creates a WASI import object with no arguments,
/// environment variables, preopened files, or mapped directories.
///
/// This function is the same as calling [`wasmer_wasi_generate_import_object`] with all
/// empty values.
wasmer_import_object_t *wasmer_wasi_generate_default_import_object();
#endif

#if defined(WASMER_WASI_ENABLED)
/// Creates a WASI import object.
///
/// This function treats null pointers as empty collections.
/// For example, passing null for a string in `args`, will lead to a zero
/// length argument in that position.
wasmer_import_object_t *wasmer_wasi_generate_import_object(const wasmer_byte_array *args,
                                                           unsigned int args_len,
                                                           const wasmer_byte_array *envs,
                                                           unsigned int envs_len,
                                                           const wasmer_byte_array *preopened_files,
                                                           unsigned int preopened_files_len,
                                                           const wasmer_wasi_map_dir_entry_t *mapped_dirs,
                                                           unsigned int mapped_dirs_len);
#endif

#if defined(WASMER_WASI_ENABLED)
/// Creates a WASI import object for a specific version.
///
/// This function is similar to `wasmer_wasi_generate_import_object`
/// except that the first argument describes the WASI version.
///
/// The version is expected to be of kind `Version`.
wasmer_import_object_t *wasmer_wasi_generate_import_object_for_version(unsigned char version,
                                                                       const wasmer_byte_array *args,
                                                                       unsigned int args_len,
                                                                       const wasmer_byte_array *envs,
                                                                       unsigned int envs_len,
                                                                       const wasmer_byte_array *preopened_files,
                                                                       unsigned int preopened_files_len,
                                                                       const wasmer_wasi_map_dir_entry_t *mapped_dirs,
                                                                       unsigned int mapped_dirs_len);
#endif

#if defined(WASMER_WASI_ENABLED)
/// Find the version of WASI used by the module.
///
/// In case of error, the returned version is `Version::Unknown`.
Version wasmer_wasi_get_version(const wasmer_module_t *module);
#endif

} // extern "C"

#endif // WASMER_H
