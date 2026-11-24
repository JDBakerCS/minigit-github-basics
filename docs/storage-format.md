# MiniGit storage format

Paths are relative to `.minigit/`, which `init` creates.

## What `init` creates
`init` creates `.minigit/` and these four entries, all empty. It creates no snapshot.
- `stage/` (empty folder)
- `commits/` (empty folder)
- `index.txt` (empty file)
- `HEAD` (empty file)

Running `init` in a valid repository prints a message and changes nothing.

## Every file and folder

| Path | Contents | Changed by |
|---|---|---|
| `stage/` | Selected bytes under relative paths | `add` writes; `commit` clears after successful publication |
| `commits/` | Saved snapshots | `commit` adds; `status` and `log` read |
| `index.txt` | Current selected paths | `add` updates; `commit` clears |
| `HEAD` | Latest snapshot ID; empty means no commits | `commit` updates; `status` and `log` read |
| `commits/<id>/` | Record metadata and complete snapshot; absent until the first commit | `commit` creates; `status` and `log` read through SnapshotStore |
| `commits/<id>/files/<relativePath>` | Exact recorded bytes (existing `committedFile()` location) | `commit` writes; snapshot readers read |

## Rules
- An empty `HEAD` means there are no commits yet.
- Record metadata is kept separate from user file paths, so their names cannot collide.
- The record metadata filenames and how the message, previous ID, and non-empty HEAD are written are defined later, outside this issue.