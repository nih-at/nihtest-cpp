/*
  OS.cc -- operating specific functions
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

#include "OS.h"

#include <string.h>

#include <fstream>

#include "Exception.h"


std::string OS::basename(const std::string &name) {
    auto pos = name.rfind(path_separator);
    
    if (pos == std::string::npos) {
        return name;
    }
    else {
        return name.substr(pos + 1);
    }
}


bool OS::compare_files(const std::string &left, const std::string &right) {
    auto left_file = std::ifstream(left);
    if (!left_file) {
        throw Exception("cannot open '" + left + "'", true);
    }

    auto right_file = std::ifstream(right);
    if (!right_file) {
        throw Exception("cannot open '" + right + "'", true);
    }
    
    while (!left_file.eof()) {
        char left_buf[8192], right_buf[8192];
        
        left_file.read(left_buf, sizeof(left_buf));
        if (left_file.bad()) {
            throw Exception("error reading from '" + left + "'", true);
        }
        
        right_file.read(right_buf, sizeof(right_buf));
        if (right_file.bad()) {
            throw Exception("error reading from '" + right + "'", true);
        }

        if (left_file.gcount() != right_file.gcount()) {
            return false;
        }
        
        if (memcmp(left_buf, right_buf, left_file.gcount()) != 0) {
            return false;
        }
    }
    
    if (!right_file.eof()) {
        return false;
    }
    
    return true;
}


void
OS::copy_file(const std::string &from, const std::string &to) {
    auto from_file = std::ifstream(from);
    if (!from_file) {
        throw Exception("cannot open '" + from + "'", true);
    }
    
    auto to_file = std::ofstream(to);
    if (!to_file) {
        throw Exception("cannot create '" + from + "'", true);
    }

    while (!from_file.eof()) {
        char buf[8192];
        
        from_file.read(buf, sizeof(buf));
        if (from_file.bad()) {
            throw Exception("error reading from '" + from + "'", true);
        }
        to_file.write(buf, from_file.gcount());
        if (to_file.bad()) {
            throw Exception("error writing to '" + from + "'", true);
        }
    }
}


std::string OS::dirname(const std::string &name) {
    auto pos = name.rfind(path_separator);
    
    if (pos == std::string::npos) {
        return ".";
    }
    else {
        return name.substr(0, pos);
    }
}


std::string OS::extension(const std::string &file_name) {
    auto pos = file_name.rfind(".");
    if (pos == std::string::npos) {
        return "";
    }
    else {
        return file_name.substr(pos + 1);
    }
}
