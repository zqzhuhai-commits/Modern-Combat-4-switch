#!/usr/bin/env python3
"""Verify the current 孤侠魅影 frontend artifacts."""
from pathlib import Path
import runpy
runpy.run_path(str(Path(__file__).resolve().parents[1]/'frontend/tools/verify.py'),run_name='__main__')
