//! This build script aims at:
//!
//! * generating the C header files for the C API,
//! * setting `wasmer-inline-c` up.

use cbindgen::{Builder, Language};
use std::{
    env,
    ffi::OsStr,
    fs,
    path::{Path, PathBuf},
};

const PRE_HEADER: &str = r#"
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
"#;

#[allow(unused)]
const UNIVERSAL_FEATURE_AS_C_DEFINE: &str = "WASMER_UNIVERSAL_ENABLED";

#[allow(unused)]
const COMPILER_FEATURE_AS_C_DEFINE: &str = "WASMER_COMPILER_ENABLED";

#[allow(unused)]
const WASI_FEATURE_AS_C_DEFINE: &str = "WASMER_WASI_ENABLED";

#[allow(unused)]
const MIDDLEWARES_FEATURE_AS_C_DEFINE: &str = "WASMER_MIDDLEWARES_ENABLED";

#[allow(unused)]
const EMSCRIPTEN_FEATURE_AS_C_DEFINE: &str = "WASMER_EMSCRIPTEN_ENABLED";

#[allow(unused)]
const JSC_FEATURE_AS_C_DEFINE: &str = "WASMER_JSC_BACKEND";

macro_rules! map_feature_as_c_define {
    ($feature:expr, $c_define:ident, $accumulator:ident) => {
        #[cfg(feature = $feature)]
        {
            use std::fmt::Write;
            let _ = write!(
                $accumulator,
                r#"
// The `{feature}` feature has been enabled for this build.
#define {define}
"#,
                feature = $feature,
                define = $c_define,
            );
        }
    };
}

fn main() {
    if !running_self() {
        return;
    }

    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let out_dir = env::var("OUT_DIR").unwrap();

    build_wasm_c_api_headers(&crate_dir, &out_dir);
    build_inline_c_env_vars();
    build_cdylib_link_arg();
}

/// Check whether we should build the C API headers or set `wasmer-inline-c` up.
fn running_self() -> bool {
    env::var("DOCS_RS").is_err()
        && env::var("_CBINDGEN_IS_RUNNING").is_err()
        && env::var("WASMER_PUBLISH_SCRIPT_IS_RUNNING").is_err()
}

/// Build the header files for the `wasm_c_api` API.
fn build_wasm_c_api_headers(crate_dir: &str, out_dir: &str) {
    let mut crate_header_file = PathBuf::from(crate_dir);
    crate_header_file.push("wasmer");

    let mut out_header_file = PathBuf::from(out_dir);
    out_header_file.push("wasmer");

    let mut pre_header = format!(
        r#"// The Wasmer C/C++ header file compatible with the [`wasm-c-api`]
// standard API, as `wasm.h` (included here).
//
// This file is automatically generated by `lib/c-api/build.rs` of the
// [`wasmer-c-api`] Rust crate.
//
// # Stability
//
// The [`wasm-c-api`] standard API is a _living_ standard. There is no
// commitment for stability yet. We (Wasmer) will try our best to keep
// backward compatibility as much as possible. Nonetheless, some
// necessary API aren't yet standardized, and as such, we provide a
// custom API, e.g. `wasi_*` types and functions.
//
// The documentation makes it clear whether a function is unstable.
// 
// When a type or a function will be deprecated, it will be marked as
// such with the appropriated compiler warning, and will be removed at
// the next release round.
//
// # Documentation
//
// At the time of writing, the [`wasm-c-api`] standard has no
// documentation. This file also does not include inline
// documentation. However, we have made (and we continue to make) an
// important effort to document everything. [See the documentation
// online][documentation]. Please refer to this page for the real
// canonical documentation. It also contains numerous examples.
//
// To generate the documentation locally, run `cargo doc --open` from
// within the [`wasmer-c-api`] Rust crate.
//
// [`wasm-c-api`]: https://github.com/WebAssembly/wasm-c-api
// [`wasmer-c-api`]: https://github.com/wasmerio/wasmer/tree/master/lib/c-api
// [documentation]: https://wasmerio.github.io/wasmer/crates/wasmer_c_api/

#if !defined(WASMER_H_PRELUDE)

#define WASMER_H_PRELUDE
{pre_header}"#,
        pre_header = PRE_HEADER
    );

    map_feature_as_c_define!("jsc", JSC_FEATURE_AS_C_DEFINE, pre_header);
    map_feature_as_c_define!("compiler", UNIVERSAL_FEATURE_AS_C_DEFINE, pre_header);
    map_feature_as_c_define!("compiler", COMPILER_FEATURE_AS_C_DEFINE, pre_header);
    map_feature_as_c_define!("wasi", WASI_FEATURE_AS_C_DEFINE, pre_header);
    map_feature_as_c_define!("middlewares", MIDDLEWARES_FEATURE_AS_C_DEFINE, pre_header);
    map_feature_as_c_define!("emscripten", EMSCRIPTEN_FEATURE_AS_C_DEFINE, pre_header);

    add_wasmer_version(&mut pre_header);

    // Close pre header.
    pre_header.push_str(
        r#"
#endif // WASMER_H_PRELUDE


//
// OK, here we go. The code below is automatically generated.
//
"#,
    );

    let guard = "WASMER_H";

    // C bindings.
    {
        // Generate the bindings in the `OUT_DIR`.
        out_header_file.set_extension("h");

        // Build and generate the header file.
        new_builder(Language::C, crate_dir, guard, &pre_header)
            .with_include("wasm.h")
            .generate()
            .expect("Unable to generate C bindings")
            .write_to_file(out_header_file.as_path());

        // Copy the generated bindings from `OUT_DIR` to
        // `CARGO_MANIFEST_DIR`.
        crate_header_file.set_extension("h");

        fs::copy(out_header_file.as_path(), crate_header_file.as_path())
            .expect("Unable to copy the generated C bindings");
    }
}

fn add_wasmer_version(pre_header: &mut String) {
    use std::fmt::Write;
    let _ = write!(
        pre_header,
        r#"
// This file corresponds to the following Wasmer version.
#define WASMER_VERSION "{full}"
#define WASMER_VERSION_MAJOR {major}
#define WASMER_VERSION_MINOR {minor}
#define WASMER_VERSION_PATCH {patch}
#define WASMER_VERSION_PRE "{pre}"
"#,
        full = env!("CARGO_PKG_VERSION"),
        major = env!("CARGO_PKG_VERSION_MAJOR"),
        minor = env!("CARGO_PKG_VERSION_MINOR"),
        patch = env!("CARGO_PKG_VERSION_PATCH"),
        pre = env!("CARGO_PKG_VERSION_PRE"),
    );
}

/// Create a fresh new `Builder`, already pre-configured.
fn new_builder(language: Language, crate_dir: &str, include_guard: &str, header: &str) -> Builder {
    Builder::new()
        .with_config(cbindgen::Config {
            sort_by: cbindgen::SortKey::Name,
            cpp_compat: true,
            ..cbindgen::Config::default()
        })
        .with_language(language)
        .with_crate(crate_dir)
        .with_include_guard(include_guard)
        .with_header(header)
        .with_documentation(false)
        .with_define("target_family", "windows", "_WIN32")
        .with_define("target_arch", "x86_64", "ARCH_X86_64")
        .with_define("feature", "universal", UNIVERSAL_FEATURE_AS_C_DEFINE)
        .with_define("feature", "compiler", COMPILER_FEATURE_AS_C_DEFINE)
        .with_define("feature", "wasi", WASI_FEATURE_AS_C_DEFINE)
        .with_define("feature", "emscripten", EMSCRIPTEN_FEATURE_AS_C_DEFINE)
}

fn build_inline_c_env_vars() {
    let shared_object_dir = shared_object_dir();
    let shared_object_dir = shared_object_dir.as_path().to_string_lossy();
    let include_dir = env::var("CARGO_MANIFEST_DIR").unwrap();

    // The following options mean:
    //
    // * `-I`, add `include_dir` to include search path,
    // * `-L`, add `shared_object_dir` to library search path,
    // * `-D_DEBUG`, enable debug mode to enable `assert.h`.
    // * `-D_CRT_SECURE_NO_WARNINGS`, disable security features in the
    //   Windows C runtime, which allows to use `getenv` without any
    //   warnings.
    println!(
        "cargo:rustc-env=INLINE_C_RS_CFLAGS=-I{I} -L{L} -D_DEBUG -D_CRT_SECURE_NO_WARNINGS",
        I = include_dir,
        L = shared_object_dir.clone(),
    );

    if let Ok(compiler_engine) = env::var("TEST") {
        println!(
            "cargo:rustc-env=INLINE_C_RS_TEST={test}",
            test = compiler_engine
        );
    }

    println!(
        "cargo:rustc-env=INLINE_C_RS_LDFLAGS=-rpath,{shared_object_dir} {shared_object_dir}/{lib}",
        shared_object_dir = shared_object_dir,
        lib = if cfg!(target_os = "windows") {
            "wasmer.dll".to_string()
        } else if cfg!(target_vendor = "apple") {
            "libwasmer.dylib".to_string()
        } else {
            let path = format!(
                "{shared_object_dir}/{lib}",
                shared_object_dir = shared_object_dir,
                lib = "libwasmer.so"
            );

            if Path::new(path.as_str()).exists() {
                "libwasmer.so".to_string()
            } else {
                "libwasmer.a".to_string()
            }
        }
    );
}

fn build_cdylib_link_arg() {
    // Code inspired by the `cdylib-link-lines` crate.
    let mut lines = Vec::new();
    let os = env::var("CARGO_CFG_TARGET_OS").unwrap();
    let env = env::var("CARGO_CFG_TARGET_ENV").unwrap();
    let version_major = env::var("CARGO_PKG_VERSION_MAJOR").unwrap();
    let version_minor = env::var("CARGO_PKG_VERSION_MINOR").unwrap();
    let version_patch = env::var("CARGO_PKG_VERSION_PATCH").unwrap();
    let shared_object_dir = shared_object_dir();

    match (os.as_str(), env.as_str()) {
        ("android", _) => {
            lines.push("-Wl,-soname,libwasmer.so".to_string());
        }

        ("linux", _) | ("freebsd", _) | ("dragonfly", _) | ("netbsd", _) if env != "musl" => {
            lines.push("-Wl,-soname,libwasmer.so".to_string());
        }

        ("macos", _) | ("ios", _) => {
            lines.push(format!(
                "-Wl,-install_name,@rpath/libwasmer.dylib,-current_version,{x}.{y}.{z},-compatibility_version,{x}",
                x = version_major,
                y = version_minor,
                z = version_patch,
            ));
        }

        ("windows", "gnu") => {
            // This is only set up to work on GNU toolchain versions of Rust
            lines.push(format!(
                "-Wl,--out-implib,{}",
                shared_object_dir.join("wasmer.dll.a").display()
            ));
            lines.push(format!(
                "-Wl,--output-def,{}",
                shared_object_dir.join("wasmer.def").display()
            ));
        }

        _ => {}
    }

    for line in lines {
        println!("cargo:rustc-cdylib-link-arg={}", line);
    }
}

fn shared_object_dir() -> PathBuf {
    // We start from `OUT_DIR` because `cargo publish` uses a different directory
    // so traversing from `CARGO_MANIFEST_DIR` is less reliable.
    let mut shared_object_dir = PathBuf::from(env::var("OUT_DIR").unwrap());

    assert_eq!(shared_object_dir.file_name(), Some(OsStr::new("out")));
    shared_object_dir.pop();

    assert!(shared_object_dir
        .file_name()
        .as_ref()
        .unwrap()
        .to_string_lossy()
        .to_string()
        .starts_with("wasmer-c-api"));
    shared_object_dir.pop();

    assert_eq!(shared_object_dir.file_name(), Some(OsStr::new("build")));
    shared_object_dir.pop();
    shared_object_dir.pop(); // "debug" or "release"

    // We either find `target` or the target triple if cross-compiling.
    if shared_object_dir.file_name() != Some(OsStr::new("target")) {
        let target = env::var("TARGET").unwrap();
        if shared_object_dir.file_name() != Some(OsStr::new("llvm-cov-target")) {
            assert_eq!(shared_object_dir.file_name(), Some(OsStr::new(&target)));
        } else {
            shared_object_dir.set_file_name(&target);
        }
    }

    shared_object_dir.push(env::var("PROFILE").unwrap());

    shared_object_dir
}
