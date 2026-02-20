# File Format (.spx)

`.spx` files are MFC CArchive binary archives containing serialized `CNode` networks.

## Schema Versions

| Version | Fields |
|---------|--------|
| 5 | name, description, image, URL, children, links |
| 6 | + class_name |
| 7 | + activation (double), position (3× double), max activator |
| 8 | + optSSV (always NULL) |

## CNodeLink Schema

| Version | Fields |
|---------|--------|
| 1 | weight (float), target (CObject* ref) |
| 2 | + IsStabilizer (BOOL) |

## Parser

A Python parser is available at `scripts/spx_parser.py`:

```python
from scripts.spx_parser import parse_spx
root = parse_spx("data/MySpace.spx")
```
