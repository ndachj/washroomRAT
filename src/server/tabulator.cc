#include "tabulator.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace tabulator
{
// Set header
void
Table::setHeaders (const std::vector<std::string> &headerRow)
{
  m_headers = headerRow;
  adjustColumnWidths (m_headers);
}

// Add a row of data
void
Table::addRow (const std::vector<std::string> &row)
{
  m_rows.push_back (row);
  adjustColumnWidths (row);
}

// Set footer
void
Table::setFooter (const std::vector<std::string> &footerRow)
{
  m_footer = footerRow;
  adjustColumnWidths (m_footer);
}

// Sort rows by a specific column (default = ascending order),
// does nothing when the column index is invalid
// @param columnIndex
// @param ascending
void
Table::sortColumn (size_t columnIndex, bool ascending)
{
  if (columnIndex >= m_headers.size ())
    {
      return;
    }

  auto comparator
      = [columnIndex, ascending] (const std::vector<std::string> &a,
                                  const std::vector<std::string> &b) {
          if (columnIndex >= a.size () || columnIndex >= b.size ())
            return false;

          if (ascending)
            return a[columnIndex] < b[columnIndex];
          else
            return a[columnIndex] > b[columnIndex];
        };

  std::sort (m_rows.begin (), m_rows.end (), comparator);
}

// Display the table,
// TIP: row(s) must be present
// @param alignment
// @param hc headers color
// @param rc row(s) color
// @param fc footer color
void
Table::display (Alignment alignment, Color hc, Color rc, Color fc) const
{
  if (m_rows.empty ())
    {
      std::cout << "No data to display.\n";
      return;
    }

  if (!m_headers.empty ())
    {
      printDivider ();
      printRow (m_headers, alignment, hc);
    }

  printDivider ();

  for (const auto &row : m_rows)
    {
      printRow (row, alignment, rc);
    }

  if (!m_footer.empty ())
    {
      printDivider ();
      printRow (m_footer, alignment, fc);
    }

  printDivider ();
}

// Set padding around text in cells,
// TIP: does nothing if padding > 10
// @param padding
void
Table::setPadding (size_t pad)
{
  if (pad > 10)
    {
      return;
    }
  m_padding = pad;
  recalculateColumnWidths ();
}

// Set custom border characters
// @param border border character
// @param horizontal horizontal character
// @param corner corner character
void
Table::setBorders (char border, char horizontal, char corner)
{
  m_borderChar = border;
  m_horizontalChar = horizontal;
  m_cornerChar = corner;
}

// recalculate column width  after padding change
void
Table::recalculateColumnWidths ()
{
  m_columnWidths.assign (m_headers.size (), 0);
  adjustColumnWidths (m_headers);
  for (const auto &row : m_rows)
    {
      adjustColumnWidths (row);
    }
  if (!m_footer.empty ())
    {
      adjustColumnWidths (m_footer);
    }
}

// Adjust column widths dynamically based on the content
void
Table::adjustColumnWidths (const std::vector<std::string> &row)
{
  for (size_t i = 0; i < row.size (); ++i)
    {
      if (i >= m_columnWidths.size ())
        m_columnWidths.push_back (0);
      m_columnWidths[i]
          = std::max (m_columnWidths[i], row[i].size () + m_padding * 2);
    }
}

// Print a row of data
void
Table::printRow (const std::vector<std::string> &row, Alignment alignment,
                 Color color) const
{
  std::cout << m_colorMap.at (color) << m_borderChar;

  for (size_t i = 0; i < row.size (); ++i)
    {
      std::cout << formatCell (row[i], m_columnWidths[i], alignment);
      std::cout << m_borderChar;
    }

  std::cout << m_colorMap.at (Color::DEFAULT) << "\n";
}

// Print divider line
void
Table::printDivider () const
{
  std::cout << m_cornerChar;
  for (const auto &width : m_columnWidths)
    {
      std::cout << std::string (width, m_horizontalChar) << m_cornerChar;
    }
  std::cout << "\n";
}

// Format a cell with padding and alignment
std::string
Table::formatCell (const std::string &content, size_t width,
                   Alignment alignment) const
{
  std::ostringstream oss;
  size_t contentLength = content.size ();
  size_t space = width - contentLength;
  size_t padLeft = m_padding;
  size_t padRight = m_padding;

  if (alignment == Alignment::Left)
    {
      padRight += space - m_padding * 2;
    }
  else if (alignment == Alignment::Right)
    {
      padLeft += space - m_padding * 2;
    }
  else if (alignment == Alignment::Center)
    {
      padLeft += (space - m_padding * 2) / 2;
      padRight += space - m_padding * 2 - (space - m_padding * 2) / 2;
    }

  oss << std::string (padLeft, ' ') << content << std::string (padRight, ' ');
  return oss.str ();
}

} // namespace tabulator
