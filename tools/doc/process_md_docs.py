#!/usr/bin/env python3

import os
import sys


def parse_header_line(header_line):
    level = len(header_line.split(" ")[0])
    title = header_line[level + 1 :].rstrip()
    anchor_title = title.lower().replace(" ", "-")
    ignored_chars = [":", "+", ",", "!", '"', "(", ")", "?", "*", "`"]
    for ignored_char in ignored_chars:
        anchor_title = anchor_title.replace(ignored_char, "")
    return level, title, anchor_title


def repeat(s, nb):
    r = ""
    for i in range(nb):
        r = r + s
    return r


def is_header_line(line):
    return line.startswith("#") and not (line.startswith("#include"))


def remove_code_blocks(lines):
    filtered_lines = []
    in_code_block = False
    for line in lines:
        if line.strip().startswith("```"):
            in_code_block = not in_code_block
        if not in_code_block:
            filtered_lines.append(line)
    return filtered_lines


def make_toc(file):
    with open(file, "r", newline='') as f:
        lines = f.readlines()
    lines = remove_code_blocks(lines)

    header_lines = [line[:-1] for line in lines if is_header_line(line)]
    toc = '<span id="TOC"/></span>\n\n'
    for header_line in header_lines:
        level, title, anchor_title = parse_header_line(header_line)
        toc = toc + "{}* [{}]({})\n".format(repeat("  ", level - 1), title, "#" + anchor_title)
    return toc


LANGUAGES = {"cpp": "cpp", "cc": "cpp", "cxx": "cpp", "h": "cpp", "hpp": "cpp", "hh": "cpp", "py": "python"}


def comment_prefix(line):
    """The comment prefix of a marker line: "//", "#", or "" (inside a block comment)"""
    t = line.strip()
    if t.startswith("//"):
        return "//"
    if t.startswith("#"):
        return "#"
    return ""


def marker_name(line):
    """The name of an opening marker (@@md#Name, after an optional comment prefix), or None"""
    t = line.strip()
    t = t[len(comment_prefix(line)):].strip()
    if not t.startswith("@@md#"):
        return None
    return t[5:].strip() or None


def is_md_block_end(line):
    t = line.strip()
    return t[len(comment_prefix(line)):].strip() == "@@/md"


def dedent(lines):
    """Removes the common leading whitespace (blank lines ignored)"""
    indents = [len(l) - len(l.lstrip()) for l in lines if l.strip()]
    if not indents or min(indents) == 0:
        return lines
    n = min(indents)
    return [l[n:] if l.strip() else l.rstrip("\r\n") + "\n" for l in lines]


def trim_blank_lines(lines):
    while lines and not lines[0].strip():
        lines = lines[1:]
    while lines and not lines[-1].strip():
        lines = lines[:-1]
    return lines


def extract_sections(file):
    """The sections of a source file: name -> (raw block lines, prose lines, code lines).
    A section is a prose block (@@md#Name ... @@/md; prose = the block with the comment prefix and the
    common indentation removed, raw = the block's lines as they are) and the code that follows it: up to
    the next top-level item (a blank line, then a line at column 0) or the next marker.
    Same rules as ImGuiMd::ResolveImports (imgui_bundle)."""
    with open(file, "r", newline='') as f:
        lines = f.readlines()
    sections = {}
    order = []
    i = 0
    while i < len(lines):
        name = marker_name(lines[i])
        if name is None:
            i += 1
            continue
        prefix = comment_prefix(lines[i])
        prose = []
        raw = []
        j = i + 1
        while j < len(lines) and not is_md_block_end(lines[j]):
            line = lines[j]
            raw.append(line)
            if prefix and line.lstrip().startswith(prefix):
                line = line.lstrip()[len(prefix):]
            prose.append(line)
            j += 1
        if j >= len(lines):
            sys.exit(f"{file}: block '{name}' is not closed (missing @@/md)")
        prose = trim_blank_lines(dedent(prose))
        code = []
        k = j + 1
        while k < len(lines) and marker_name(lines[k]) is None:
            starts_top_level_item = code and not lines[k - 1].strip() and lines[k].strip() and lines[k][0] not in " \t"
            if starts_top_level_item:
                break
            code.append(lines[k])
            k += 1
        while k < len(lines) and marker_name(lines[k]) is None:
            k += 1
        sections[name] = (raw, prose, trim_blank_lines(code))
        order.append(name)
        i = k
    return sections, order


def parse_import_line(line):
    """
    @import "app_window_params.h" {md_id=DockingParams, part=prose, dedent=false}
    Returns (file, attributes dict). The file may be empty (an import of the current file).
    """
    t = line.strip()[len("@import"):].strip()
    file = ""
    if t.startswith('"'):
        close = t.index('"', 1)
        file = t[1:close]
        t = t[close + 1:].strip()
    attrs = {}
    if t:
        if not (t.startswith("{") and t.endswith("}")):
            sys.exit(f"bad directive: {line.strip()}")
        for item in t[1:-1].split(","):
            if item.strip():
                key, _, value = item.partition("=")
                attrs[key.strip()] = value.strip()
    for key in attrs:
        if key not in ("md_id", "part", "dedent"):
            sys.exit(f"unknown attribute '{key}': {line.strip()}")
    return file, attrs


def render_section(raw, prose, code, attrs, language):
    """Without `part`: the block's raw lines, as the book has always used them (also inside code fences).
    part=prose: the prose (comment prefix and indentation removed); part=code: the code after the block;
    part=both: prose then code, as ImGuiMd::ResolveImports does by default."""
    if "part" not in attrs:
        return "".join(raw)
    part = attrs["part"]
    if part not in ("prose", "code", "both"):
        sys.exit(f"part must be prose, code or both (got '{part}')")
    result = ""
    if part != "code":
        result += "".join(prose) + "\n"
    if part != "prose" and code:
        if attrs.get("dedent", "true") != "false":
            code = dedent(code)
        result += f"```{language}\n" + "".join(code) + "```\n\n"
    return result


def resolve_import(line, current_file):
    """The markdown that replaces an @import line (see ImGuiMd::ResolveImports for the rules)"""
    file, attrs = parse_import_line(line)
    file = os.path.join(os.path.dirname(current_file), file) if file else current_file
    if not os.path.isfile(file):
        sys.exit(f"{current_file}: file not found: {file} ({line.strip()})")
    sections, order = extract_sections(file)
    language = LANGUAGES.get(os.path.splitext(file)[1][1:], "")
    md_id = attrs.get("md_id")
    if md_id is not None:
        if md_id not in sections:
            sys.exit(f"{current_file}: no block '{md_id}' in {file}")
        return render_section(*sections[md_id], attrs, language)
    return "".join(render_section(*sections[name], attrs, language) for name in order)


def process_md_file(input_file, output_file):
    with open(input_file, "r", newline='') as f:
        lines = f.readlines()

    src_basename = os.path.basename(input_file)
    content = f"<!-- This file is autogenerated from {src_basename} by tools/doc/process_md_docs.py -->\n"
    for line in lines:
        if line.startswith("@import"):
            content = content + resolve_import(line, input_file)
        elif line.startswith("[TOC]"):
            content = content + make_toc(input_file)
        elif is_header_line(line):
            content = content + line
        else:
            content = content + line

    with open(output_file, "w", newline='') as f:
        f.write(content)


def process_main_readme(repo_dir: str):
    with open(repo_dir + "README.src.md", "r", newline='') as f:
        readme_src_content = f.read()
    with open(repo_dir + "/docs_src/intro.md", "r", newline='') as f:
        intro_content = f.read()

    readme_content = readme_src_content.replace("<!-- INTRO -->", intro_content)

    with open(repo_dir + "README.md", "w", newline='') as f:
        f.write(readme_content)


if __name__ == "__main__":
    this_dir = os.path.dirname(os.path.realpath(__file__)) + "/"
    repo_dir = this_dir + "/../../"
    hello_imgui_dir = repo_dir + "/src/hello_imgui/"

    process_main_readme(repo_dir)

    process_md_file(hello_imgui_dir + "doc_params.src.md", hello_imgui_dir + "doc_params.md")
    process_md_file(hello_imgui_dir + "doc_api.src.md", hello_imgui_dir + "doc_api.md")
