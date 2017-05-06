extern crate clap;
extern crate toml;
extern crate xdg;
#[macro_use]
extern crate serde_derive;

mod config;

fn initialization(args: clap::ArgMatches) {

}


fn main() {
    use clap::{Arg, App, SubCommand, AppSettings};
    let matches = App::new("exlog")
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
    let config = config::Config::load();
    initialization(matches);
    println!("Config Path: {:?}", config)
}
