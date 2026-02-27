default:
    just --list

# Build the doc (html and pdf)
doc_build:
    python docs_src/process_md_docs.py
    cd docs_src && jupyter book build --html && jupyter book build --pdf

# Serve the doc while working on them (deployment is via github action)
doc_serve:
    python docs_src/process_md_docs.py
    cd docs_src && jupyter book start
