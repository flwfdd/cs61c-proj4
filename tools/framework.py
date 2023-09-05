from pathlib import Path
from typing import Callable, List, Dict
import hashlib
import numpy as np
import os
import random
import struct
import subprocess

import convolve_oracle

project_dir = Path(__file__).parent.resolve().parent
tests_dir = project_dir / "tests"
# oracle_path = Path("/home/ff/cs61c/sp23/proj4/convolve_oracle")

all_tests: Dict[str, "TestSpec"] = {}


def set_tests_dir(path: Path):
    global tests_dir
    tests_dir = path


def run_oracle(a_path: Path, b_path: Path, out_path: Path):
    # if not oracle_path.exists():
    #     raise RuntimeError("Oracle does not exist, please run on the hive machines")
    # subprocess.run([oracle_path, a_path, b_path, out_path])
    convolve_oracle.convolve(a_path, b_path, out_path)


def randint(lower, upper, **kwargs):
    return np.random.randint(lower, upper + 1, **kwargs)


def md5sum(path: Path) -> str:
    with path.open("rb") as f:
        contents = f.read()
    return hashlib.md5(contents).hexdigest()


def md5sum_file(path: Path):
    md5 = md5sum(path)
    with (path.parent / f"{path.name}.md5").open("w") as f:
        f.write(f"{md5}\n")


class Matrix:
    def random(rows: int, cols: int, min_value=-1000, max_value=1000) -> "Matrix":
        data_fn = lambda: list(
            randint(min_value, max_value, size=rows * cols) & 0xFFFFFFFF
        )
        return Matrix(rows, cols, data_fn)

    def __init__(self, rows: int, cols: int, data_fn: Callable[[], List[int]]):
        self.rows = rows
        self.cols = cols
        self.data_fn = data_fn

    def generate(self, path: Path):
        with path.open("wb") as f:
            # Write row and column counts
            f.write(struct.pack("i", self.rows))
            f.write(struct.pack("i", self.cols))

            # Write matrix elements as bytes
            f.write(struct.pack("I" * self.rows * self.cols, *self.data_fn()))


class Task:
    def __init__(self, a_matrix: Matrix, b_matrix: Matrix):
        self.a_matrix = a_matrix
        self.b_matrix = b_matrix

    def generate(self, path: Path):
        path.mkdir(exist_ok=True)
        self.a_matrix.generate(path / "a.bin")
        self.b_matrix.generate(path / "b.bin")
        a_md5 = md5sum(path / "a.bin")
        b_md5 = md5sum(path / "b.bin")

        try:
            a_md5_old, b_md5_old = None, None
            with (path / "a.bin.md5").open("r") as f:
                a_md5_old = f.read().strip()
            with (path / "b.bin.md5").open("r") as f:
                b_md5_old = f.read().strip()
            if a_md5 == a_md5_old and b_md5 == b_md5_old:
                return
        except:
            pass

        run_oracle(path / "a.bin", path / "b.bin", path / "ref.bin")

        md5sum_file(path / "a.bin")
        md5sum_file(path / "b.bin")
        # md5sum_file(path / "ref.bin")


class TestSpec:
    def __init__(self, name: str):
        self.path = tests_dir / name
        self.tasks: List[Task] = []

    def add_task(self, task: Task):
        self.tasks.append(task)

    def generate(self):
        self.path.mkdir(exist_ok=True, parents=True)
        with (self.path / "input.txt").open("w") as f:
            f.write(str(len(self.tasks)) + "\n")
            for i, task in enumerate(self.tasks):
                task_path = self.path / f"task{i}"
                task.generate(task_path)
                rel_task_dir = os.path.relpath(task_path, self.path)
                f.write(f"./{rel_task_dir}\n")


def Test(name: str = None, seed: int = 42):
    def decorator(func):
        nonlocal name
        name = name or str(func.__name__)
        random.seed(seed)
        np.random.seed(seed)

        test = TestSpec(name)
        func(test)
        all_tests[name] = test

    return decorator
