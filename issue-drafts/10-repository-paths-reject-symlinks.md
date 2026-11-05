# Issue RP-6 (stretch) Reject symbolic links in safeRelativeFile

## Goal
Stop a symbolic link inside the project from pointing MiniGit at a file outside it.

## Dependency
RP-5 merged.

## Acceptance criteria
- After the RP-5 checks, walk from `projectRoot_` through each part of the clean path and reject the path if any part that exists is a symbolic link.
- Use `std::filesystem::symlink_status(p, error)` with a `std::error_code`, so a part that does not exist is not treated as an error.
- All RP-5 tests still pass.
- Local CTest and Windows/Ubuntu CI pass.

## Tests to write
- Create a temporary project under `std::filesystem::temp_directory_path()` with a real file, a symbolic link to that file, and a symbolic link to a folder.
- The real file is accepted; the file link and a file reached through the folder link are rejected.
- Creating a link can fail on Windows without Developer Mode. If `create_symlink` reports an error, print `skipped` and count the test as passed.
- Remove the temporary folder at the end, even when a check fails.

## Out of scope
Following or resolving links.

## Reviewer focus
Is the temporary folder always removed? Does the test still pass on a Windows runner that cannot create links?
