use ::chrono::{Local, datetime};

pub struct Exlog {
    content: String,
    project: Option<String>,
    location: Location,
    timestamp: datetime::DateTime<Local>,
}


pub enum Location {
    NoLocation,
    SimpleLocation(String),
}

impl Exlog {
    pub fn new(content: String) -> Exlog {
        Exlog {
            location: Location::NoLocation,
            content: content,
            project: None,
            timestamp: Local::now(),
        }
    }

    pub fn load_from_file() -> ::std::io::Result<Exlog> {
        Ok(Exlog {
            location: Location::SimpleLocation("Somewhere".to_string()),
            content: "".to_string(),
            project: None,
            timestamp: Local::now(),
        })
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

    pub fn store(&self) -> ::std::result::Result<(), Box<::std::error::Error>> {
        use std::io;
        use std::fs::File;
        use std::io::prelude::Write;
        let file_name: String = format!("{}.txt", self.timestamp.timestamp());
        let target_path = ::xdg::BaseDirectories::with_prefix(::PROGRAM_NAME).map_err(|err| {
                io::Error::new(io::ErrorKind::NotFound,
                               ::std::error::Error::description(&err))
            })
            .and_then(|p| p.place_data_file(file_name))?;
        let mut file = File::create(&target_path)?;

        if self.project.is_some() {
            let print = self.project.clone().unwrap();
            writeln!(file, "project: {}", print)?;
        }
        match self.location {
            Location::NoLocation => {}
            Location::SimpleLocation(ref loc) => writeln!(file, "location: {}", loc)?,
        }
        file.write_all(self.content.as_bytes())?;
        Ok(())
    }
}
