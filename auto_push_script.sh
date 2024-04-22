#!/bin/bash
# echo "# Projects" >> README.md
git init
# git add README.md
git add .
git commit -m "commit # $1"
git branch -M main
git remote add origin https://github.com/awaiz360/Projects.git
git push -u origin main
