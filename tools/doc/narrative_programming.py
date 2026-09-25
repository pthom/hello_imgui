"""Narrative programming: the annotations of source files (::md, ::code) and the transclusions (![[file#name]]).

Specification: https://github.com/pthom/imgui_rich_md/blob/main/docs/narrative_programming/narrative_programming_spec.md
A port of imgui_rich_md's narrative_programming.cpp, checked by the same cases (imgui_rich_md/tests/narrative/cases).
"""

from dataclasses import dataclass, field
from typing import Callable, Dict, List, Optional, Tuple

ReadTextFile = Callable[[str], Optional[str]]

MAX_DEPTH = 8
_WHITESPACE = " \t\r"


def _trim(s: str) -> str:
    return s.strip(_WHITESPACE)


def _is_blank(s: str) -> bool:
    return not _trim(s)


def split_lines(text: str) -> List[str]:
    lines = text.split("\n")
    if text.endswith("\n"):
        lines.pop()
    return [line[:-1] if line.endswith("\r") else line for line in lines]


def _join_lines(lines: List[str]) -> str:
    return "".join(line + "\n" for line in lines)


def _trim_blank_lines(lines: List[str]) -> List[str]:
    begin, end = 0, len(lines)
    while end > begin and _is_blank(lines[end - 1]):
        end -= 1
    while begin < end and _is_blank(lines[begin]):
        begin += 1
    return lines[begin:end]


def _dedent(lines: List[str]) -> List[str]:
    """Removes the longest common leading whitespace of the non-blank lines, compared char by char
    (a tab never matches a space). Blank lines become empty."""
    common: Optional[str] = None
    for line in lines:
        if _is_blank(line):
            continue
        indent = line[: len(line) - len(line.lstrip(" \t"))]
        if common is None:
            common = indent
        n = 0
        while n < len(common) and n < len(indent) and common[n] == indent[n]:
            n += 1
        common = common[:n]
    n = len(common or "")
    return ["" if _is_blank(line) else line[n:] for line in lines]


# Files
# -----

_FILE_KINDS = {
    "py": ("python", "python"), "pyi": ("python", "python"),
    "c": ("clike", "c"), "h": ("clike", "cpp"), "cpp": ("clike", "cpp"), "cc": ("clike", "cpp"),
    "cxx": ("clike", "cpp"), "hpp": ("clike", "cpp"), "hh": ("clike", "cpp"), "glsl": ("clike", "glsl"),
    "vert": ("clike", "glsl"), "frag": ("clike", "glsl"), "js": ("clike", "javascript"), "ts": ("clike", "typescript"),
    "md": ("markdown", ""), "markdown": ("markdown", ""),
    "json": ("other", "json"), "cmake": ("other", "cmake"), "txt": ("other", ""),
}


def _kind_of_file(path: str) -> Tuple[str, str]:
    """(syntax, language of its code blocks)"""
    dot, slash = path.rfind("."), max(path.rfind("/"), path.rfind("\\"))
    ext = "" if dot < 0 or dot < slash else path[dot + 1:].lower()
    return _FILE_KINDS.get(ext, ("other", ext))


def _directory_of(path: str) -> str:
    slash = max(path.rfind("/"), path.rfind("\\"))
    return "" if slash < 0 else path[: slash + 1]


def _is_absolute(path: str) -> bool:
    return path[:1] in ("/", "\\") or (len(path) > 1 and path[1] == ":")


def _normalize_path(path: str) -> str:
    """"a/b/../c/./d" -> "a/c/d", lexically (a leading ".." stays); the separators become '/'"""
    path = path.replace("\\", "/")
    parts: List[str] = []
    for part in path.split("/"):
        if part in ("", "."):
            continue
        if part == ".." and parts and parts[-1] != "..":
            parts.pop()
        else:
            parts.append(part)
    return ("/" if path.startswith("/") else "") + "/".join(parts)


# Annotations
# -----------

_DIRECTIVES = ("::md", "::code", "::endcode", "::endmd")


def _parse_directive(text: str) -> Tuple[Optional[str], str]:
    """`text` is a line without its comment token or its container opener: a directive is its first token.
    Returns (directive, name), or (None, "")."""
    t = _trim(text)
    for keyword in _DIRECTIVES:
        n = len(keyword)
        if t.startswith(keyword) and (len(t) == n or t[n] in " \t"):
            return keyword, _trim(t[n:])
    return None, ""


def _line_comment_text(line: str, syntax: str) -> Optional[str]:
    """The text after the line comment token ("#" or "//"), when the line is a line comment"""
    token = {"python": "#", "clike": "//"}.get(syntax)
    stripped = line.lstrip(" \t")
    if not token or not stripped.startswith(token):
        return None
    return stripped[len(token):]


def _container_opener(line: str, syntax: str) -> Optional[Tuple[str, str]]:
    """A string or block comment that starts the line (after its indentation): (its closing delimiter, the text
    after the opener). Python: \"\"\" or ''', with an optional prefix such as r. C-like: /* or /**."""
    stripped = line.lstrip(" \t")
    if syntax == "clike" and stripped.startswith("/*"):
        return "*/", stripped[3:] if stripped.startswith("/**") else stripped[2:]
    if syntax == "python":
        q = 0
        while q < len(stripped) and q < 2 and stripped[q] in "rRbBuUfF":
            q += 1
        for quotes in ('"""', "'''"):
            if stripped.startswith(quotes, q):
                return quotes, stripped[q + 3:]
    return None


@dataclass
class _MdSection:
    name: str
    line: int  # of its ::md (0-based)
    prose: List[str] = field(default_factory=list)
    code: int = -1  # its associated code region


@dataclass
class _CodeRegion:
    name: str  # empty: the associated code of a section
    section: int  # the section it belongs to, for an associated code
    line: int  # of its ::code
    begin: int  # its source lines: [begin, end)
    end: int = 0


@dataclass
class _ParsedFile:
    lines: List[str]
    directive_only: List[bool]  # the lines removed from the extracted code
    sections: List[_MdSection] = field(default_factory=list)
    regions: List[_CodeRegion] = field(default_factory=list)
    error: str = ""  # the first annotation error: "file:line: message"


class _AnnotationError(Exception):
    pass


class _Parser:
    """Reads the annotations of a source file, line by line. Stops at the first error."""

    def __init__(self, file: str, lines: List[str], syntax: str) -> None:
        self.file = file
        self.syntax = syntax
        self.r = _ParsedFile(lines, [False] * len(lines))
        self.open_regions: List[int] = []  # innermost last
        self.prose_section = -1  # a section whose prose (in line comments) is open
        self.prose: List[str] = []  # its lines, comment tokens removed
        self.redundant_endmd = -1  # a section closed by the ::endcode just above: an ::endmd may follow

    def fail(self, line: int, message: str) -> None:
        raise _AnnotationError(f"{self.file}:{line + 1}: {message}")

    def check_name(self, line: int, name: str) -> None:
        if any(c in name for c in "#|[]"):
            self.fail(line, f"'{name}': the characters # | [ ] are not allowed in names")

    def new_section(self, line: int, name: str) -> None:
        if not name:
            self.fail(line, "::md needs a name")
        self.check_name(line, name)
        self.r.sections.append(_MdSection(name, line))

    def set_prose(self, section: _MdSection, lines: List[str]) -> None:
        section.prose = _trim_blank_lines(_dedent(lines))

    def open_region(self, line: int, name: str, section: int, begin: int) -> None:
        """`line` is the line of the ::code; the region starts at `begin`"""
        if not name and section < 0:
            self.fail(line, "an unnamed ::code must follow the prose of a ::md section: name this region")
        self.check_name(line, name)
        self.r.regions.append(_CodeRegion(name, section, line, begin))
        if section >= 0:
            self.r.sections[section].code = len(self.r.regions) - 1
        self.open_regions.append(len(self.r.regions) - 1)

    def close_region(self, line: int, name: str) -> None:
        if not self.open_regions:
            self.fail(line, "::endcode without an open ::code")
        region = self.r.regions[self.open_regions[-1]]
        if name and not region.name:
            self.fail(line, f"the code of ::md {self.r.sections[region.section].name} closes with an unnamed ::endcode")
        if name and name != region.name:
            self.fail(line, f"::endcode {name} closes ::code {region.name}")
        region.end = line
        self.open_regions.pop()
        if region.section >= 0:
            self.redundant_endmd = region.section

    def container_section(self, i: int, closer: str, after_opener: str) -> int:
        """A section in a string or block comment: its prose goes up to the end of the container, or up to ::code,
        whose region starts after the container. Returns the line of the closing delimiter."""
        close = after_opener.find(closer)
        name = _parse_directive(after_opener if close < 0 else after_opener[:close])[1]
        self.new_section(i, name)
        if close >= 0:
            return i  # opened and closed on one line: an empty section
        section = len(self.r.sections) - 1
        lines: List[str] = []
        code_line = -1
        j = i + 1
        while j < len(self.r.lines):
            c = self.r.lines[j].find(closer)
            content = self.r.lines[j] if c < 0 else self.r.lines[j][:c]
            if code_line < 0:
                directive, directive_name = _parse_directive(content)
                if directive == "::code" and not directive_name:
                    code_line = j
                    self.r.directive_only[j] = True
                elif directive is not None:
                    self.fail(j, f"only an unnamed ::code may appear in the prose of ::md {name}")
                elif c < 0 or not _is_blank(content):
                    lines.append(content)
            if c >= 0:
                break
            j += 1
        if j == len(self.r.lines):
            self.fail(i, f"::md {name}: its string or block comment is not closed")
        self.set_prose(self.r.sections[section], lines)
        if code_line >= 0:
            self.open_region(code_line, "", section, j + 1)
        return j

    def prose_line(self, i: int) -> None:
        """A line inside the prose of a section written in line comments"""
        name = self.r.sections[self.prose_section].name
        if _is_blank(self.r.lines[i]):
            self.prose.append("")
            return
        text = _line_comment_text(self.r.lines[i], self.syntax)
        if text is None:
            self.fail(i, f"::md {name}: its prose is interrupted by source code (missing ::endmd or ::code?)")
            return
        directive, directive_name = _parse_directive(text)
        if directive is None:
            self.prose.append(text)
            return
        self.r.directive_only[i] = True
        if directive == "::md":
            self.fail(i, f"::md {directive_name} inside the prose of ::md {name} (missing ::endmd?)")
        if directive == "::endcode":
            self.fail(i, f"::endcode inside the prose of ::md {name}")
        if directive == "::code" and directive_name:
            self.fail(i, f"::code {directive_name} inside the prose of ::md {name} (missing ::endmd?)")
        if directive == "::endmd" and directive_name and directive_name != name:
            self.fail(i, f"::endmd {directive_name} closes ::md {name}")
        section = self.prose_section
        self.set_prose(self.r.sections[section], self.prose)
        self.prose_section = -1
        if directive == "::code":
            self.open_region(i, "", section, i + 1)

    def line(self, i: int) -> int:
        """Reads line i; returns the last line read (a section in a container spans several lines)"""
        if self.prose_section >= 0:
            self.prose_line(i)
            return i
        line = self.r.lines[i]
        redundant = self.redundant_endmd
        if not _is_blank(line):
            self.redundant_endmd = -1
        text = _line_comment_text(line, self.syntax)
        if text is not None:
            directive, name = _parse_directive(text)
            if directive is None:
                return i
            self.r.directive_only[i] = True
            if directive == "::md":
                self.new_section(i, name)
                self.prose_section = len(self.r.sections) - 1
                self.prose = []
            elif directive == "::code":
                self.open_region(i, name, -1, i + 1)
            elif directive == "::endcode":
                self.close_region(i, name)
            else:  # ::endmd: only as a redundant close, right after the ::endcode of a section's code
                if redundant < 0:
                    self.fail(i, "::endmd without an open ::md")
                if name and name != self.r.sections[redundant].name:
                    self.fail(i, f"::endmd {name} closes ::md {self.r.sections[redundant].name}")
            return i
        opener = _container_opener(line, self.syntax)
        if opener is not None:
            closer, rest = opener
            directive, _ = _parse_directive(rest)
            if directive is None:
                return i
            if directive != "::md":
                self.fail(i, "a string or block comment may only start with ::md")
            return self.container_section(i, closer, rest)
        return i

    def run(self) -> None:
        i = 0
        while i < len(self.r.lines):
            i = self.line(i) + 1
        if self.prose_section >= 0:
            s = self.r.sections[self.prose_section]
            self.fail(s.line, f"::md {s.name} is not closed (missing ::endmd)")
        if self.open_regions:
            region = self.r.regions[self.open_regions[-1]]
            what = f"::code {region.name}" if region.name else f"the code of ::md {self.r.sections[region.section].name}"
            self.fail(region.line, f"{what} is not closed (missing ::endcode)")
        names: Dict[str, int] = {}  # name -> line
        named = [(s.name, s.line) for s in self.r.sections] + [(r.name, r.line) for r in self.r.regions if r.name]
        for name, line in named:
            if name in names:
                self.fail(line, f"the name '{name}' is already used, line {names[name] + 1}")
            names[name] = line


def _parse(file: str, lines: List[str], syntax: str) -> _ParsedFile:
    parser = _Parser(file, lines, syntax)
    try:
        parser.run()
    except _AnnotationError as e:
        parser.r.error = str(e)
    return parser.r


def _extract_code(file: _ParsedFile, region: _CodeRegion) -> List[str]:
    code = [file.lines[i] for i in range(region.begin, region.end) if not file.directive_only[i]]
    return _dedent(_trim_blank_lines(code))


def _code_block(code: List[str], language: str) -> str:
    """A fenced code block, with a fence longer than any backtick run starting a line of the code"""
    longest = 0
    for line in code:
        t = _trim(line)
        longest = max(longest, len(t) - len(t.lstrip("`")))
    fence = "`" * (longest + 1 if longest >= 3 else 3)
    return fence + language + "\n" + _join_lines(code) + fence + "\n"


# Markdown documents
# ------------------


class _FenceTracker:
    """Follows the fenced code blocks of markdown lines: update() is true for a fence line or a line inside a fence"""

    def __init__(self) -> None:
        self.fence_char = ""
        self.fence_length = 0

    def update(self, line: str) -> bool:
        t = _trim(line)
        c = t[:1]
        n = len(t) - len(t.lstrip(c)) if c in ("`", "~") else 0
        if not self.fence_char:
            if n < 3:
                return False
            self.fence_char, self.fence_length = c, n
            return True
        if c == self.fence_char and n >= self.fence_length and _is_blank(t[n:]):
            self.fence_char = ""
        return True


def _headings_of(lines: List[str]) -> List[Tuple[int, str, int]]:
    """The ATX headings (# Title), outside the fenced code blocks: (level, text, line)"""
    headings = []
    fence = _FenceTracker()
    for i, line in enumerate(lines):
        if fence.update(line):
            continue
        b = len(line) - len(line.lstrip(" "))
        if b == len(line) or b > 3 or line[b] != "#":
            continue
        e = len(line) - len(line[b:].lstrip("#"))
        if e - b > 6 or (e < len(line) and line[e] not in " \t"):
            continue
        text = _trim(line[e:])
        last = len(text.rstrip("#")) - 1  # an optional closing sequence: "## Title ##"
        if last < 0:
            text = ""
        elif last + 1 < len(text) and text[last] in " \t":
            text = _trim(text[:last])
        headings.append((e - b, text, i))
    return headings


def _find_heading(lines: List[str], path: List[str]) -> Tuple[Optional[Tuple[int, int]], str]:
    """The lines under a path of headings (Setup, then Linux under it): ((begin, end), ""), or (None, the missing step)"""
    headings = _headings_of(lines)
    start, begin, end = 0, 0, len(lines)
    for step in path:
        found = next((k for k, h in enumerate(headings) if start <= h[2] < end and h[1] == step), None)
        if found is None:
            return None, step
        level, _, line = headings[found]
        following = next((h[2] for h in headings[found + 1:] if h[0] <= level), len(lines))
        begin, end, start = line, min(end, following), line + 1
    return (begin, end), ""


# Transclusions
# -------------


def _embed_target(line: str) -> Optional[str]:
    """The target of an embed alone on its line (![[target]]), or None"""
    t = _trim(line)
    if len(t) < 5 or not t.startswith("![[") or not t.endswith("]]"):
        return None
    target = t[3:-2]
    return None if "[[" in target or "]]" in target else target


def _error_span(line: str, reason: str) -> str:
    return '<md-error title="' + reason.replace('"', "'") + '">`' + _trim(line) + "`</md-error>\n"


class Resolver:
    """Replaces the embeds of markdown texts. The errors render inline (as imgui_rich_md does) and are listed in
    `errors`, for tools that fail on them."""

    def __init__(self, read_file: ReadTextFile) -> None:
        self.read_file = read_file
        self.files: Dict[str, Optional[str]] = {}  # read once
        self.visiting: List[str] = []  # the transclusions being expanded ("file#target")
        self.errors: List[str] = []

    def read(self, path: str) -> Optional[str]:
        if path not in self.files:
            self.files[path] = self.read_file(path)
        return self.files[path]

    def error(self, line: str, reason: str) -> str:
        self.errors.append(f"{_trim(line)}: {reason}")
        return _error_span(line, reason)

    def resolve(self, lines: List[str], current_file: str, depth: int = 0) -> str:
        """Markdown lines, their embeds replaced. A transcluded block is separated from its neighbours by a blank line."""
        out = ""
        blank_needed = False
        fence = _FenceTracker()
        for line in lines:
            target = None if fence.update(line) else _embed_target(line)
            if target is None:
                if blank_needed and not _is_blank(line):
                    out += "\n"
                blank_needed = False
                out += line + "\n"
                continue
            block = self.transclude(target, line, current_file, depth)
            if not block:
                continue
            after_blank_line = out == "\n" or out.endswith("\n\n")
            if out and not after_blank_line:
                out += "\n"
            out += block
            blank_needed = True
        return out

    def transclude(self, target: str, line: str, current_file: str, depth: int) -> str:
        steps = target.split("#")
        path = steps.pop(0)
        code_selector = bool(steps) and steps[-1] == "code"
        if code_selector:
            steps.pop()
        if code_selector and not steps:
            return self.error(line, "#code follows the name of a Markdown section")
        if "" in steps or (not path and not steps):
            return self.error(line, "empty name")
        if not path and not current_file:
            return self.error(line, "![[#...]] refers to the current file, and this text comes from no file")
        if depth >= MAX_DEPTH:
            return self.error(line, "transclusions nested too deeply")
        file = current_file if not path else (path if _is_absolute(path) else _directory_of(current_file) + path)
        file = _normalize_path(file)
        key = file + target[len(path):]
        if key in self.visiting:
            return self.error(line, "transclusion cycle: " + key)
        content = self.read(file)
        if content is None:
            return self.error(line, "file not found: " + file)
        self.visiting.append(key)
        try:
            return self.transclude_from(file, split_lines(content), steps, code_selector, line, depth)
        finally:
            self.visiting.pop()

    def transclude_from(self, file: str, lines: List[str], steps: List[str], code_selector: bool, line: str, depth: int) -> str:
        syntax, language = _kind_of_file(file)
        if syntax == "markdown":
            if code_selector:
                return self.error(line, "#code: a Markdown document has no associated code")
            if steps:
                found, missing = _find_heading(lines, steps)
                if found is None:
                    return self.error(line, f"no heading '{missing}' in {file}")
                lines = lines[found[0]:found[1]]
            return self.resolve(_trim_blank_lines(lines), file, depth + 1)

        parsed = _parse(file, lines, syntax)
        if parsed.error:
            return self.error(line, parsed.error)
        if not steps:  # the whole file, without its directive lines
            code = [x for i, x in enumerate(parsed.lines) if not parsed.directive_only[i]]
            return _code_block(_trim_blank_lines(code), language)
        if len(steps) > 1:
            return self.error(line, "a source file has no headings: its targets are single names")
        name = steps[0]
        for s in parsed.sections:
            if s.name != name:
                continue
            if not code_selector:
                return self.resolve(s.prose, file, depth + 1)
            if s.code < 0:
                return self.error(line, f"::md {name} has no associated code")
            return _code_block(_extract_code(parsed, parsed.regions[s.code]), language)
        for region in parsed.regions:
            if region.name != name:
                continue
            if code_selector:
                return self.error(line, f"#code selects the code of a Markdown section, and '{name}' is a code region")
            return _code_block(_extract_code(parsed, region), language)
        return self.error(line, f"no section or code region named '{name}' in {file}")


def resolve_transclusions(markdown: str, read_file: ReadTextFile, current_file: str = "") -> str:
    """Replaces the embeds of a markdown text: read_file reads a file (or returns None); current_file is the file the
    text comes from, if any."""
    if "![[" not in markdown:
        return markdown
    return Resolver(read_file).resolve(split_lines(markdown), current_file)
