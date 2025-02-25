#ifndef SRC_SERVER_TABULATOR_H
#define SRC_SERVER_TABULATOR_H

#include <string>
#include <unordered_map>
#include <vector>

namespace tabulator
{
/**
 * make drawing table easy
 */
class Table
{
public:
  enum class Color
  {
    DEFAULT,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE
  };

  enum class Alignment
  {
    Left,
    Right,
    Center
  };

private:
  std::vector<std::string> m_headers;
  std::vector<std::vector<std::string> > m_rows;
  std::vector<std::string> m_footer;
  std::vector<size_t> m_columnWidths;
  char m_borderChar = ' ';
  char m_horizontalChar = '-';
  char m_cornerChar = '+';
  size_t m_padding = 1;
  std::unordered_map<Color, std::string> m_colorMap
      = { { Color::DEFAULT, "\033[0m" }, { Color::RED, "\033[31m" },
          { Color::GREEN, "\033[32m" },  { Color::YELLOW, "\033[33m" },
          { Color::BLUE, "\033[34m" },   { Color::MAGENTA, "\033[35m" },
          { Color::CYAN, "\033[36m" },   { Color::WHITE, "\033[37m" } };

public:
  /**
   * Set header
   *
   * @param headerRow data to rep in the header
   */
  void setHeaders (const std::vector<std::string> &headerRow);

  /**
   * Add a row of data
   *
   * @param row data to add in the
   */
  void addRow (const std::vector<std::string> &row);

  /**
   * Set footer
   *
   * @param footerRow data to rep in the footer
   */
  void setFooter (const std::vector<std::string> &footerRow);

  /**
   * Sort rows by a specific column does, nothing when the column index is
   * invalid.
   *
   * @param columnIndex
   * @param ascending order
   */
  void sortColumn (size_t columnIndex, bool ascending = true);

  /**
   * Display the table
   *
   * @param alignment
   * @param hc header color
   * @param rc row color
   * @param fc footer color
   */
  void display (Alignment alignment = Alignment::Left, Color hc = Color::CYAN,
                Color rc = Color::DEFAULT, Color fc = Color::BLUE) const;

  /**
   * Set padding around the text in the cells
   *
   * @note does nothing if padding > 10
   * @param padding
   */
  void setPadding (size_t pad);

  /**
   * Set custom border character
   *
   * @param border border character
   * @param horizontal horizontal character
   * @param corner corner character
   */
  void setBorders (char border, char horizontal, char corner);

private:
  /**
   * recalculate column width  after padding change
   */
  void recalculateColumnWidths ();

  // Adjust column widths dynamically based on the content
  void adjustColumnWidths (const std::vector<std::string> &row);

  // Print a row of data
  void printRow (const std::vector<std::string> &row, Alignment alignment,
                 Color color) const;

  // Print divider line
  void printDivider () const;

  // Format a cell with padding and alignment
  std::string formatCell (const std::string &content, size_t width,
                          Alignment alignment) const;
};

} // namespace tabulator

#endif // SRC_SERVER_TABULATOR_H
