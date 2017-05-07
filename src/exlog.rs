use ::chrono::{Local, datetime};

pub struct Exlog {
    content: String,
    project: Option<String>,
    location: Location,
    timestamp: datetime::DateTime<Local>,
}


enum Location {
    NoLocation,
    SimpleLocation(String),
}

impl Exlog {
    pub fn new(content: String) -> Exlog{
        Exlog {
            location: Location::NoLocation,
            content: content,
            project: None,
            timestamp: Local::now(),
        }
    }

    pub fn project(&mut self, project: String) {
        self.project = Some(project);
    }

    pub fn location(&mut self, location: Location) {
        self.location = location;
    }

    pub fn timestamp(&mut self, timestamp: datetime::DateTime<Local>) {
        self.timestamp = timestamp;
    }
}


pub fn write_exlog(exlog: Exlog) -> ::std::result::Result<(), Box<::std::error::Error>> {
    use std::io;
    use std::fs::File;
    use std::io::prelude::Write;
    let target_path = ::xdg::BaseDirectories::with_prefix(::PROGRAM_NAME)
        .map_err(|err| io::Error::new(io::ErrorKind::NotFound, ::std::error::Error::description(&err)))
        .and_then(|p| p.place_data_file("tmp.txt"))?;
    File::create(&target_path)?.write_all(exlog.content.as_bytes())?;
    Ok(())
}
