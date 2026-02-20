"""
MFC CArchive binary parser for theWheel .spx files.

Reads serialized CSpace/CNode/CNodeLink data from MFC's binary archive format
and returns a Python object tree.
"""

import struct
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional


@dataclass
class SpxNodeLink:
    """Represents a CNodeLink (directed weighted edge)."""
    weight: float = 0.0
    target: Optional['SpxNode'] = None
    is_stabilizer: bool = False


@dataclass
class SpxNode:
    """Represents a CNode in the semantic network."""
    name: str = ""
    description: str = ""
    image_filename: str = ""
    url: str = ""
    class_name: str = ""
    primary_activation: float = 0.0
    secondary_activation: float = 0.0
    position: tuple = (0.0, 0.0, 0.0)
    children: list = field(default_factory=list)
    links: list = field(default_factory=list)
    # Eevorg-specific fields (populated only for Eevorg/CEevorg nodes)
    eevorg_max_val: int = 0
    eevorg_max_rule: int = 0
    eevorg_score: int = 0
    eevorg_rules: bytes = b""
    is_eevorg: bool = False


class MfcArchiveReader:
    """Reads MFC CArchive binary format, tracking class and object references."""

    # MFC archive tags
    NULL_TAG = 0x0000
    NEW_CLASS_TAG = 0xFFFF
    OLD_CLASS_TAG = 0x8000
    BIG_OBJECT_TAG = 0x7FFF
    BIG_CLASS_TAG = 0x7FFE

    def __init__(self, data: bytes, debug: bool = False):
        self.data = data
        self.pos = 0
        self.debug = debug
        self.hit_unknown_class = False  # set when we can't continue parsing
        # Shared map: indices for both classes and objects
        # Index 0 is reserved (NULL), counting starts at 1
        self.map_entries = [None]  # index 0 = NULL
        self.class_schemas = {}  # class_name -> schema

    def read_byte(self) -> int:
        val = self.data[self.pos]
        self.pos += 1
        return val

    def read_word(self) -> int:
        val = struct.unpack_from('<H', self.data, self.pos)[0]
        self.pos += 2
        return val

    def read_dword(self) -> int:
        val = struct.unpack_from('<I', self.data, self.pos)[0]
        self.pos += 4
        return val

    def read_float(self) -> float:
        val = struct.unpack_from('<f', self.data, self.pos)[0]
        self.pos += 4
        return val

    def read_double(self) -> float:
        val = struct.unpack_from('<d', self.data, self.pos)[0]
        self.pos += 8
        return val

    def read_bool(self) -> bool:
        """Read MFC BOOL (4 bytes, DWORD)."""
        return self.read_dword() != 0

    def read_bytes(self, count: int) -> bytes:
        val = self.data[self.pos:self.pos + count]
        self.pos += count
        return val

    def read_cstring(self) -> str:
        """Read an MFC CString in ANSI format."""
        length = self.read_byte()
        if length == 0xFF:
            length = self.read_word()
            if length == 0xFFFF:
                length = self.read_dword()
        if length == 0:
            return ""
        raw = self.read_bytes(length)
        return raw.decode('latin-1')

    def read_count(self) -> int:
        """Read MFC WriteCount format (WORD, or WORD 0xFFFF + DWORD)."""
        count = self.read_word()
        if count == 0xFFFF:
            count = self.read_dword()
        return count

    def read_object(self) -> tuple:
        """
        Read an MFC CObject* from the archive.

        Returns (class_name, schema, object) where object is:
        - None for NULL pointers
        - An existing object for back-references
        - A new placeholder that the caller should populate
        """
        tag = self.read_word()

        if tag == self.NULL_TAG:
            return None, 0, None

        if tag == self.NEW_CLASS_TAG:
            # New class: schema WORD, name length WORD, name bytes
            schema = self.read_word()
            name_len = self.read_word()
            class_name = self.read_bytes(name_len).decode('ascii')

            if self.debug:
                print(f"[0x{self.pos:04x}] NEW CLASS: {class_name} schema={schema} "
                      f"(map idx={len(self.map_entries)})")

            # Register class in map
            self.map_entries.append(('class', class_name))
            self.class_schemas[class_name] = schema

            # Create and register the new object
            obj = self._create_object(class_name, schema)
            self.map_entries.append(obj)

            if obj is None:
                # Unknown class - can't deserialize its data, must abort
                self.hit_unknown_class = True
                return class_name, schema, None

            # Deserialize object data
            self._deserialize_object(obj, class_name, schema)
            return class_name, schema, obj

        if tag & self.OLD_CLASS_TAG:
            # Known class, new object
            class_index = tag & 0x7FFF
            if self.debug:
                print(f"[0x{self.pos:04x}] OLD CLASS tag=0x{tag:04x} -> class idx={class_index} "
                      f"(obj map idx={len(self.map_entries)})")
            entry = self.map_entries[class_index]
            if isinstance(entry, tuple) and entry[0] == 'class':
                class_name = entry[1]
            else:
                raise ValueError(f"Expected class at index {class_index}, got {entry}")

            schema = self.class_schemas[class_name]

            # Create and register the new object
            obj = self._create_object(class_name, schema)
            self.map_entries.append(obj)

            if obj is None:
                self.hit_unknown_class = True
                return class_name, schema, None

            # Deserialize object data
            self._deserialize_object(obj, class_name, schema)
            return class_name, schema, obj

        # Object back-reference (tag is the object index)
        obj_index = tag
        if tag == self.BIG_OBJECT_TAG:
            obj_index = self.read_dword()

        if self.debug:
            print(f"[0x{self.pos:04x}] BACK-REF to object idx={obj_index}")

        if obj_index >= len(self.map_entries):
            raise ValueError(f"Object back-reference index {obj_index} out of range "
                             f"(max {len(self.map_entries) - 1}) at offset 0x{self.pos:04x}")
        obj = self.map_entries[obj_index]
        if isinstance(obj, tuple):
            raise ValueError(f"Expected object at index {obj_index}, got class entry {obj}")
        return type(obj).__name__, 0, obj

    # Eevorg is a CNode subclass that calls CNode::Serialize then adds its own fields
    CNODE_SUBCLASSES = {'Eevorg', 'CEevorg'}

    def _create_object(self, class_name: str, schema: int):
        """Create an empty object of the given MFC class."""
        if class_name == 'CNode' or class_name in self.CNODE_SUBCLASSES:
            return SpxNode()
        if class_name == 'CNodeLink':
            return SpxNodeLink()
        return None  # unknown class - can't deserialize

    def _deserialize_object(self, obj, class_name: str, schema: int):
        """Deserialize object data based on class and schema."""
        if class_name == 'CNode':
            self._deserialize_node(obj, schema)
        elif class_name in self.CNODE_SUBCLASSES:
            self._deserialize_eevorg_auto(obj)
        elif class_name == 'CNodeLink':
            self._deserialize_nodelink(obj, schema)
        else:
            raise ValueError(f"Cannot deserialize: {class_name}")

    def _deserialize_eevorg_auto(self, obj: SpxNode):
        """Auto-detect Eevorg format: standalone (CObject) or CNode subclass.

        Two versions exist:
        - Original (1995): extends CObject, serializes 3 WORDs + CByteArray
        - theWheel: extends CNode, serializes CNode fields + 3 WORDs + CByteArray

        We try standalone first (smaller, easier to validate), and if the
        resulting rule count doesn't match maxRule+1, try CNode subclass.
        """
        saved_pos = self.pos
        saved_map_len = len(self.map_entries)

        # Try standalone: 3 WORDs + CByteArray
        max_val = self.read_word()
        max_rule = self.read_word()
        score = self.read_word()
        rule_count = self.read_count()

        # Validate: rule_count should be close to maxRule+1 (mutations add
        # 3 rules per maxVal increment, so generous bound is maxRule + 3*maxVal + 16)
        expected_max = max(max_rule + 3 * max_val + 16, 32)
        standalone_ok = (0 < rule_count <= expected_max
                         and max_val < 100
                         and max_rule < 1000)

        if standalone_ok:
            obj.is_eevorg = True
            obj.eevorg_max_val = max_val
            obj.eevorg_max_rule = max_rule
            obj.eevorg_score = score
            obj.eevorg_rules = self.read_bytes(rule_count)
            if self.debug:
                print(f"  [0x{saved_pos:04x}] Eevorg (standalone): "
                      f"maxVal={max_val} maxRule={max_rule} score={score} "
                      f"rules={rule_count} bytes")
            return

        # Standalone didn't look right — restore and try CNode subclass
        self.pos = saved_pos
        # Remove any map entries added during the failed standalone attempt
        del self.map_entries[saved_map_len:]

        node_schema = self.class_schemas.get('CNode', 6)
        self._deserialize_node(obj, node_schema)
        self._deserialize_eevorg_fields(obj)
        if self.debug:
            print(f"  [0x{saved_pos:04x}] Eevorg (CNode subclass)")

    def _deserialize_eevorg_fields(self, node: SpxNode):
        """Deserialize Eevorg-specific fields after CNode::Serialize."""
        start_pos = self.pos
        node.is_eevorg = True
        # Original Eevorg stores as WORDs: maxVal, maxRule, score
        node.eevorg_max_val = self.read_word()
        node.eevorg_max_rule = self.read_word()
        node.eevorg_score = self.read_word()
        # CByteArray::Serialize: WriteCount + raw bytes
        byte_count = self.read_count()
        node.eevorg_rules = self.read_bytes(byte_count)
        if self.debug:
            print(f"  [0x{start_pos:04x}] Eevorg extra: maxVal={node.eevorg_max_val} "
                  f"maxRule={node.eevorg_max_rule} score={node.eevorg_score} "
                  f"rules={byte_count} bytes, end_pos=0x{self.pos:04x}")

    def _deserialize_node(self, node: SpxNode, schema: int):
        """Deserialize CNode fields based on schema version."""
        start_pos = self.pos
        node.name = self.read_cstring()
        node.description = self.read_cstring()
        node.image_filename = self.read_cstring()

        if schema >= 5:
            node.url = self.read_cstring()

        if schema >= 6:
            node.class_name = self.read_cstring()

        if self.debug:
            print(f"  [0x{start_pos:04x}] CNode schema={schema}: "
                  f"name={node.name!r}, desc={node.description[:40]!r}..., "
                  f"img={node.image_filename!r}, url={node.url!r}, "
                  f"class={node.class_name!r}")

        if schema >= 7:
            node.primary_activation = self.read_double()
            node.secondary_activation = self.read_double()
            x = self.read_double()
            y = self.read_double()
            z = self.read_double()
            node.position = (x, y, z)

            # m_pMaxActivator (CNode* - could be NULL or back-ref)
            _cls, _sch, _obj = self.read_object()
            node.primary_activation  # max activator is runtime-only, ignore

            node.secondary_activation  # placeholder
            max_delta = self.read_double()

        if schema >= 8:
            # optSSV placeholder (always NULL in practice)
            _cls, _sch, _obj = self.read_object()

        # Children CObArray
        child_count = self.read_count()
        for _ in range(child_count):
            if self.hit_unknown_class:
                break
            _cls, _sch, child = self.read_object()
            if child is not None:
                node.children.append(child)

        # Links CObArray
        link_count = self.read_count()
        if self.hit_unknown_class:
            return
        for _ in range(link_count):
            if self.hit_unknown_class:
                break
            _cls, _sch, link = self.read_object()
            if link is not None:
                node.links.append(link)

    def _deserialize_nodelink(self, link: SpxNodeLink, schema: int):
        """Deserialize CNodeLink fields based on schema version."""
        start_pos = self.pos
        link.weight = self.read_float()

        # Target is a CNode* (back-reference to already-serialized node)
        _cls, _sch, target = self.read_object()
        link.target = target

        if schema >= 2:
            link.is_stabilizer = self.read_bool()

        # Threshold: weight < 0.01 becomes 0
        if link.weight < 0.01:
            link.weight = 0.0

        if self.debug:
            target_name = link.target.name if link.target else "NULL"
            print(f"  [0x{start_pos:04x}] CNodeLink schema={schema}: "
                  f"w={link.weight:.4f} -> {target_name!r} stab={link.is_stabilizer}")


def parse_spx(filepath: str, debug: bool = False) -> SpxNode:
    """
    Parse an .spx file and return the root SpxNode tree.

    Handles both formats:
    - Direct CNode serialization (starts with 0xFFFF)
    - DWORD schema prefix followed by CNode serialization
    """
    data = Path(filepath).read_bytes()
    reader = MfcArchiveReader(data, debug=debug)

    # Check if file starts with DWORD schema prefix
    first_word = struct.unpack_from('<H', data, 0)[0]
    if first_word != MfcArchiveReader.NEW_CLASS_TAG:
        # Read and skip DWORD schema prefix
        space_schema = reader.read_dword()

    # Read root CNode
    _cls, _sch, root = reader.read_object()

    if root is None:
        raise ValueError("Failed to read root node from .spx file")

    return root


def print_tree(node: SpxNode, indent: int = 0):
    """Debug: print node tree to stdout."""
    prefix = "  " * indent
    class_str = f" [{node.class_name}]" if node.class_name else ""
    print(f"{prefix}- {node.name}{class_str}")
    if node.description:
        desc = node.description[:60] + "..." if len(node.description) > 60 else node.description
        print(f"{prefix}  {desc}")
    if node.links:
        for link in node.links:
            target_name = link.target.name if link.target else "???"
            print(f"{prefix}  -> {target_name} (w={link.weight:.3f})")
    for child in node.children:
        print_tree(child, indent + 1)


if __name__ == "__main__":
    import sys
    sys.setrecursionlimit(10000)
    if len(sys.argv) < 2:
        print("Usage: python spx_parser.py <file.spx>")
        sys.exit(1)
    debug = '--debug' in sys.argv
    filepath = [a for a in sys.argv[1:] if not a.startswith('--')][0]
    root = parse_spx(filepath, debug=debug)

    # Collect all unique nodes (including those only reachable via links)
    all_nodes = {}
    def collect_nodes(node):
        if id(node) in all_nodes:
            return
        all_nodes[id(node)] = node
        for child in node.children:
            collect_nodes(child)
        for link in node.links:
            if link.target:
                collect_nodes(link.target)
    collect_nodes(root)
    eevorg_count = sum(1 for n in all_nodes.values() if n.is_eevorg)
    print(f"Total nodes parsed: {len(all_nodes)}")
    if eevorg_count:
        print(f"  (including {eevorg_count} Eevorg automata)")

    # Show top-level categories
    for child in root.children:
        child_count = len(child.children)
        link_count = len(child.links)
        extras = []
        if child.image_filename:
            extras.append(f"img={child.image_filename}")
        if child.is_eevorg:
            extras.append(f"eevorg maxVal={child.eevorg_max_val}")
        extra_str = f" ({', '.join(extras)})" if extras else ""
        print(f"  {child.name} [{child.class_name}]{extra_str} "
              f"({child_count} children, {link_count} links)")
        for grandchild in child.children:
            gc_children = len(grandchild.children)
            gc_links = len(grandchild.links)
            print(f"    {grandchild.name} [{grandchild.class_name}] "
                  f"({gc_children} children, {gc_links} links)")
