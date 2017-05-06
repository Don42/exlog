
use std::io::{self};
use std::fs::File;
use std::io::prelude::Read;
use std::convert::From;
use xdg;
use toml;

/// Errors that can occur during config loading
#[derive(Debug)]
pub enum Error {
    /// Config file not found
    NotFound,

    /// Unable to load xdg directories
    XDG(xdg::BaseDirectoriesError),

    /// IO error reading file
    IO(io::Error),

    /// Invalid toml in file
    TOML(toml::de::Error),
}

impl From<io::Error> for Error {
    fn from(val: io::Error) -> Error {
        if val.kind() == io::ErrorKind::NotFound {
            Error::NotFound
        } else {
            Error::IO(val)
        }
    }
}

impl From<toml::de::Error> for Error {
    fn from(val: toml::de::Error) -> Error {
        Error::TOML(val)
    }
}

impl From<xdg::BaseDirectoriesError> for Error {
    fn from(val: xdg::BaseDirectoriesError) -> Error {
        Error::XDG(val)
    }
}


#[derive(Debug,Deserialize)]
pub struct Config {
    test: Option<String>,
}

pub type Result<T> = ::std::result::Result<T, Error>;

impl Config {
    pub fn load() -> Result<Config> {
        // Unsure under which conditions this fails
        let dirs = xdg::BaseDirectories::with_prefix("exlog")?;

        let config_path = dirs.find_config_file("config.toml")
            .ok_or_else(|| Error::NotFound)?;
        Config::load_from(config_path)
    }

    fn load_from<P: Into<::std::path::PathBuf>>(path: P) -> Result<Config> {
        let path = path.into();
        let raw = Config::read_file(path.as_path())?;
        let config: Config = toml::from_str(&raw)?;
        Ok(config)
    }

    fn read_file<P: AsRef<::std::path::Path>>(path: P) -> Result<String> {
        let mut f = File::open(path)?;
        let mut contents = String::new();
        f.read_to_string(&mut contents)?;

        Ok(contents)
    }
}