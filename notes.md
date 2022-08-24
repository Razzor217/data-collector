# Notes for data collection


* [Trigger detection](#trigger-detection)
    * [Specification of directory to monitor](#specification-of-directory-to-monitor)
    * [Detection of file creation](#detection-of-file-creation)
    * [Matching file name](#matching-file-name)
* [Data collection and storage](#data-collection-and-storage)
    * [Files and directories](#files-and-directories)
    * [Disk usage information](#disk-usage-information)
* [Program structure](#program-structure)
* [Testing](#testing)
    * [Unit testing](#unit-testing)
    * [Component testing](#component-testing)


## Trigger detection

### Specification of directory to monitor

Specified as first command line parameter.

### Detection of file creation

Assumptions:

* File creation is only monitored in the specified directory,
**not** recursively in subdirectories

Different options:

1. Detect file creation system call, i.e. with `ptrace()`.
**Disadvantage**: `pid` of caller has to be known beforehand
(in case of `ptrace`).
Otherwise complicated.
2. Periodically check the target directory for new files, i.e. check file
creation time stamp (e.g. with `stat`) and compare it with the newest observed
time stamp.
**Disadvantage**: Performance issues when iterating over large directories
3. Use the C interface of `inotify`
(see [man pages](https://man7.org/linux/man-pages/man7/inotify.7.html "man7.org > linux > man-pages > inotify"))
to add a directory to a watch list, then periodically read from that list and
add events to an event buffer. Check whether the event matches file creation

### Matching file name

* File name is matched against a `std::regex`, using `std::regex_match()`
* Used regex:
    * Start with `core`
    * Then match a string of letters
    * Then match sets of hexadecimal numbers
    * End with `lz4`
    * All aforementioned groups are separated by dots
    * Matching against anything after `core` and before the hex groups,
    as well as matching against anything after the hex groups and before `lz4`
    seems unintentional, therefore it is disregarded


## Data collection and storage

Assumptions:

* Upon detection of a trigger event of a file named `file` within the monitored
directory `parent`, **all** data in `parent` is scanned for data collection
* Output directory specified as second command line parameter

Variants:

* Upon detection of a trigger event of a file named `file` within the monitored
directory `parent`, only data in `file` (and possibly contained subdirectories)
is scanned for data collection

Method:

* Modular approach in order to support the collection of other forms of data
* Upon detection of a trigger event
    1. Collect selected data
   (see [files and directories](#files-and-directories) and [disk usage information](#disk-usage-information))
    2. Collect file names to archive in a `std::vector`
    3. Create unique hash using `std::hash` depending on file name of event
    trigger
    4. Create archive by invoking `tar` with `std::system`, append archive name
   with hash

### Files and directories

Collect:

* Files only
* Directories and their contents, recursively

Method:

* Selection of collected data via command line parameter, e.g. `-f, -d`
* Traverse `parent` directory recursively using
`std::filesystem::recursive_directory_iterator`
* Depending on selection, push file names onto a `std::vector`

### Disk usage information

* Invoke `du -sh` using `std::system` on each selected file
* Append disk usage of each selected file to a single file within `parent`
* Push disk usage file name onto the aforementioned `std::vector`


## Program structure

Multithreading:

1. One thread monitors the specified directory
    * Continuously `read()` from the watched directory
    * Match file names, create event on match
    * Push event onto a **concurrent queue**
2. Second thread handles incoming events
    * Pop events from a **concurrent queue**
    * Collect data from watched directory
    * Store `tar` archive in output directory
3. Main thread
    * Start monitoring and event handling
    * Request stop with a `std::atomic<bool>`

Classes:

* Controller
* Concurrent queue
* Helper classes for monitoring, matching, data collection, data storage


## Testing

### Unit testing

* File name matching
* Collection of selected file names

Note: Unit testing of the concurrent queue is omitted as it was tested in
previous projects.

### Component testing

* Event trigger on matching file creation
* `tar` archive creation for given directory
* Complete program
