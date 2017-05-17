
use std::io;
use std::fs::File;
use std::io::prelude::{Read, Write};
use std::convert::From;
use xdg;
use toml;

static DEFAULT_EXLOG_CONFIG: &'static str = include_str!("../exlog.toml");

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

impl ::std::error::Error for Error {
    fn cause(&self) -> Option<&::std::error::Error> {
        match *self {
            Error::NotFound => None,
            Error::XDG(ref err) => Some(err),
            Error::IO(ref err) => Some(err),
            Error::TOML(ref err) => Some(err),
        }
    }

    fn description(&self) -> &str {
        match *self {
            Error::NotFound => "config file not found",
            Error::XDG(ref err) => err.description(),
            Error::IO(ref err) => err.description(),
            Error::TOML(ref err) => err.description(),
        }
    }
}

impl ::std::fmt::Display for Error {
    fn fmt(&self, f: &mut ::std::fmt::Formatter) -> ::std::fmt::Result {
        match *self {
            Error::TOML(ref err) => write!(f, "error during config parsing: {}", err),
            Error::IO(ref err) => write!(f, "error during read operation: {}", err),
            Error::XDG(ref err) => write!(f, "error reading xdg folders: {}", err),
            Error::NotFound => write!(f, "{}", ::std::error::Error::description(self)),
        }
    }
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
        let dirs = xdg::BaseDirectories::with_prefix(::PROGRAM_NAME)?;

        let config_path = dirs.find_config_file("exlog.toml")
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

pub fn write_defaults() -> io::Result<::std::path::PathBuf> {
    let path = xdg::BaseDirectories::with_prefix(::PROGRAM_NAME).map_err(|err| {
            io::Error::new(io::ErrorKind::NotFound,
                           ::std::error::Error::description(&err))
        })
        .and_then(|p| p.place_config_file("exlog.toml"))?;
    File::create(&path)?.write_all(DEFAULT_EXLOG_CONFIG.as_bytes())?;
    Ok(path)
}
