/*
  OS.h -- operating specific functions
  Copyright (C) 2020 Dieter Baron and Thomas Klausner

  This file is part of nihtest, regression tests for command line utilities.
  The authors can be contacted at <nihtest@nih.at>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
  3. The names of the authors may not be used to endorse or promote
     products derived from this software without specific prior
     written permission.

  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HAD_OS_H
#define HAD_OS_H

#include <string>
#include <unordered_map>
#include <vector>

class OS {
public:
    struct Command {
        Command() : input(NULL), limits(NULL) { }
        
        // The command line arguments, not including the program itself (argv[0]).
        std::vector<std::string> arguments;
        
        // Environment variables to set in sub process.
        std::vector<const std::unordered_map<std::string, std::string> *> environments;
        
        // Lines to feed program on standard input.
        std::vector<std::string> *input;
        
        // File to redirect standard input from.
        std::string input_file;
        
        // Limits to set, currently not used.
        std::unordered_map<char, int> *limits;
        
        // List of directories in which to search for program.
        std::vector<std::string> path;
        
        // Preload shared library (not used on Windows).
        std::string preload_library;
        
        // Name of the program. This is used to search the executable and also as argv[0].
        std::string program;
    };

    // Character used to separate path components.
    static const std::string path_separator;
    
    // Environment variables to set for standard environment (e. g. time zone, language).
    static const std::unordered_map<std::string, std::string> standard_environment;

    // Append `name` to `directory`, converting to use platform native path separator.
    static std::string append_path_component(const std::string &directory, const std::string &name);
    
    // Return last path component.
    static std::string basename(const std::string &name);
    
    // Change the working directory to `directory`.
    static void change_directory(const std::string &directory);
    
    // Get all but last path components.
    static std::string dirname(const std::string &name);
    
    // Copy file `from` to file `to`, creating intermediary directories if neccessary.
    static void copy_file(const std::string &from, const std::string &to);
    
    // Compare files `from` and `to`, returning true if they have identical contents.
    static bool compare_files(const std::string &from, const std::string &to);
    
    // Get file name extension.
    static std::string extension(const std::string &name);
    
    // Check whether `file` exists.
    static bool file_exists(const std::string &file);
    
    // Get string describing last system error.
    static std::string get_error_string();
    
    // Return a list of files in `directory` and its subdirectories, sorted alphabetically.
    static std::vector<std::string> list_files(const std::string &directory);

    // Check whether `name` is an absolute path name.
    static bool is_absolute(const std::string &name);
    
    // Make unique temporary directory in `directory`, using `name` as part of its name.
    static std::string make_temp_directory(const std::string &directory, const std::string &name);
    
    // Recursively remove `directory`.
    static void remove_directory(const std::string &directory);
    
    // Run command described by `command`, returning lines from standard output in `output` and error output  in `error_output`.
    static std::string run_command(const Command *command, std::vector<std::string> *output, std::vector<std::string> *error_output);
    
    // Get name of the operating system.
    static std::string operating_system();
};

#endif // HAD_OS_H
