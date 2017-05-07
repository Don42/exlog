extern crate clap;
extern crate toml;
extern crate xdg;
#[macro_use]
extern crate serde_derive;

mod config;

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
}
