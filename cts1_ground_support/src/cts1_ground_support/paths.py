"""Utility functions for working with paths in the repository."""

from pathlib import Path

import git


def get_repo_path() -> Path:
    """Get the path to the root of the repository."""
    repo = git.Repo(__file__, search_parent_directories=True)
    return Path(repo.working_tree_dir)
