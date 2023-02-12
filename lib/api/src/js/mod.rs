#[cfg(all(feature = "std", feature = "core"))]
compile_error!(
    "The `std` and `core` features are both enabled, which is an error. Please enable only once."
);

#[cfg(all(not(feature = "std"), not(feature = "core")))]
compile_error!("Both the `std` and `core` features are disabled. Please enable one of them.");

#[cfg(feature = "core")]
pub(crate) extern crate alloc;

mod lib {
    #[cfg(feature = "core")]
    pub mod std {
        pub use crate::alloc::{borrow, boxed, str, string, sync, vec};
        pub use core::fmt;
        pub use hashbrown as collections;
    }

    #[cfg(feature = "std")]
    pub mod std {
        pub use std::{borrow, boxed, collections, fmt, str, string, sync, vec};
    }
}

mod as_js;
pub(crate) mod engine;
pub(crate) mod error;
pub(crate) mod extern_ref;
pub(crate) mod externals;
pub(crate) mod instance;
pub(crate) mod module;
#[cfg(feature = "wasm-types-polyfill")]
mod module_info_polyfill;
pub(crate) mod store;
mod trap;
pub(crate) mod typed_function;
pub(crate) mod vm;
mod wasm_bindgen_polyfill;

pub use crate::js::as_js::AsJs;
pub use crate::js::error::{DeserializeError, LinkError, SerializeError};
pub use crate::js::externals::{
    Extern, FromToNativeWasmType, Function, Global, HostFunction, Memory, MemoryError, MemoryView,
    Table, WasmTypeList,
};
pub use crate::js::module::{Module, ModuleTypeHints};
pub use crate::js::store::StoreObjects;
pub use crate::js::trap::RuntimeError;

#[cfg(feature = "wasm-types-polyfill")]
pub use wasmparser;
