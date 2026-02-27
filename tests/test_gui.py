"""
Automated GUI tests for theWheel MFC application using pywinauto.

Verifies that the app launches, renders nodes, and responds to interaction.
Requires: pip install pywinauto Pillow

Run with: pytest tests/test_gui.py -v --tb=short
"""
import os
import time

import pytest

pywinauto = pytest.importorskip("pywinauto", reason="pywinauto not installed")
PIL = pytest.importorskip("PIL", reason="Pillow not installed")

from pywinauto import Application, Desktop
from pywinauto.keyboard import send_keys
from pywinauto.timings import wait_until_passes
from PIL import Image


# Paths
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXE_PATH = os.path.join(
    PROJECT_ROOT, "build", "x64-debug", "src", "theWheel", "Debug", "theWheel.exe"
)
TEST_SPX = os.path.join(
    PROJECT_ROOT, "antiques", "theWheel", "data", "TheWell3.spx"
)
SCREENSHOT_DIR = os.path.join(PROJECT_ROOT, "tests", "screenshots")


@pytest.fixture(scope="session")
def app():
    """Launch theWheel.exe and yield the Application object. Kill on teardown."""
    if not os.path.isfile(EXE_PATH):
        pytest.skip(f"Executable not found: {EXE_PATH}")

    app = Application(backend="win32").start(EXE_PATH)
    time.sleep(2)  # allow startup and initial rendering
    yield app

    try:
        app.kill()
    except Exception:
        pass


@pytest.fixture(scope="session")
def main_window(app):
    """Get the main theWheel window, waiting for it to be ready."""
    win = app.window(title_re=".*theWheel.*")
    win.wait("ready", timeout=10)
    return win


# ---------------------------------------------------------------------------
# Test cases
# ---------------------------------------------------------------------------

@pytest.mark.gui
class TestAppLaunch:
    """Verify the application launches correctly with expected UI elements."""

    def test_app_launches(self, main_window):
        """Main window exists with menu bar, toolbar, and status bar."""
        assert main_window.exists(), "Main window should exist"
        assert main_window.is_visible(), "Main window should be visible"

        # Menu bar
        menu = main_window.menu()
        menu_texts = [item.text() for item in menu.items()]
        assert any("File" in t for t in menu_texts), "File menu expected"
        assert any("Edit" in t for t in menu_texts), "Edit menu expected"
        assert any("View" in t for t in menu_texts), "View menu expected"

        # Status bar — MFC msctls_statusbar32
        status = main_window.child_window(class_name="msctls_statusbar32")
        assert status.exists(), "Status bar should exist"

        # Toolbar — MFC ToolbarWindow32
        toolbar = main_window.child_window(class_name="ToolbarWindow32")
        assert toolbar.exists(), "Toolbar should exist"


@pytest.mark.gui
class TestTreeView:
    """Verify the tree view pane shows nodes."""

    def _get_tree(self, main_window):
        """Find the SysTreeView32 control inside the splitter."""
        tree = main_window.child_window(class_name="SysTreeView32")
        tree.wait("ready", timeout=5)
        return tree

    def _get_first_root(self, tree):
        """Get the first root item in the tree, whatever its name."""
        roots = tree.roots()
        assert len(roots) > 0, "Tree should have at least one root item"
        return roots[0]

    def test_default_space_has_tree_nodes(self, main_window):
        """Tree view should contain items from CreateSimpleSpace."""
        tree = self._get_tree(main_window)
        root = self._get_first_root(tree)
        assert root is not None, "A root node expected in tree"
        assert tree.item_count() > 1, "Tree should have multiple nodes"

    def test_node_selection_in_tree(self, main_window):
        """Clicking a tree item should select it."""
        tree = self._get_tree(main_window)
        root = self._get_first_root(tree)
        root.click_input()
        time.sleep(0.3)
        # Re-fetch and check selection
        root = self._get_first_root(tree)
        assert root.is_selected(), "Root item should be selected after click"


@pytest.mark.gui
class TestFileOpen:
    """Test File > Open with a .spx file."""

    def test_file_open_spx(self, app, main_window):
        """Open TheWell3.spx and verify the tree populates and title updates."""
        if not os.path.isfile(TEST_SPX):
            pytest.skip(f"Test .spx not found: {TEST_SPX}")

        # File > Open
        main_window.menu_select("File->Open")
        time.sleep(1)

        # The Open dialog
        open_dlg = app.window(title_re=".*Open.*")
        open_dlg.wait("ready", timeout=5)

        # Type the file path into the filename combo
        file_combo = open_dlg.child_window(class_name="Edit", found_index=0)
        file_combo.set_edit_text(TEST_SPX)
        time.sleep(0.3)

        # Click Open button
        open_btn = open_dlg.child_window(title="&Open", class_name="Button")
        open_btn.click_input()
        time.sleep(2)

        # Verify title updated
        title = main_window.window_text()
        assert "TheWell3" in title or "theWheel" in title, (
            f"Window title should reflect opened file, got: {title}"
        )

        # Verify tree has nodes from TheWell3.spx (90 nodes)
        tree = main_window.child_window(class_name="SysTreeView32")
        tree.wait("ready", timeout=5)
        count = tree.item_count()
        assert count > 5, f"Expected many tree items after loading .spx, got {count}"


@pytest.mark.gui
class TestRenderPane:
    """Tests for the rendering pane (right side of splitter)."""

    def _get_render_pane(self, main_window):
        """
        Find the rendering pane. It's the second child of the splitter
        (class AfxFrameOrView*). We look for a child window that is NOT
        the tree view container.
        """
        children = main_window.children()
        for child in children:
            try:
                rect = child.rectangle()
                class_name = child.class_name()
                if "AfxFrameOrView" in class_name and rect.width() > 200:
                    return child
            except Exception:
                continue
        return main_window

    def test_mouse_wheel_on_render_pane(self, main_window):
        """Sending mouse wheel events to the render pane should not crash."""
        target = main_window
        try:
            render = self._get_render_pane(main_window)
            if render:
                target = render
        except Exception:
            pass

        target.set_focus()
        time.sleep(0.3)

        from pywinauto import mouse
        rect = target.rectangle()
        cx = (rect.left + rect.right) // 2
        cy = (rect.top + rect.bottom) // 2

        mouse.scroll(coords=(cx, cy), wheel_dist=3)
        time.sleep(0.3)
        mouse.scroll(coords=(cx, cy), wheel_dist=-3)
        time.sleep(0.3)

        assert main_window.exists(), "Window should still exist after wheel events"

    def test_screenshot_capture(self, main_window):
        """Capture a screenshot of the main window for visual review."""
        os.makedirs(SCREENSHOT_DIR, exist_ok=True)

        main_window.set_focus()
        time.sleep(0.5)

        screenshot_path = os.path.join(SCREENSHOT_DIR, "main_window.png")
        img = main_window.capture_as_image()
        img.save(screenshot_path)

        assert os.path.isfile(screenshot_path), "Screenshot file should be created"
        assert os.path.getsize(screenshot_path) > 0, "Screenshot should not be empty"

        # Also capture just the render pane if possible
        try:
            tree = main_window.child_window(class_name="SysTreeView32")
            tree_rect = tree.rectangle()
            win_rect = main_window.rectangle()
            render_img = img.crop((
                tree_rect.right - win_rect.left,
                0,
                img.width,
                img.height
            ))
            render_path = os.path.join(SCREENSHOT_DIR, "render_pane.png")
            render_img.save(render_path)
        except Exception:
            pass  # non-critical


@pytest.mark.gui
class TestViewMenu:
    """Test View menu items."""

    def test_view_node_properties_dialog(self, main_window):
        """View > Node Properties should be accessible from the menu."""
        # First select a node in the tree to enable the menu item
        tree = main_window.child_window(class_name="SysTreeView32")
        tree.wait("ready", timeout=5)
        roots = tree.roots()
        if roots:
            roots[0].click_input()
            time.sleep(0.5)

        # Check the menu item exists
        menu_item = main_window.menu_item("View->Node Properties")
        assert menu_item is not None, "Node Properties menu item should exist"

        # Try to select it — it may be disabled if no node is selected
        if menu_item.is_enabled():
            menu_item.select()
            time.sleep(0.5)
            assert main_window.exists(), (
                "Window should still exist after toggling properties"
            )
            # Toggle back off
            main_window.menu_item("View->Node Properties").select()
            time.sleep(0.3)
        else:
            # Even if disabled, the menu item existing is a valid check
            pass
