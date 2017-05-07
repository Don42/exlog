extern crate clap;
extern crate toml;
extern crate xdg;
#[macro_use]
extern crate serde_derive;
extern crate open;
extern crate chrono;


mod config;
mod exlog;

const PROGRAM_NAME: &'static str = "exlog";

const EXLOG_TEMPLATE: &'static str = include_str!("../exlog.template");


fn main() {
    use clap::{App, SubCommand, AppSettings};
    let matches = App::new(PROGRAM_NAME)
        .version("0.1.0")
        .author("Marco \"don\" Kaulea don4221@gmail.com")
        .about("Manage log/diary entries")
        .setting(AppSettings::SubcommandRequired)
        .subcommand(SubCommand::with_name("add")
                .about("Add log"))
        .subcommand(SubCommand::with_name("rm")
                .about("Remove log"))
        .subcommand(SubCommand::with_name("list")
                .about("List logs"))
        .get_matches();

    println!("{:?}", matches);
    let config = config::Config::load().unwrap_or_else(|err| {
        match err {
            config::Error::NotFound => {
                match config::write_defaults() {
                    Ok(path) => println!("Config file not found; writing default config {:?}", path),
                    Err(err) => println!("Failure to write default config: {}", err)
                }
                config::Config::load().unwrap()
            },
            _ => panic!("{}", err),
        }
    });
    if let Err(err) = run(matches, config) {
        panic!("{}", err)
    }
}

fn run(args: clap::ArgMatches, config: config::Config)
    -> std::result::Result<(), Box<std::error::Error>> {
    let timestamp = chrono::Local::now();
    println!("{}", timestamp);
    match args.subcommand_name() {
        Some("add") => add_exlog(),
        Some("rm") => panic!("RM not yet implemented"),
        Some("list") => panic!("LIST not yet implemented"),
        _ => panic!("Command does not exists"),
    }
}

fn add_exlog()
    -> std::result::Result<(), Box<std::error::Error>> {
    let content = get_user_entry()?;
    exlog::Exlog::new(content).store()
}

fn get_user_entry()
    -> std::result::Result<String, Box<std::error::Error>> {
    use std::io;
    use std::fs::File;
    use std::io::prelude::{Read, Write};

    let path = xdg::BaseDirectories::with_prefix(PROGRAM_NAME)
        .map_err(|err| io::Error::new(io::ErrorKind::NotFound, ::std::error::Error::description(&err)))
        .and_then(|p| p.place_cache_file("tmp.txt"))?;
    File::create(&path)?.write_all(EXLOG_TEMPLATE.as_bytes())?;
    open::that(&path)?;
    let mut content: String = String::new();
    File::open(&path)?.read_to_string(&mut content)?;

    std::fs::remove_file(&path)?;
    Ok(content)
}
