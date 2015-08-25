git filter-branch -f --prune-empty --index-filter 'git rm -rf --cached --ignore-unmatch Applications Bazaar remove-proprietary.sh' HEAD
