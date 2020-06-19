#!/usr/bin/env python3

import os
import sys


def parse_header_line(header_line):
    level = len(header_line.split(' ')[0])
    title = header_line[level + 1:]
    anchor_title = title.lower().replace(" ", "-")
    ignored_chars = [":", "+", ",", "!", "\"", "(", ")"]
    for ignored_char in ignored_chars:
        anchor_title = anchor_title.replace(ignored_char, "")
    return level, title, anchor_title


def repeat(s, nb):
    r = ""
    for i in range(nb):
        r = r + s
    return r


def toc_link():
    toc_image = "docs/toc.png"
    # <div style="text-align: right"> your-text-here </div>
    image_link = f"[![TOC]({toc_image})](#TOC)"
    r = f" <div style=\"text-align: right\">{image_link}</div>"
    return r


def is_header_line(line):
    return line.startswith("#") and not (line.startswith("#include"))

def make_toc(file):
    with open(file, "r") as f:
        lines = f.readlines()
    header_lines = [line[:-1] for line in lines if is_header_line(line)]
    toc =  "<span id=\"TOC\"/></span>\n\n"
    for header_line in header_lines:
        level, title, anchor_title = parse_header_line(header_line)
        toc = toc + "{}* [{}]({})\n".format( 
            repeat("  ", level - 1),  
            title,
            "#" + anchor_title
            )
    return toc


def extract_lines_between(file, line_begin, line_end):
    with open(file, "r") as f:
        lines = f.readlines()
    result = ""
    for i, line in enumerate(lines):
        if i >= line_begin and i < line_end:
            result = result + line
    return result


def parse_import_line(line):
    """
    @import "app_window_params.h" {line_begin=8 line_end=22 code_block=false}
    """
    def get_word_after(s, after_what):
        end = s[s.index(after_what) + len(after_what):]
        word = end.split(" ")[0]
        return word

    imported_file = get_word_after(line, '@import "').replace('"', '')
    line_begin = int(get_word_after(line, "line_begin="))
    line_end = int(get_word_after(line, "line_end="))
    return imported_file, line_begin, line_end


def process_md_file(input_file, output_file):
    with open(input_file, "r") as f:
        lines = f.readlines()

    content = ""
    for line in lines:
        if line.startswith("@import"):
            imported_file, line_begin, line_end = parse_import_line(line)
            imported_file = os.path.dirname(input_file) + "/" + imported_file
            content = content + extract_lines_between(imported_file, line_begin, line_end)
        elif line.startswith("[TOC]"):
            content = content + make_toc(input_file)
        elif is_header_line(line):
            content = content + line[:-1] + toc_link() + "\n"
        else:
            content = content + line

    with open(output_file, "w") as f:
        f.write(content)


if __name__ == "__main__":
    this_dir = os.path.dirname(os.path.realpath(__file__)) + "/"
    repo_dir = this_dir + "/../"
    hello_imgui_dir = repo_dir + "/src/hello_imgui/"

    process_md_file(repo_dir + "README.src.md", repo_dir + "README.md")
    process_md_file(hello_imgui_dir + "hello_imgui_api.src.md", hello_imgui_dir + "hello_imgui_api.md")
