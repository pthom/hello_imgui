default:
    just --list

doc_process_md:
    python docs_src/process_md_docs.py

# Deploy the book to GitHub pages (this will update the docs branch)
doc_deploy:
    ./docs_src/deploy_book.sh


