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
    static const std::string path_separator;

    static std::string append_path_component(const std::string &directory, const std::string &name);
    static std::string basename(const std::string &name);
    static void change_directory(const std::string &directory);
    static void copy_file(const std::string &from, const std::string &to);
    static bool file_exists(const std::string &file_name);
    static std::string get_error_string();
    static bool is_absolute(const std::string &file_name);
    static std::string make_temp_directory(const std::string &directory, const std::string &name);
    static void remove_directory(const std::string &directory);
    static int run_command(const std::string &program, const std::vector<std::string> &arguments, const std::unordered_map<std::string, std::string> &environment, const std::vector<std::string> &input, std::vector<std::string> *output, std::vector<std::string> *error_output);
};

#endif // HAD_OS_H
