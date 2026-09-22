from pathlib import Path

root = Path(r"C:\Users\DELL\Downloads\Automated_Railway_Gate_System_IoT")
md_path = root / "docs" / "Automated_Railway_Gate_System_IoT_Documentation.md"
pdf_path = root / "docs" / "Automated_Railway_Gate_System_IoT_Documentation.pdf"

text = md_path.read_text(encoding="utf-8")
lines = text.splitlines()


def escape_pdf_text(s: str) -> str:
    s = s.replace("\\", "\\\\")
    s = s.replace("(", "\\(")
    s = s.replace(")", "\\)")
    return s


pages = []
chunk = []
for line in lines:
    if len(chunk) >= 42:
        pages.append(chunk)
        chunk = []
    chunk.append(line)
if chunk:
    pages.append(chunk)

if not pages:
    pages = [["No content"]]

objects = []

# object 1: catalog
objects.append(b"<< /Type /Catalog /Pages 2 0 R >>")

# object 2: pages tree
page_refs = []
for i in range(len(pages)):
    page_num = 3 + (i * 2)
    page_refs.append(f"{page_num} 0 R")
objects.append(b"<< /Type /Pages /Kids [" + b" ".join([f"{i} 0 R".encode() for i in page_refs]) + b"] /Count " + str(len(pages)).encode() + b" >>")

# Build page objects and contents objects later
page_objs = []
content_objs = []

for i, page_lines in enumerate(pages):
    page_obj_num = 3 + (i * 2)
    content_obj_num = page_obj_num + 1
    stream = []
    stream.append(b"BT")
    stream.append(b"/F1 9 Tf")
    stream.append(b"72 760 Td")
    for line_no, line in enumerate(page_lines):
        y = 760 - (line_no * 12)
        stream.append(f"{y} 0 Td".encode())
        stream.append(f"({escape_pdf_text(line)}) Tj".encode())
    stream.append(b"ET")
    stream_data = b"\n".join(stream)
    page_obj = f"<< /Type /Page /Parent 2 0 R /MediaBox [0 0 612 792] /Contents {content_obj_num} 0 R /Resources << /Font << /F1 5 0 R >> >> >>"
    page_objs.append((page_obj_num, page_obj))
    content_objs.append((content_obj_num, stream_data))

# Build object list in order: 1 catalog, 2 pages, then pages/contents/font
objects = [b"1 0 obj << /Type /Catalog /Pages 2 0 R >>",
           b"2 0 obj << /Type /Pages /Kids ["]

# page references will be inserted later after page objects are known
page_refs = []
for i, _ in enumerate(pages):
    page_num = 3 + (i * 2)
    page_refs.append(f"{page_num} 0 R")
objects[1] = b"2 0 obj << /Type /Pages /Kids [" + b" ".join([ref.encode() for ref in page_refs]) + f" ] /Count {len(pages)} >>".encode()

# add page and content objects
for page_num, page_obj in page_objs:
    objects.append(f"{page_num} 0 obj {page_obj}".encode())
for content_num, stream_data in content_objs:
    objects.append(f"{content_num} 0 obj << /Length {len(stream_data)} >> stream\n".encode() + stream_data + b"\nendstream")

# font object
objects.append(b"5 0 obj << /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>")

# build pdf bytes
pdf_parts = [b"%PDF-1.4\n"]
offsets = [0]
for obj in objects:
    offsets.append(len(b"".join(pdf_parts)))
    pdf_parts.append(obj + b"\n")

# xref
xref_position = len(b"".join(pdf_parts))
pdf_parts.append(f"xref\n0 {len(objects)+1}\n".encode())
pdf_parts.append(b"0000000000 65535 f \n")
for offset in offsets[1:]:
    pdf_parts.append(f"{offset:010d} 00000 n \n".encode())
pdf_parts.append(f"trailer\n<< /Size {len(objects)+1} /Root 1 0 R >>\nstartxref\n{xref_position}\n%%EOF\n".encode())

pdf_path.write_bytes(b"".join(pdf_parts))
print(f"Created {pdf_path}")
