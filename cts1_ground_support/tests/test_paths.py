"""Test the `paths` module."""

import tempfile
from pathlib import Path

import pytest

from cts1_ground_support.paths import get_repo_path, read_text_file


def test_get_repo_path() -> None:
    """Test the `get_repo_path` function."""
    repo_path = get_repo_path()

    assert repo_path == get_repo_path(), "get_repo_path should be idempotent"
    assert repo_path == Path(__file__).parent.parent.parent

    # Assert expectations about the contents of the repo.
    assert (repo_path / "README.md").is_file()
    assert (repo_path / "firmware").is_dir()


test_str_ascii = "Test str with only ascii~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=`~\ncharacters"
test_str_unicode = """Test with unicode characters 🤔 (result == 0)\n? "PASS ✅" : "FAIL ❌", """


@pytest.mark.parametrize("test_str", [test_str_ascii, test_str_unicode])
def test_read_text_file(test_str: str) -> None:
    """Test the `read_text_file` function. Function exists because Windows is wacky."""
    with tempfile.TemporaryDirectory() as dir_path:
        f = Path(dir_path) / "test.txt"
        f.write_bytes(test_str.encode("utf-8"))

        read_back = read_text_file(f)
    assert read_back == test_str
