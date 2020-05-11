//! Common module with common used structures across different
//! commands.

use crate::common::WasmFeatures;
use anyhow::{bail, Error, Result};
use std::str::FromStr;
use std::string::ToString;
use std::sync::Arc;
use structopt::StructOpt;
use wasmer::*;
use wasmer_compiler::CompilerConfig;

#[derive(Debug, Clone, StructOpt)]
/// The compiler options
pub struct StoreOptions {
    /// Use Singlepass compiler
    #[structopt(long, conflicts_with_all = &["cranelift", "llvm", "backend"])]
    singlepass: bool,

    /// Use Cranelift compiler
    #[structopt(long, conflicts_with_all = &["singlepass", "llvm", "backend"])]
    cranelift: bool,

    /// Use LLVM compiler
    #[structopt(long, conflicts_with_all = &["singlepass", "cranelift", "backend"])]
    llvm: bool,

    /// Use JIT Engine
    #[structopt(long, conflicts_with_all = &["native"])]
    jit: bool,

    /// Use Native Engine
    #[structopt(long, conflicts_with_all = &["jit"])]
    native: bool,

    /// The deprecated backend flag - Please not use
    #[structopt(long = "backend", hidden = true, conflicts_with_all = &["singlepass", "cranelift", "llvm"])]
    backend: Option<String>,

    #[structopt(flatten)]
    features: WasmFeatures,
    // #[structopt(flatten)]
    // llvm_options: LLVMCLIOptions,
}

#[derive(Debug)]
enum Compiler {
    Singlepass,
    Cranelift,
    LLVM,
}

impl ToString for Compiler {
    fn to_string(&self) -> String {
        match self {
            Self::Singlepass => "singlepass".to_string(),
            Self::Cranelift => "cranelift".to_string(),
            Self::LLVM => "llvm".to_string(),
        }
    }
}

impl FromStr for Compiler {
    type Err = Error;
    fn from_str(s: &str) -> Result<Self> {
        match s {
            "singlepass" => Ok(Self::Singlepass),
            "cranelift" => Ok(Self::Cranelift),
            "llvm" => Ok(Self::LLVM),
            backend => bail!("The `{}` compiler does not exist.", backend),
        }
    }
}

#[cfg(all(feature = "compiler", feature = "engine"))]
impl StoreOptions {
    fn get_compiler(&self) -> Result<Compiler> {
        if self.cranelift {
            return Ok(Compiler::Cranelift);
        } else if self.llvm {
            return Ok(Compiler::LLVM);
        } else if self.singlepass {
            return Ok(Compiler::Singlepass);
        } else if let Some(backend) = self.backend.clone() {
            warning!(
                "the `--backend={0}` flag is deprecated, please use `--{0}` instead",
                backend
            );
            return Compiler::from_str(&backend);
        } else {
            // Auto mode, we choose the best compiler for that platform
            if cfg!(feature = "cranelift") && cfg!(target_arch = "x86_64") {
                return Ok(Compiler::Cranelift);
            } else if cfg!(feature = "singlepass") && cfg!(target_arch = "x86_64") {
                return Ok(Compiler::Singlepass);
            } else if cfg!(feature = "llvm") {
                return Ok(Compiler::LLVM);
            } else {
                bail!("There are no available compilers for your architecture")
            }
        }
    }

    /// Get the Target architecture
    pub fn get_features(&self) -> Result<Features> {
        Ok(Features::default())
    }

    /// Get the Target architecture
    pub fn get_target(&self) -> Result<Target> {
        Ok(Target::default())
    }

    /// Get the Compiler Config for the current options
    #[allow(unused_variables)]
    fn get_config(&self, compiler: Compiler) -> Result<Box<dyn CompilerConfig>> {
        let features = self.get_features()?;
        let target = self.get_target()?;
        let config: Box<dyn CompilerConfig> = match compiler {
            #[cfg(feature = "singlepass")]
            Compiler::Singlepass => {
                let config = wasmer_compiler_singlepass::SinglepassConfig::new(features, target);
                Box::new(config)
            }
            #[cfg(feature = "cranelift")]
            Compiler::Cranelift => {
                let config = wasmer_compiler_cranelift::CraneliftConfig::new(features, target);
                Box::new(config)
            }
            #[cfg(feature = "llvm")]
            Compiler::LLVM => {
                let config = wasmer_compiler_llvm::LLVMConfig::new(features, target);
                Box::new(config)
            }
            #[cfg(not(all(feature = "singlepass", feature = "cranelift", feature = "llvm",)))]
            compiler => bail!(
                "The `{}` compiler is not included in this binary.",
                compiler.to_string()
            ),
        };
        return Ok(config);
    }

    /// Gets the compiler config
    fn get_compiler_config(&self) -> Result<(Box<dyn CompilerConfig>, String)> {
        let compiler = self.get_compiler()?;
        let compiler_name = compiler.to_string();
        let compiler_config = self.get_config(compiler)?;
        Ok((compiler_config, compiler_name))
    }

    /// Gets the tunables for the compiler target
    pub fn get_tunables(&self, compiler_config: &dyn CompilerConfig) -> Tunables {
        Tunables::for_target(compiler_config.target().triple())
    }

    /// Gets the store, with the engine name and compiler name selected
    pub fn get_store(&self) -> Result<(Store, String, String)> {
        let (compiler_config, compiler_name) = self.get_compiler_config()?;
        let tunables = self.get_tunables(&*compiler_config);
        let (engine, engine_name) = self.get_engine_with_compiler(tunables, compiler_config)?;
        let store = Store::new(engine);
        Ok((store, engine_name, compiler_name))
    }

    fn get_engine_with_compiler(
        &self,
        tunables: Tunables,
        compiler_config: Box<dyn CompilerConfig>,
    ) -> Result<(Arc<dyn Engine + Send + Sync>, String)> {
        let engine_type = self.get_engine()?;
        let engine: Arc<dyn Engine + Send + Sync> = match engine_type {
            #[cfg(feature = "jit")]
            EngineOptions::JIT => Arc::new(wasmer_engine_jit::JITEngine::new(
                &*compiler_config,
                tunables,
            )),
            #[cfg(feature = "native")]
            EngineOptions::Native => Arc::new(wasmer_engine_native::NativeEngine::new(
                &*compiler_config,
                tunables,
            )),
            #[cfg(not(all(feature = "jit", feature = "native",)))]
            engine => bail!(
                "The `{}` engine is not included in this binary.",
                engine.to_string()
            ),
        };
        return Ok((engine, engine_type.to_string()));
    }
}

enum EngineOptions {
    JIT,
    Native,
}

impl ToString for EngineOptions {
    fn to_string(&self) -> String {
        match self {
            Self::JIT => "jit".to_string(),
            Self::Native => "native".to_string(),
        }
    }
}

#[cfg(feature = "engine")]
impl StoreOptions {
    fn get_engine(&self) -> Result<EngineOptions> {
        if self.jit {
            Ok(EngineOptions::JIT)
        } else if self.native {
            Ok(EngineOptions::Native)
        } else {
            // Auto mode, we choose the best engine for that platform
            if cfg!(feature = "jit") {
                Ok(EngineOptions::JIT)
            } else if cfg!(feature = "native") {
                Ok(EngineOptions::Native)
            } else {
                bail!("There are no available engines for your architecture")
            }
        }
    }
}

// If we don't have a compiler, but we have an engine
#[cfg(all(not(feature = "compiler"), feature = "engine"))]
impl StoreOptions {
    fn get_engine_headless(&self, tunables: Tunables) -> Result<(Arc<dyn Engine>, String)> {
        let engine_type = self.get_engine()?;
        let engine: Arc<dyn Engine> = match engine_type {
            #[cfg(feature = "jit")]
            EngineOptions::JIT => Arc::new(wasmer_engine_jit::JITEngine::headless(tunables)),
            #[cfg(feature = "native")]
            EngineOptions::Native => {
                Arc::new(wasmer_engine_native::NativeEngine::headless(tunables))
            }
            #[cfg(not(all(feature = "jit", feature = "native",)))]
            engine => bail!(
                "The `{}` engine is not included in this binary.",
                engine.to_string()
            ),
        };
        return Ok((engine, engine_type.to_string()));
    }

    /// Get the store (headless engine)
    pub fn get_store(&self) -> Result<(Store, String, String)> {
        // Get the tunables for the current host
        let tunables = Tunables::default();
        let (engine, engine_name) = self.get_engine_headless(tunables)?;
        let store = Store::new(engine);
        Ok((store, engine_name, "headless".to_string()))
    }
}

// If we don't have any engine enabled
#[cfg(not(feature = "engine"))]
impl StoreOptions {
    /// Get the store (headless engine)
    pub fn get_store(&self) -> Result<(Store, String, String)> {
        bail!("No engines are enabled");
    }
}
