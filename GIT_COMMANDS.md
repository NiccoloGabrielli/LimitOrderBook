# Git Cheat Sheet & Common Terminal Commands

A quick reference guide for daily Git operations: staging, committing, branching, syncing with GitHub, and troubleshooting.

---

## 1. Initial Setup & Connecting to GitHub

Your local repository is already initialized (`git init`), but to link it to GitHub:

### Step 1: Create a new repository on GitHub
1. Go to [github.com/new](https://github.com/new).
2. Enter repository name: `lob` (or your preferred name).
3. Choose **Public** or **Private**.
4. **Leave "Initialize this repository with README, .gitignore, license" UNCHECKED** (since we already have local files).
5. Click **Create repository**.

### Step 2: Link local repository and push
Copy your repository URL from GitHub (HTTPS or SSH) and run:

```bash
# Add the remote repository URL
git remote add origin https://github.com/<your-username>/lob.git

# Verify remote configuration
git remote -v

# Ensure default branch is main
git branch -M main

# Make your first commit (if not already done)
git add .
git commit -m "Initial commit: project structure and git guide"

# Push to GitHub and set upstream tracking (-u)
git push -u origin main
```

---

## 2. Everyday Workflow: Staging & Committing

```bash
# Check the status of your working tree (modified, staged, untracked files)
git status

# View exact line changes not yet staged
git diff

# View line changes that are staged for commit
git diff --staged

# Stage a specific file
git add <filename>

# Stage all modified and new files in the project
git add .

# Commit staged changes with an informative message
git commit -m "feat: implement OrderBook buy/sell matching engine"

# Amend the last commit (modify message or include forgotten files)
git commit --amend -m "feat: new commit message"
```

---

## 3. Branching & Merging

Use branches to isolate new features, fixes, or experiments without breaking `main`.

```bash
# List local branches (* indicates current branch)
git branch

# List all branches including remote branches
git branch -a

# Create and switch to a new branch
git checkout -b feature/order-matching
# Or using modern git:
git switch -c feature/order-matching

# Switch between existing branches
git checkout main
# Or:
git switch main

# Merge a branch into your current branch
git checkout main
git merge feature/order-matching

# Delete a branch locally (after it has been merged)
git branch -d feature/order-matching

# Force delete an unmerged branch
git branch -D feature/order-matching

# Delete a branch on remote (GitHub)
git push origin --delete feature/order-matching
```

---

## 4. Syncing with Remote (GitHub)

```bash
# Push your current branch to GitHub (first time for this branch)
git push -u origin feature/order-matching

# Subsequent pushes on the same branch
git push

# Fetch latest changes and branches from GitHub without merging
git fetch origin

# Pull and merge changes from GitHub into current branch
git pull origin main

# Pull with rebase to keep linear git history
git pull --rebase origin main
```

---

## 5. Inspection & History

```bash
# View commit history (condensed one-line per commit)
git log --oneline

# View commit history with graphical branch tree
git log --oneline --graph --all

# View the last 5 commits with file change statistics
git log -n 5 --stat

# See details and diff of a specific commit
git show <commit-hash>
```

---

## 6. Stashing (Save Work Without Committing)

Useful when you need to switch branches quickly but aren't ready to commit:

```bash
# Temporarily shelve all modified tracked files
git stash

# Stash with a descriptive message
git stash save "WIP: limit order cancellation logic"

# List all saved stashes
git stash list

# Re-apply the most recent stash and remove it from stash list
git stash pop

# Discard the most recent stash
git stash drop
```

---

## 7. Undoing Changes & Recovery

```bash
# Discard unstaged changes in a specific file (restore to last commit)
git restore <filename>

# Unstage a file (keep local changes in file)
git restore --staged <filename>

# Soft reset: undo commit(s) but keep changes staged
git reset --soft HEAD~1

# Mixed reset (default): undo commit(s), unstage files, keep local edits
git reset HEAD~1

# Hard reset: CAUTION! Completely discards all uncommitted changes and resets to last commit
git reset --hard HEAD

# Revert a commit by creating a new inverse commit (safe for shared branches)
git revert <commit-hash>
```

---

## 8. Quick Recommended Workflow for New Features

```bash
# 1. Start from up-to-date main
git switch main
git pull

# 2. Create feature branch
git switch -c feature/order-id-generator

# 3. Code, test, stage, commit
git add .
git commit -m "feat: add thread-safe order id generator"

# 4. Push to GitHub
git push -u origin feature/order-id-generator

# 5. Open Pull Request on GitHub, review, merge, then return to main
git switch main
git pull
```
