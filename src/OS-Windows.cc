
/*
  OS-Windows.h -- Windows specific functions
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

#include <direct.h>
#include <windows.h>

#include <algorithm>

#include "Exception.h"

const std::string OS::path_separator = "\\";

const std::unordered_map<std::string, std::string> OS::standard_environment = {
};


static bool has_drive_letter(const std::string &path) {
    if (path.size() < 2) {
        return false;
    }
    
    if ((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z')) {
        return path[1] == ':';
    }
    else {
        return false;
    }
}


static std::string native_path(const std::string &path) {
    auto native_path = path;
    native_path.replace(native_path.begin(), native_path.end(), '/', '\\');
    return native_path;
}


static std::wstring utf8_to_utf16(const std::string &utf8) {
    int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.c_str(), -1, NULL, 0);
    if (size == 0) {
        throw Exception("invalid UTF-8 string", true);
    }
    wchar_t *utf16_c = (wchar_t *)malloc(size * sizeof(wchar_t));
    if (utf16_c == NULL) {
        throw Exception("out of memory");
    }

    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.c_str(), -1, utf16_c, size);

    auto utf16 =  std::wstring(utf16_c);
    free(utf16_c);
    return utf16;
}


static std::string utf16_to_utf8(const wchar_t *utf16) {
    int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, utf16, -1, NULL, 0, NULL, NULL);
    if (size == 0) {
        throw Exception("invalid UTF-16 string", true);
    }
    char *utf8_c = (char *)malloc(size);
    if (utf8_c == NULL) {
        throw Exception("out of memory");
    }
    
    WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, utf16, -1, utf8_c, size, NULL, NULL);
    
    auto utf8 =  std::string(utf8_c);
    free(utf8_c);
    return utf8;

}


std::string OS::append_path_component(const std::string &directory, const std::string &name) {
    if (directory.empty() || directory == ".") {
        return native_path(name);
    }
    else {
        // TODO: handle drive letter in name
        return native_path(directory) + path_separator + native_path(name);
    }
}


void OS::change_directory(const std::string &directory) {
    auto native_directory = native_path(directory);
    if (_chdir(native_directory.c_str()) < 0) {
        throw Exception("can't change into directory '" + native_directory + "'", true);
    }
}


void OS::create_directory(const std::string &directory) {
    auto native_directory = native_path(directory);
    auto w_native_directory = utf8_to_utf16(native_directory);

    if (!CreateDirectoryW(w_native_directory.c_str(), NULL)) {
        throw Exception("can't create directory '" + native_directory + "'", true);
    }
}

bool OS::directory_exists(const std::string &name) {
    auto w_file_name = utf8_to_utf16(native_path(name));
    DWORD attrs = GetFileAttributesW(w_file_name.c_str());
    if ((attrs != INVALID_FILE_ATTRIBUTES) && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
	return true;
    }
    return false;
}


bool OS::file_exists(const std::string &file_name) {
    auto w_file_name = utf8_to_utf16(native_path(file_name));
    DWORD attrs = GetFileAttributesW(w_file_name.c_str());
    if ((attrs != INVALID_FILE_ATTRIBUTES) && (attrs & FILE_ATTRIBUTE_NORMAL)) {
	// TODO: Is this the proper check for a file?
	return true;
    }
    return false;
}


std::string OS::get_error_string() {
    wchar_t error_string[8192];
    
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), error_string, (sizeof(error_string) / sizeof(wchar_t)), NULL);
    
    return utf16_to_utf8(error_string);
}


bool OS::is_absolute(const std::string &file_name) {
    if (file_name.empty()) {
        return false;
    }
    
    size_t offset = 0;
    
    if (has_drive_letter(file_name)) {
        if (file_name.size() < 3) {
            return false;
        }
        offset = 2;
    }
    return file_name[offset] == '/' || file_name[offset] == '\\';
}


std::vector<std::string> OS::list_files(const std::string &directory) {
    std::vector<std::string> files;
    auto pattern = directory + OS::path_separator + "*";
    WIN32_FIND_DATA directory_iterator;

    if (pattern.length() > MAX_PATH) {
	// directory path too long
	return files;
    }

    HANDLE dir_handle = FindFirstFile(pattern.c_str(), &directory_iterator);
    if (dir_handle == INVALID_HANDLE_VALUE) {
	// problem opening directory
	return files;
    }

    do {
	files.push_back(directory_iterator.cFileName);
    } while (FindNextFile(dir_handle, &directory_iterator) != 0);

    DWORD last_error = GetLastError();
    if (last_error != ERROR_NO_MORE_FILES) {
	// problem reading the whole file list
    }
    std::sort(files.begin(), files.end());
    
    return files;
}
    
std::string OS::make_temp_directory(const std::string &directory, const std::string &name) {
    // TODO: implement
    return "";
}


void OS::remove_directory(const std::string &directory) {
    // TODO: implement
}


std::string OS::run_command(const Command *command, std::vector<std::string> *output, std::vector<std::string> *error_output) {
    // TODO: implement
    return "0";
}


std::string OS::operating_system() {
    return "Windows";
}
