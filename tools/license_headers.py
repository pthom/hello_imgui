#!/usr/bin/env python3
#
# Copyright (c) 2020 Kai Szuttor.
#
# This file is part of CPP Project Template
# (see https://github.com/kaiszuttor/cpp_project_template).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

"""A tool to change or add license headers in all supported files in or below a directory."""

# Copyright (c) 2016-2018 Johann Petrak
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

import argparse
import fnmatch
import logging
import os
import sys
from shutil import copyfile
from string import Template

import regex as re

__version__ = '0.8.4'
__author__ = 'Johann Petrak'
__license__ = 'MIT'

LOGGER = logging.getLogger("licenseheaders_{}".format(__version__))

# for each processing type, the detailed settings of how to process files of that type
typeSettings = {
    "java": {
        "extensions": [".java", ".scala", ".groovy", ".jape", ".js"],
        "keepFirst": None,
        "blockCommentStartPattern": re.compile(r'^\s*/\*'),
        "blockCommentEndPattern": re.compile(r'\*/\s*$'),
        "lineCommentStartPattern": re.compile(r'\s*//'),
        "lineCommentEndPattern": None,
        "headerStartLine": "/*\n",
        "headerEndLine": " */\n",
        "headerLinePrefix": " * ",
        "headerLineSuffix": None,
    },
    "script": {
        "extensions": [".sh", ".csh", ".py", ".pl"],
        "keepFirst": re.compile(r'^#!|^# -\*-'),
        "blockCommentStartPattern": None,
        "blockCommentEndPattern": None,
        "lineCommentStartPattern": re.compile(r'\s*#'),
        "lineCommentEndPattern": None,
        "headerStartLine": "##\n",
        "headerEndLine": "##\n",
        "headerLinePrefix": "## ",
        "headerLineSuffix": None
    },
    "perl": {
        "extensions": [".pl"],
        "keepFirst": re.compile(r'^#!|^# -\*-'),
        "blockCommentStartPattern": None,
        "blockCommentEndPattern": None,
        "lineCommentStartPattern": re.compile(r'\s*#'),
        "lineCommentEndPattern": None,
        "headerStartLine": "##\n",
        "headerEndLine": "##\n",
        "headerLinePrefix": "## ",
        "headerLineSuffix": None
    },
    "python": {
        "extensions": [".py"],
        "keepFirst": re.compile(r'^#!|^# +pylint|^# +-\*-|^# +coding|^# +encoding'),
        "blockCommentStartPattern": None,
        "blockCommentEndPattern": None,
        "lineCommentStartPattern": re.compile(r'\s*#'),
        "lineCommentEndPattern": None,
        "headerStartLine": "#\n",
        "headerEndLine": "#\n",
        "headerLinePrefix": "# ",
        "headerLineSuffix": None
    },
    "robot": {
        "extensions": [".robot"],
        "keepFirst": re.compile(r'^#!|^# +pylint|^# +-\*-|^# +coding|^# +encoding'),
        "blockCommentStartPattern": None,
        "blockCommentEndPattern": None,
        "lineCommentStartPattern": re.compile(r'\s*#'),
        "lineCommentEndPattern": None,
        "headerStartLine": None,
        "headerEndLine": None,
        "headerLinePrefix": "# ",
        "headerLineSuffix": None
    },
    "xml": {
        "extensions": [".xml"],
        "keepFirst": re.compile(r'^\s*<\?xml.*\?>'),
        "blockCommentStartPattern": re.compile(r'^\s*<!--'),
        "blockCommentEndPattern": re.compile(r'-->\s*$'),
        "lineCommentStartPattern": None,
        "lineCommentEndPattern": None,
        "headerStartLine": "<!--\n",
        "headerEndLine": "  -->\n",
        "headerLinePrefix": "-- ",
        "headerLineSuffix": None
    },
    "sql": {
        "extensions": [".sql"],
        "keepFirst": None,
        "blockCommentStartPattern": None,  # re.compile('^\s*/\*'),
        "blockCommentEndPattern": None,  # re.compile(r'\*/\s*$'),
        "lineCommentStartPattern": re.compile(r'\s*--'),
        "lineCommentEndPattern": None,
        "headerStartLine": "--\n",
        "headerEndLine": "--\n",
        "headerLinePrefix": "-- ",
        "headerLineSuffix": None
    },
    "c": {
        "extensions": [".c", ".cc", ".cpp", "c++", ".h", ".hpp"],
        "keepFirst": None,
        "blockCommentStartPattern": re.compile(r'^\s*/\*'),
        "blockCommentEndPattern": re.compile(r'\*/\s*$'),
        "lineCommentStartPattern": re.compile(r'\s*//'),
        "lineCommentEndPattern": None,
        "headerStartLine": "/*\n",
        "headerEndLine": " */\n",
        "headerLinePrefix": " * ",
        "headerLineSuffix": None
    },
    "ruby": {
        "extensions": [".rb"],
        "keepFirst": "^#!",
        "blockCommentStartPattern": re.compile('^=begin'),
        "blockCommentEndPattern": re.compile(r'^=end'),
        "lineCommentStartPattern": re.compile(r'\s*#'),
        "lineCommentEndPattern": None,
        "headerStartLine": "##\n",
        "headerEndLine": "##\n",
        "headerLinePrefix": "## ",
        "headerLineSuffix": None
    },
    "csharp": {
        "extensions": [".cs"],
        "keepFirst": None,
        "blockCommentStartPattern": None,
        "blockCommentEndPattern": None,
        "lineCommentStartPattern": re.compile(r'\s*//'),
        "lineCommentEndPattern": None,
        "headerStartLine": None,
        "headerEndLine": None,
        "headerLinePrefix": "// ",
        "headerLineSuffix": None
    },
    "vb": {
        "extensions": [".vb"],
        "keepFirst": None,
        "blockCommentStartPattern": None,
        "blockCommentEndPattern": None,
        "lineCommentStartPattern": re.compile(r"^\s*\'"),
        "lineCommentEndPattern": None,
        "headerStartLine": None,
        "headerEndLine": None,
        "headerLinePrefix": "' ",
        "headerLineSuffix": None
    },
    "erlang": {
        "extensions": [".erl", ".src", ".config", ".schema"],
        "keepFirst": None,
        "blockCommentStartPattern": None,
        "blockCommentEndPattern": None,
        "lineCommentStartPattern": None,
        "lineCommentEndPattern": None,
        "headerStartLine": "%% -*- erlang -*-\n%% %CopyrightBegin%\n%%\n",
        "headerEndLine": "%%\n%% %CopyrightEnd%\n\n",
        "headerLinePrefix": "%% ",
        "headerLineSuffix": None,
    },
    "html": {
        "extensions": [".html"],
        "keepFirst": re.compile(r'^\s*<\!DOCTYPE.*>'),
        "blockCommentStartPattern": re.compile(r'^\s*<!--'),
        "blockCommentEndPattern": re.compile(r'-->\s*$'),
        "lineCommentStartPattern": None,
        "lineCommentEndPattern": None,
        "headerStartLine": "<!--\n",
        "headerEndLine": "-->\n",
        "headerLinePrefix": "-- ",
        "headerLineSuffix": None
    },
    "css": {
        "extensions": [".css", ".scss", ".sass"],
        "keepFirst": None,
        "blockCommentStartPattern": re.compile(r'^\s*/\*'),
        "blockCommentEndPattern": re.compile(r'\*/\s*$'),
        "lineCommentStartPattern": None,
        "lineCommentEndPattern": None,
        "headerStartLine": "/*\n",
        "headerEndLine": "*/\n",
        "headerLinePrefix": None,
        "headerLineSuffix": None
    },
    "docker": {
        "extensions": [".dockerfile"],
        "filenames": ["Dockerfile"],
        "keepFirst": None,
        "blockCommentStartPattern": None,
        "blockCommentEndPattern": None,
        "lineCommentStartPattern": re.compile(r'\s*#'),
        "lineCommentEndPattern": None,
        "headerStartLine": "##\n",
        "headerEndLine": "##\n",
        "headerLinePrefix": "## ",
        "headerLineSuffix": None
    },
    "yaml": {
        "extensions": [".yaml", ".yml"],
        "keepFirst": None,
        "blockCommentStartPattern": None,
        "blockCommentEndPattern": None,
        "lineCommentStartPattern": re.compile(r'\s*#'),
        "lineCommentEndPattern": None,
        "headerStartLine": "##\n",
        "headerEndLine": "##\n",
        "headerLinePrefix": "## ",
        "headerLineSuffix": None
    }
}

yearsPattern = re.compile(
    r"(?<=Copyright\s*(?:\(\s*[Cc©]\s*\)\s*))?([0-9][0-9][0-9][0-9](?:-[0-9][0-9]?[0-9]?[0-9]?)?)",
    re.IGNORECASE)
licensePattern = re.compile(r"license", re.IGNORECASE)
emptyPattern = re.compile(r'^\s*$')

# maps each extension to its processing type. Filled from tpeSettings during initialization
ext2type = {}
name2type = {}
patterns = []


# class for dict args. Use --argname key1=val1,val2 key2=val3 key3=val4, val5
class DictArgs(argparse.Action):
    def __call__(self, parser, namespace, values, option_string=None):
        dict_args = {}
        if not isinstance(values, (list,)):
            values = (values,)
        for value in values:
            n, v = value.split("=")
            if n not in typeSettings:
                LOGGER.error("No valid language '%s' to add additional file extensions for" % n)
            if v and "," in str(v):
                dict_args[n] = v.split(",")
            else:
                dict_args[n] = list()
                dict_args[n].append(str(v).strip())
        setattr(namespace, self.dest, dict_args)


def parse_command_line(argv):
    """
    Parse command line argument. See -h option.
    :param argv: the actual program arguments
    :return: parsed arguments
    """
    import textwrap

    default_dir = "."
    default_encoding = "utf-8"

    known_extensions = [ftype+":"+",".join(conf["extensions"]) for ftype, conf in typeSettings.items()]
    # known_extensions = [ext for ftype in typeSettings.values() for ext in ftype["extensions"]]

    example = textwrap.dedent("""
      Known extensions: {0}

      If -t/--tmpl is specified, that header is added to (or existing header replaced for) all source files of known type
      If -t/--tmpl is not specified byt -y/--years is specified, all years in existing header files
        are replaced with the years specified

      Examples:
        {1} -t lgpl-v3 -y 2012-2014 -o ThisNiceCompany -n ProjectName -u http://the.projectname.com
        {1} -y 2012-2015
        {1} -y 2012-2015 -d /dir/where/to/start/
        {1} -y 2012-2015 -d /dir/where/to/start/ --additional-extensions python=.j2
        {1} -y 2012-2015 -d /dir/where/to/start/ --additional-extensions python=.j2,.tpl script=.txt
      See: https://github.com/johann-petrak/licenseheaders
    """).format(known_extensions, os.path.basename(argv[0]))
    formatter_class = argparse.RawDescriptionHelpFormatter
    parser = argparse.ArgumentParser(description="Python license header updater",
                                     epilog=example,
                                     formatter_class=formatter_class)
    parser.add_argument("-V", "--version", action="version",
                        version="%(prog)s {}".format(__version__))
    parser.add_argument("-v", "--verbose", dest="verbose_count",
                        action="count", default=0,
                        help="increases log verbosity (can be specified "
                             "1 to 3 times, default shows errors only)")
    parser.add_argument("-d", "--dir", dest="dir", default=default_dir,
                        help="The directory to recursively process (default: {}).".format(default_dir))
    parser.add_argument("-b", action="store_true",
                        help="Back up all files which get changed to a copy with .bak added to the name")
    parser.add_argument("-t", "--tmpl", dest="tmpl", default=None,
                        help="Template name or file to use.")
    parser.add_argument("-y", "--years", dest="years", default=None,
                        help="Year or year range to use.")
    parser.add_argument("-o", "--owner", dest="owner", default=None,
                        help="Name of copyright owner to use.")
    parser.add_argument("-n", "--projname", dest="projectname", default=None,
                        help="Name of project to use.")
    parser.add_argument("-u", "--projurl", dest="projecturl", default=None,
                        help="Url of project to use.")
    parser.add_argument("--enc", nargs=1, dest="encoding", default=default_encoding,
                        help="Encoding of program files (default: {})".format(default_encoding))
    parser.add_argument("--dry", action="store_true", help="Only show what would get done, do not change any files")
    parser.add_argument("--safesubst", action="store_true",
                        help="Do not raise error if template variables cannot be substituted.")
    parser.add_argument("-D", "--debug", action="store_true", help="Enable debug messages (same as -v -v -v)")
    parser.add_argument("-E","--ext", type=str, nargs="*",  
                        help="If specified, restrict processing to the specified extension(s) only")
    parser.add_argument("--additional-extensions", dest="additional_extensions", default=None, nargs="+",
                        help="Provide a comma-separated list of additional file extensions as value for a "
                             "specified language as key, each with a leading dot and no whitespace (default: None).",
                        action=DictArgs)
    parser.add_argument("-x", "--exclude", type=str, nargs="*",
                        help="File path patterns to exclude")
    arguments = parser.parse_args(argv[1:])

    # Sets log level to WARN going more verbose for each new -V.
    loglevel = max(4 - arguments.verbose_count, 1) * 10
    global LOGGER
    LOGGER.setLevel(loglevel)
    if arguments.debug:
        LOGGER.setLevel(logging.DEBUG)
    # fmt = logging.Formatter('%(asctime)s|%(levelname)s|%(name)s|%(message)s')
    fmt = logging.Formatter('%(name)s %(levelname)s: %(message)s')
    hndlr = logging.StreamHandler(sys.stderr)
    hndlr.setFormatter(fmt)
    LOGGER.addHandler(hndlr)

    return arguments


def get_paths(fnpatterns, start_dir="."):
    """
    Retrieve files that match any of the glob patterns from the start_dir and below.
    :param fnpatterns: the file name patterns
    :param start_dir: directory where to start searching
    :return: generator that returns one path after the other
    """
    seen = set()
    for root, dirs, files in os.walk(start_dir):
        names = []
        for pattern in fnpatterns:
            names += fnmatch.filter(files, pattern)
        for name in names:
            path = os.path.join(root, name)
            if path in seen:
                continue
            seen.add(path)
            yield path


def read_template(template_file, vardict, args):
    """
    Read a template file replace variables from the dict and return the lines.
    Throws exception if a variable cannot be replaced.
    :param template_file: template file with variables
    :param vardict: dictionary to replace variables with values
    :param args: the program arguments
    :return: lines of the template, with variables replaced
    """
    with open(template_file, 'r') as f:
        lines = f.readlines()
    if args.safesubst:
        lines = [Template(line).safe_substitute(vardict) for line in lines]
    else:
        lines = [Template(line).substitute(vardict) for line in lines]
    return lines


def for_type(templatelines, ftype):
    """
    Format the template lines for the given ftype.
    :param templatelines: the lines of the template text
    :param ftype: file type
    :return: header lines
    """
    lines = []
    settings = typeSettings[ftype]
    header_start_line = settings["headerStartLine"]
    header_end_line = settings["headerEndLine"]
    header_line_prefix = settings["headerLinePrefix"]
    header_line_suffix = settings["headerLineSuffix"]
    if header_start_line is not None:
        lines.append(header_start_line)
    for line in templatelines:
        tmp = line
        if header_line_prefix is not None and line == '\n':
            tmp = header_line_prefix.rstrip() + tmp
        elif header_line_prefix is not None:
            tmp = header_line_prefix + tmp
        if header_line_suffix is not None:
            tmp = tmp + header_line_suffix
        lines.append(tmp)
    if header_end_line is not None:
        lines.append(header_end_line)
    return lines


##
def read_file(file, args):
    """
    Read a file and return a dictionary with the following elements:
    :param file: the file to read
    :param args: the options specified by the user
    :return: a dictionary with the following entries or None if the file is not supported:
      - skip: number of lines at the beginning to skip (always keep them when replacing or adding something)
       can also be seen as the index of the first line not to skip
      - headStart: index of first line of detected header, or None if non header detected
      - headEnd: index of last line of detected header, or None
      - yearsLine: index of line which contains the copyright years, or None
      - haveLicense: found a line that matches a pattern that indicates this could be a license header
      - settings: the type settings
    """
    skip = 0
    head_start = None
    head_end = None
    years_line = None
    have_license = False
    filename, extension = os.path.splitext(file)
    LOGGER.debug("File name is %s", os.path.basename(filename))
    LOGGER.debug("File extension is %s", extension)
    # if we have no entry in the mapping from extensions to processing type, return None
    ftype = ext2type.get(extension)
    LOGGER.debug("Type for this file is %s", ftype)
    if not ftype:
        ftype = name2type.get(os.path.basename(filename))
        if not ftype:
            return None
    settings = typeSettings.get(ftype)
    with open(file, 'r', encoding=args.encoding) as f:
        lines = f.readlines()
    # now iterate throw the lines and try to determine the various indies
    # first try to find the start of the header: skip over shebang or empty lines
    keep_first = settings.get("keepFirst")
    block_comment_start_pattern = settings.get("blockCommentStartPattern")
    block_comment_end_pattern = settings.get("blockCommentEndPattern")
    line_comment_start_pattern = settings.get("lineCommentStartPattern")
    i = 0
    LOGGER.info("Processing file {} as {}".format(file, ftype))
    for line in lines:
        if i == 0 and keep_first and keep_first.findall(line):
            skip = i + 1
        elif emptyPattern.findall(line):
            pass
        elif block_comment_start_pattern and block_comment_start_pattern.findall(line):
            head_start = i
            break
        elif line_comment_start_pattern and line_comment_start_pattern.findall(line):
            head_start = i
            break
        elif not block_comment_start_pattern and \
                line_comment_start_pattern and \
                line_comment_start_pattern.findall(line):
            head_start = i
            break
        else:
            # we have reached something else, so no header in this file
            # logging.debug("Did not find the start giving up at line %s, line is >%s<",i,line)
            return {"type": ftype,
                    "lines": lines,
                    "skip": skip,
                    "headStart": None,
                    "headEnd": None,
                    "yearsLine": None,
                    "settings": settings,
                    "haveLicense": have_license
                    }
        i = i + 1
    LOGGER.debug("Found preliminary start at {}, i={}, lines={}".format(head_start, i, len(lines)))
    # now we have either reached the end, or we are at a line where a block start or line comment occurred
    # if we have reached the end, return default dictionary without info
    if i == len(lines):
        LOGGER.debug("We have reached the end, did not find anything really")
        return {"type": ftype,
                "lines": lines,
                "skip": skip,
                "headStart": head_start,
                "headEnd": head_end,
                "yearsLine": years_line,
                "settings": settings,
                "haveLicense": have_license
                }
    # otherwise process the comment block until it ends
    if block_comment_start_pattern:
        LOGGER.debug("Found comment start, process until end")
        for j in range(i, len(lines)):
            LOGGER.debug("Checking line {}".format(j))
            if licensePattern.findall(lines[j]):
                have_license = True
            elif block_comment_end_pattern.findall(lines[j]):
                return {"type": ftype,
                        "lines": lines,
                        "skip": skip,
                        "headStart": head_start,
                        "headEnd": j,
                        "yearsLine": years_line,
                        "settings": settings,
                        "haveLicense": have_license
                        }
            elif yearsPattern.findall(lines[j]):
                have_license = True
                years_line = j
        # if we went through all the lines without finding an end, maybe we have some syntax error or some other
        # unusual situation, so lets return no header
        LOGGER.debug("Did not find the end of a block comment, returning no header")
        return {"type": ftype,
                "lines": lines,
                "skip": skip,
                "headStart": None,
                "headEnd": None,
                "yearsLine": None,
                "settings": settings,
                "haveLicense": have_license
                }
    else:
        LOGGER.debug("ELSE1")
        for j in range(i, len(lines)):
            if line_comment_start_pattern.findall(lines[j]) and licensePattern.findall(lines[j]):
                have_license = True
            elif not line_comment_start_pattern.findall(lines[j]):
                LOGGER.debug("ELSE2")
                return {"type": ftype,
                        "lines": lines,
                        "skip": skip,
                        "headStart": i,
                        "headEnd": j - 1,
                        "yearsLine": years_line,
                        "settings": settings,
                        "haveLicense": have_license
                        }
            elif yearsPattern.findall(lines[j]):
                have_license = True
                years_line = j
        # if we went through all the lines without finding the end of the block, it could be that the whole
        # file only consisted of the header, so lets return the last line index
        LOGGER.debug("RETURN")
        return {"type": ftype,
                "lines": lines,
                "skip": skip,
                "headStart": i,
                "headEnd": len(lines) - 1,
                "yearsLine": years_line,
                "settings": settings,
                "haveLicense": have_license
                }


def make_backup(file, arguments):
    """
    Backup file by copying it to a file with the extension .bak appended to the name.
    :param file: file to back up
    :param arguments: program args, only backs up, if required by an option
    :return:
    """
    if arguments.b:
        LOGGER.info("Backing up file {} to {}".format(file, file + ".bak"))
        if not arguments.dry:
            copyfile(file, file + ".bak")


def main():
    """Main function."""
    # LOGGER.addHandler(logging.StreamHandler(stream=sys.stderr))
    # init: create the ext2type mappings
    arguments = parse_command_line(sys.argv)
    additional_extensions = arguments.additional_extensions
    for t in typeSettings:
        settings = typeSettings[t]
        exts = settings["extensions"]
        if "filenames" in settings:
            names = settings['filenames']
        else:
            names = []
        # if additional file extensions are provided by the user, they are "merged" here:
        if additional_extensions and t in additional_extensions:
            for aext in additional_extensions[t]:
                LOGGER.debug("Enable custom file extension '%s' for language '%s'" % (aext, t))
                exts.append(aext)

        for ext in exts:
            ext2type[ext] = t
            patterns.append("*" + ext)

        for name in names:
            name2type[name] = t
            patterns.append(name)

    LOGGER.debug("Allowed file patterns %s" % patterns)

    limit2exts = None
    if arguments.ext is not None and len(arguments.ext) > 0:
        limit2exts = arguments.ext

    try:
        error = False
        template_lines = None
        start_dir = arguments.dir
        settings = {}
        if arguments.years:
            settings["years"] = arguments.years
        if arguments.owner:
            settings["owner"] = arguments.owner
        if arguments.projectname:
            settings["projectname"] = arguments.projectname
        if arguments.projecturl:
            settings["projecturl"] = arguments.projecturl
        # if we have a template name specified, try to get or load the template
        if arguments.tmpl:
            opt_tmpl = arguments.tmpl
            # first get all the names of our own templates
            # for this get first the path of this file
            templates_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "templates")
            LOGGER.debug("File path: {}".format(os.path.abspath(__file__)))
            # get all the templates in the templates directory
            templates = [f for f in get_paths("*.tmpl", templates_dir)]
            templates = [(os.path.splitext(os.path.basename(t))[0], t) for t in templates]
            # filter by trying to match the name against what was specified
            tmpls = [t for t in templates if opt_tmpl in t[0]]
            # check if one of the matching template names is identical to the parameter, then take that one
            tmpls_eq = [t for t in tmpls if opt_tmpl == t[0]]
            if len(tmpls_eq) > 0:
                tmpls = tmpls_eq
            if len(tmpls) == 1:
                tmpl_name = tmpls[0][0]
                tmpl_file = tmpls[0][1]
                LOGGER.info("Using template file {} for {}".format(tmpl_file, tmpl_name))
                template_lines = read_template(tmpl_file, settings, arguments)
            else:
                if len(tmpls) == 0:
                    # check if we can interpret the option as file
                    if os.path.isfile(opt_tmpl):
                        LOGGER.info("Using file {}".format(os.path.abspath(opt_tmpl)))
                        template_lines = read_template(os.path.abspath(opt_tmpl), settings, arguments)
                    else:
                        LOGGER.error("Not a built-in template and not a file, cannot proceed: {}".format(opt_tmpl))
                        LOGGER.error("Built in templates: {}".format(", ".join([t[0] for t in templates])))
                        error = True
                else:
                    LOGGER.error("There are multiple matching template names: {}".format([t[0] for t in tmpls]))
                    error = True
        else:
            # no tmpl parameter
            if not arguments.years:
                LOGGER.error("No template specified and no years either, nothing to do (use -h option for usage info)")
                error = True
        if not error:
            # logging.debug("Got template lines: %s",templateLines)
            # now do the actual processing: if we did not get some error, we have a template loaded or
            # no template at all
            # if we have no template, then we will have the years.
            # now process all the files and either replace the years or replace/add the header
            LOGGER.debug("Processing directory %s", start_dir)
            LOGGER.debug("Patterns: %s", patterns)
            paths = get_paths(patterns, start_dir)
            for file in paths:
                LOGGER.debug("Considering file: {}".format(file))
                file = os.path.normpath(file)
                if limit2exts is not None and not any([file.endswith(ext) for ext in limit2exts]):
                    LOGGER.info("Skipping file with non-matching extension: {}".format(file))
                    continue
                if arguments.exclude and any([fnmatch.fnmatch(file, pat) for pat in arguments.exclude]):
                    LOGGER.info("Ignoring file {}".format(file))
                    continue
                finfo = read_file(file, arguments)
                if not finfo:
                    LOGGER.debug("File not supported %s", file)
                    continue
                # logging.debug("FINFO for the file: %s", finfo)
                lines = finfo["lines"]
                LOGGER.debug(
                    "Info for the file: headStart=%s, headEnd=%s, haveLicense=%s, skip=%s, len=%s, yearsline=%s",
                    finfo["headStart"], finfo["headEnd"], finfo["haveLicense"], finfo["skip"], len(lines),
                    finfo["yearsLine"])
                # if we have a template: replace or add
                if template_lines:
                    make_backup(file, arguments)
                    if arguments.dry:
                        LOGGER.info("Would be updating changed file: {}".format(file))
                    else:
                        with open(file, 'w', encoding=arguments.encoding) as fw:
                            # if we found a header, replace it
                            # otherwise, add it after the lines to skip
                            head_start = finfo["headStart"]
                            head_end = finfo["headEnd"]
                            have_license = finfo["haveLicense"]
                            ftype = finfo["type"]
                            skip = finfo["skip"]
                            if head_start is not None and head_end is not None and have_license:
                                LOGGER.debug("Replacing header in file {}".format(file))
                                # first write the lines before the header
                                fw.writelines(lines[0:head_start])
                                #  now write the new header from the template lines
                                fw.writelines(for_type(template_lines, ftype))
                                #  now write the rest of the lines
                                fw.writelines(lines[head_end + 1:])
                            else:
                                LOGGER.debug("Adding header to file {}, skip={}".format(file, skip))
                                fw.writelines(lines[0:skip])
                                fw.writelines(for_type(template_lines, ftype))
                                fw.writelines(lines[skip:])
                        # TODO: optionally remove backup if all worked well?
                else:
                    # no template lines, just update the line with the year, if we found a year
                    years_line = finfo["yearsLine"]
                    if years_line is not None:
                        make_backup(file, arguments)
                        if arguments.dry:
                            LOGGER.info("Would be updating year line in file {}".format(file))
                        else:
                            with open(file, 'w', encoding=arguments.encoding) as fw:
                                LOGGER.debug("Updating years in file {} in line {}".format(file, years_line))
                                fw.writelines(lines[0:years_line])
                                fw.write(yearsPattern.sub(arguments.years, lines[years_line]))
                                fw.writelines(lines[years_line + 1:])
                            # TODO: optionally remove backup if all worked well
    finally:
        logging.shutdown()


if __name__ == "__main__":
    sys.exit(main())
