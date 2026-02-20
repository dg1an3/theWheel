"""
Convert .spx files to markdown.

Reads MFC-serialized CNode trees via spx_parser and generates a markdown
file per top-level category, plus an index file.
"""

import sys
import os
from pathlib import Path
from scripts.spx_parser import parse_spx, SpxNode, SpxNodeLink


def sanitize_filename(name: str) -> str:
    """Convert a node name to a safe filename."""
    safe = name.replace("/", "-").replace("\\", "-").replace(":", "-")
    safe = safe.replace("&&", "and").replace("?", "").replace("*", "")
    safe = safe.replace("<", "").replace(">", "").replace("|", "")
    safe = safe.replace('"', "").strip()
    return safe or "unnamed"


def node_to_markdown(node: SpxNode, depth: int = 0) -> str:
    """Render a SpxNode as markdown at the given heading depth."""
    lines = []
    heading = "#" * min(depth + 1, 6)

    # Name and class
    class_tag = f" *[{node.class_name}]*" if node.class_name else ""
    lines.append(f"{heading} {node.name}{class_tag}")

    # Description
    if node.description:
        lines.append(f"> {node.description}")
        lines.append("")

    # Metadata
    meta = []
    if node.image_filename:
        meta.append(f"**Image:** {node.image_filename}")
    if node.url:
        meta.append(f"**URL:** {node.url}")
    if node.is_eevorg:
        meta.append(f"**Eevorg:** maxVal={node.eevorg_max_val}, "
                     f"maxRule={node.eevorg_max_rule}, "
                     f"rules={len(node.eevorg_rules)}b")
    if meta:
        lines.append(" | ".join(meta))
        lines.append("")

    # Links table
    real_links = [lk for lk in node.links if lk.target and lk.weight > 0]
    if real_links:
        # Sort by weight descending, show top links
        real_links.sort(key=lambda lk: lk.weight, reverse=True)
        shown = real_links[:20]
        lines.append("**Links:**")
        lines.append("")
        lines.append("| Target | Weight |")
        lines.append("|--------|--------|")
        for lk in shown:
            name = lk.target.name or "(unnamed)"
            lines.append(f"| {name} | {lk.weight:.3f} |")
        if len(real_links) > 20:
            lines.append(f"| *...and {len(real_links) - 20} more* | |")
        lines.append("")

    # Children (recursive)
    child_nodes = [c for c in node.children if not c.is_eevorg]
    for child in child_nodes:
        lines.append(node_to_markdown(child, depth + 1))

    return "\n".join(lines)


def convert_spx_to_markdown(spx_path: str, output_dir: str):
    """Convert an .spx file to markdown files in output_dir."""
    root = parse_spx(spx_path)
    out = Path(output_dir)
    out.mkdir(parents=True, exist_ok=True)

    spx_name = Path(spx_path).stem
    index_lines = [f"# {root.name or spx_name}", ""]

    if root.description:
        index_lines.append(f"> {root.description}")
        index_lines.append("")

    index_lines.append(f"*Parsed from `{Path(spx_path).name}`*")
    index_lines.append("")

    # Collect stats
    all_nodes = {}
    def count_nodes(n):
        if id(n) in all_nodes:
            return
        all_nodes[id(n)] = n
        for c in n.children:
            count_nodes(c)
        for lk in n.links:
            if lk.target:
                count_nodes(lk.target)
    count_nodes(root)
    eevorg_count = sum(1 for n in all_nodes.values() if n.is_eevorg)
    index_lines.append(f"**Total nodes:** {len(all_nodes)}")
    if eevorg_count:
        index_lines.append(f"  (including {eevorg_count} Eevorg automata)")
    index_lines.append("")

    # One markdown file per top-level child
    index_lines.append("## Categories")
    index_lines.append("")

    for child in root.children:
        if child.is_eevorg and not child.name:
            continue  # Skip unnamed Eevorg nodes in index

        safe_name = sanitize_filename(child.name or "eevorg")
        md_filename = f"{safe_name}.md"
        md_path = out / md_filename

        # Generate the category markdown
        md_content = node_to_markdown(child, depth=0)
        md_path.write_text(md_content, encoding="utf-8")

        # Add to index
        child_count = len(child.children)
        link_count = len([lk for lk in child.links if lk.target and lk.weight > 0])
        class_str = f" *[{child.class_name}]*" if child.class_name else ""
        index_lines.append(f"- [{child.name}{class_str}]({md_filename}) "
                          f"({child_count} children, {link_count} links)")

    index_lines.append("")

    # Write index
    index_path = out / "index.md"
    index_path.write_text("\n".join(index_lines), encoding="utf-8")

    print(f"Generated {len(root.children)} category files + index.md in {out}/")
    return out


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python -m scripts.spx_to_markdown <file.spx> [output_dir]")
        sys.exit(1)

    spx_path = sys.argv[1]
    output_dir = sys.argv[2] if len(sys.argv) > 2 else "output"
    convert_spx_to_markdown(spx_path, output_dir)
