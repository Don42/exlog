extern crate clap;
extern crate toml;
extern crate xdg;
#[macro_use]
extern crate serde_derive;
extern crate open;
extern crate chrono;

#[macro_use]
extern crate slog;
extern crate slog_term;
extern crate slog_async;

use slog::Drain;

use std::io::prelude::Write;

mod config;
mod exlog;

const PROGRAM_NAME: &'static str = "exlog";

const EXLOG_TEMPLATE: &'static str = include_str!("../exlog.template");


fn main() {
    use clap::{App, SubCommand, AppSettings, Arg};
    let matches = App::new(PROGRAM_NAME)
        .version("0.1.0")
        .author("Marco \"don\" Kaulea don@0xbeef.org")
        .about("Manage log/diary entries")
        .setting(AppSettings::SubcommandRequired)
        .subcommand(SubCommand::with_name("add").about("Add log"))
        .subcommand(SubCommand::with_name("rm").about("Remove log"))
        .subcommand(SubCommand::with_name("list").about("List logs"))
        .subcommand(SubCommand::with_name("show")
            .about("Show log")
            .arg(Arg::with_name("log_name").required(true)))
        .get_matches();

    let decorator = slog_term::TermDecorator::new().build();
    let drain = slog_term::FullFormat::new(decorator).use_custom_timestamp(time_gen).build().fuse();
    let drain = slog_async::Async::new(drain).build().fuse();
    let log = slog::Logger::root(drain, o!("version" => env!("CARGO_PKG_VERSION")));
    debug!(log, "Startup");

    println!("{:?}", matches);
    let config = config::Config::load().unwrap_or_else(|err| {
        match err {
            config::Error::NotFound => {
                match config::write_defaults() {
                    Ok(path) => {
                        println!("Config file not found; writing default config {:?}", path)
                    }
                    Err(err) => println!("Failure to write default config: {}", err),
                }
                config::Config::load().unwrap()
            }
            _ => panic!("{}", err),
        }
    });

    if let Err(err) = run(matches, config) {
        panic!("{}", err)
    }
}

fn run(args: clap::ArgMatches,
       _config: config::Config)
       -> std::result::Result<(), Box<std::error::Error>> {
    match args.subcommand_name() {
        Some("add") => add_exlog(),
        Some("rm") => unimplemented!(),
        Some("list") => list_exlog(),
        Some("show") => show_exlog(),
        _ => panic!("Command does not exists"),
    }
}

fn add_exlog() -> std::result::Result<(), Box<std::error::Error>> {
    let content = get_user_input(String::from(EXLOG_TEMPLATE))?;
    exlog::Exlog::new(content).store()
}

fn show_exlog() -> std::result::Result<(), Box<std::error::Error>> {
    let _ = exlog::Exlog::load_from_file()?;
    Ok(())
}

fn list_exlog() -> std::result::Result<(), Box<std::error::Error>> {
    use std::io;
    use std::path::PathBuf;

    let path = xdg::BaseDirectories::with_prefix(PROGRAM_NAME).map_err(|err| {
            io::Error::new(io::ErrorKind::NotFound,
                           ::std::error::Error::description(&err))
        })?;
    let files = path.list_data_files(PathBuf::from(""));
    for file in files.iter() {
        if file.file_name().is_none() {
            continue;
        }
        println!("{}", file.file_name().unwrap().to_string_lossy());
    }
    Ok(())
}

fn get_user_input(template: String) -> std::result::Result<String, Box<std::error::Error>> {
    use std::io;
    use std::fs::File;
    use std::io::prelude::{Read, Write};

    let path = xdg::BaseDirectories::with_prefix(PROGRAM_NAME).map_err(|err| {
            io::Error::new(io::ErrorKind::NotFound,
                           ::std::error::Error::description(&err))
        })
        .and_then(|p| p.place_cache_file("tmp.txt"))?;
    File::create(&path)?.write_all(template.as_bytes())?;
    open::that(&path)?;
    let mut content: String = String::new();
    File::open(&path)?.read_to_string(&mut content)?;

    std::fs::remove_file(&path)?;
    Ok(content)
}

fn time_gen(write: &mut Write) -> std::io::Result<()> {
    let utc = chrono::UTC::now();
    write!(write, "{}", utc)?;
    Ok(())
}

#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {
    }

    #[test]
    fn break_the_build() {
        panic!("This verifies that the tests breaks.")
    }
}
