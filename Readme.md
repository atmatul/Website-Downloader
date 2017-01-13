### Website Downloader

This project contains two modules, a `fetcher` to download the entire 
content of a specified website and a `server` to provide search results
 based on the keywords extracted from the downloaded website.
 
 #### Requirements
 This project uses a `cmake` based build system. Hence to build it in the 
 first place `cmake >=3.6`  is required. Additionally it also depends on 
 several external libraries which are necessary for the project to work.
 The required libraries are as follows:
 
* `openssl` - To fetch websites from `https` hosts
* `mysql` - To store fetched and processed links
* `threads` - For multi-threading during website download

#### Building
 To build this project execute:
 ```bash
cmake CMakelists.txt   
cd bin
make
 ```
 The necessary files will be generated in a `./bin` directory from where
  make can be called building the executables.
  
  #### Configuration
  The configuration can be provided in `config.ini`. Different config files
  can be used at the same time as long as they are passed on to the program
  as arguments. If nothing specified, program will take `config.ini` as the
  default configuration.
  
  #### Execution
  Both the executables can be called as follows:
  ```bash
./bin/fetcher [config_file]
./bin/server [config_file]
```
#### About
This repository was developed by Kunal Pal and Thorsten Born as part of their
Network Programming lab project.