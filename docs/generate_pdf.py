from pathlib import Path
from reportlab.lib.pagesizes import letter
from reportlab.pdfgen import canvas
from reportlab.lib.styles import getSampleStyleSheet
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, PageBreak
from reportlab.lib.enums import TA_LEFT
from reportlab.lib import colors

root = Path(r"C:\Users\DELL\Downloads\Automated_Railway_Gate_System_IoT")
md_path = root / "docs" / "Automated_Railway_Gate_System_IoT_Documentation.md"
pdf_path = root / "docs" / "Automated_Railway_Gate_System_IoT_Documentation.pdf"

text = md_path.read_text(encoding="utf-8")
lines = [line.rstrip() for line in text.splitlines() if line.strip()]

styles = getSampleStyleSheet()
styles['Normal'].fontName = 'Helvetica'
styles['Normal'].fontSize = 11
styles['Title'].fontName = 'Helvetica-Bold'
styles['Title'].fontSize = 18
styles['Heading1'].fontName = 'Helvetica-Bold'
styles['Heading1'].fontSize = 14
styles['Heading2'].fontName = 'Helvetica-Bold'
styles['Heading2'].fontSize = 12

story = []
story.append(Paragraph("Automated Railway Gate System using IoT", styles['Title']))
story.append(Spacer(1, 12))

for line in lines:
    if line.startswith('# '):
        story.append(Paragraph(line[2:], styles['Heading1']))
    elif line.startswith('## '):
        story.append(Paragraph(line[3:], styles['Heading2']))
    elif line.startswith('### '):
        story.append(Paragraph(line[4:], styles['Heading2']))
    elif line.startswith('- '):
        story.append(Paragraph(f"• {line[2:]}", styles['Normal']))
    elif line.startswith('A. ') or line.startswith('Q'):
        story.append(Paragraph(line, styles['Normal']))
    else:
        story.append(Paragraph(line, styles['Normal']))
    story.append(Spacer(1, 6))

SimpleDocTemplate(str(pdf_path), pagesize=letter, rightMargin=54, leftMargin=54, topMargin=54, bottomMargin=54).build(story)
print(f"Created {pdf_path}")
