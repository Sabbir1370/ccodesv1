#pragma once
#include <string>
#include <iostream>

class SourceLocation
{
private:
    int line;
    int column;
    std::string filename;

public:
    SourceLocation(int line = -1, int col = -1, const std::string &file = "")
        : line(line), column(col), filename(file) {}

    int getLine() const { return line; }
    int getColumn() const { return column; }
    const std::string &getFilename() const { return filename; }

    bool isValid() const { return line > 0; }

    std::string toString() const
    {
        if (filename.empty())
        {
            return "line " + std::to_string(line) + ", col " + std::to_string(column);
        }
        return filename + ":" + std::to_string(line) + ":" + std::to_string(column);
    }

    friend std::ostream &operator<<(std::ostream &os, const SourceLocation &loc)
    {
        os << loc.toString();
        return os;
    }
};