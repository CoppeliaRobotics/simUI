#!/usr/bin/env python3
"""
Convert the widgets XML documentation file into Markdown.

Usage:
    python xml_to_markdown.py widgets.xml widgets.md
"""

import sys
import xml.etree.ElementTree as ET


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def escape_text(text):
    """Escape text so Markdown renders literal <, >, & correctly.

    The XML source encodes these as entities (&lt; etc.), but ElementTree
    decodes them back into the literal characters.  If we emit those raw
    into Markdown, sequences such as <ui> are interpreted as HTML tags
    and disappear from the rendered output.
    """
    return (text.replace("&", "&amp;")
                .replace("<", "&lt;")
                .replace(">", "&gt;"))


def inner_markdown(elem):
    """Return the text content of an element, converting simple HTML tags
    (<b>, <i>, <a href=...>) into their Markdown equivalents, while
    escaping any remaining literal <, >, & characters in the text."""
    if elem is None:
        return ""
    parts = []
    if elem.text:
        parts.append(escape_text(elem.text))
    for child in elem:
        tag = child.tag
        inner = inner_markdown(child)
        if tag == "b":
            parts.append(f"**{inner}**")
        elif tag == "i":
            parts.append(f"*{inner}*")
        elif tag == "a":
            href = child.get("href", "")
            parts.append(f"[{inner}]({href})")
        else:
            parts.append(inner)
        if child.tail:
            parts.append(escape_text(child.tail))
    return "".join(parts).strip()


def escape_md(text):
    """Escape characters that break Markdown tables."""
    if text is None:
        return ""
    return text.replace("|", "\\|").replace("\n", " ").strip()


def format_type(type_elem):
    """Format a <type> element, handling the nested <one-of> case."""
    if type_elem is None:
        return ""
    one_of = type_elem.find("one-of")
    if one_of is not None:
        values = [v.text.strip() for v in one_of.findall("value") if v.text]
        return "one of: " + ", ".join(f"`{v}`" for v in values)
    return escape_md(inner_markdown(type_elem))


# ---------------------------------------------------------------------------
# Main rendering
# ---------------------------------------------------------------------------

def render(root):
    lines = ["# Widgets Reference", ""]

    for el in root.findall("element"):
        name = (el.findtext("name") or "").strip()
        desc = inner_markdown(el.find("description"))

        lines.append(f"## `<{name}>`")
        lines.append("")

        if desc:
            lines.append(desc)
            lines.append("")

        # See-also
        see_also = el.find("see-also")
        if see_also is not None:
            others = [
                (e.text or "").strip()
                for e in see_also.findall("element")
                if e.text
            ]
            if others:
                joined = ", ".join(f"`<{o}>`" for o in others)
                lines.append(f"**See also:** {joined}")
                lines.append("")

        # Attributes table
        attrs_elem = el.find("attributes")
        if attrs_elem is not None:
            attributes = attrs_elem.findall("attribute")
            if attributes:
                lines.append("| Attribute | Type | Default | Description |")
                lines.append("|-----------|------|---------|-------------|")

                for a in attributes:
                    a_name = (a.findtext("name") or "").strip()
                    a_type = format_type(a.find("type"))

                    default_elem = a.find("default")
                    a_default = ""
                    if default_elem is not None:
                        a_default = escape_md(inner_markdown(default_elem))

                    a_desc = escape_md(inner_markdown(a.find("description")))

                    lines.append(
                        f"| `{a_name}` | {a_type} | {a_default} | {a_desc} |"
                    )
                lines.append("")

    return "\n".join(lines)


def main(argv):
    if len(argv) != 3:
        print("Usage: python xml_to_markdown.py <widgets.xml> <out.md>",
              file=sys.stderr)
        return 1

    xml_path = argv[1]
    out_path = argv[2]

    tree = ET.parse(xml_path)
    markdown = render(tree.getroot())

    with open(out_path, "w", encoding="utf-8") as fh:
        fh.write(markdown)

    print(f"Written to {out_path}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))