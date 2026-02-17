#!/usr/bin/env python
"""
Convenience script to run theWheel Python tests

This script helps locate the pythewheel module and run pytest with
appropriate settings.

Usage:
    python run_tests.py              # Run all tests
    python run_tests.py -v           # Verbose output
    python run_tests.py -m unit      # Run only unit tests
    python run_tests.py --cov        # Run with coverage report
"""

import sys
import os
import subprocess


def find_pythewheel_module():
    """Search for the built pythewheel module"""
    project_root = os.path.dirname(os.path.abspath(__file__))

    search_paths = [
        os.path.join(project_root, 'src', 'out', 'build', 'x64-debug', 'python', 'Debug'),
        os.path.join(project_root, 'src', 'out', 'build', 'x64-debug', 'python'),
        os.path.join(project_root, 'src', 'out', 'build', 'x64-release', 'python', 'Release'),
        os.path.join(project_root, 'src', 'out', 'build', 'x64-release', 'python'),
        os.path.join(project_root, 'build', 'python'),
        os.path.join(project_root, 'build', 'Debug', 'python'),
        os.path.join(project_root, 'build', 'Release', 'python'),
    ]

    for path in search_paths:
        # Look for .pyd (Windows) or .so (Linux/Mac) files
        if os.path.exists(path):
            try:
                files = os.listdir(path)
                if any(f.startswith('pythewheel') and (f.endswith('.pyd') or f.endswith('.so')) for f in files):
                    return path
            except OSError:
                continue

    return None


def main():
    """Main entry point"""
    # Find the module
    module_path = find_pythewheel_module()

    # Set up environment
    env = os.environ.copy()

    if module_path:
        print(f"Found pythewheel module at: {module_path}")
        # Add to Python path for subprocess
        if 'PYTHONPATH' in env:
            env['PYTHONPATH'] = module_path + os.pathsep + env['PYTHONPATH']
        else:
            env['PYTHONPATH'] = module_path
    else:
        print("WARNING: Could not locate pythewheel module.")
        print("Please build the project first using CMake:")
        print("  cd src")
        print("  cmake --preset x64-debug")
        print("  cmake --build out/build/x64-debug")
        print()
        response = input("Continue anyway? (y/n): ")
        if response.lower() != 'y':
            return 1

    # Prepare pytest arguments
    pytest_args = ['pytest']

    # Add any command-line arguments passed to this script
    if len(sys.argv) > 1:
        pytest_args.extend(sys.argv[1:])
    else:
        # Default arguments
        pytest_args.extend(['-v', '--tb=short'])

    print(f"\nRunning: {' '.join(pytest_args)}\n")

    # Run pytest with modified environment
    result = subprocess.run(pytest_args, env=env)

    return result.returncode


if __name__ == '__main__':
    sys.exit(main())
