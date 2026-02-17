"""
Pytest configuration and shared fixtures for theWheel tests
"""
import sys
import os
import pytest


def pytest_configure(config):
    """
    Configure pytest and set up the environment
    """
    # Add custom markers
    config.addinivalue_line("markers", "unit: Unit tests for individual components")
    config.addinivalue_line("markers", "integration: Integration tests across components")
    config.addinivalue_line("markers", "activation: Tests for activation spreading")
    config.addinivalue_line("markers", "layout: Tests for spatial layout")
    config.addinivalue_line("markers", "slow: Slow-running tests")


@pytest.fixture(scope="session", autouse=True)
def setup_python_path():
    """
    Automatically add the build directory to Python path
    This fixture runs once per test session
    """
    # Try to find the pythewheel module in common build locations
    project_root = os.path.dirname(os.path.dirname(__file__))

    search_paths = [
        os.path.join(project_root, 'src', 'out', 'build', 'x64-debug', 'python', 'Debug'),
        os.path.join(project_root, 'src', 'out', 'build', 'x64-debug', 'python'),
        os.path.join(project_root, 'src', 'out', 'build', 'x64-release', 'python', 'Release'),
        os.path.join(project_root, 'src', 'out', 'build', 'x64-release', 'python'),
        os.path.join(project_root, 'build', 'python'),
        os.path.join(project_root, 'build', 'Debug', 'python'),
        os.path.join(project_root, 'build', 'Release', 'python'),
    ]

    module_found = False
    for path in search_paths:
        if os.path.exists(path):
            sys.path.insert(0, path)
            module_found = True
            print(f"\nAdded to Python path: {path}")
            break

    if not module_found:
        print("\nWARNING: Could not find pythewheel module in standard build locations.")
        print("Please build the project first using CMake.")
        print(f"Searched in:")
        for path in search_paths:
            print(f"  - {path}")

    yield

    # Cleanup (if needed)


@pytest.fixture
def verify_module():
    """
    Fixture that verifies pythewheel module can be imported
    Use this in tests that require the module
    """
    try:
        import pythewheel
        return pythewheel
    except ImportError as e:
        pytest.skip(f"pythewheel module not available: {e}")
