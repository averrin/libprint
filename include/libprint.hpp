#ifndef __LIBPRINT_H_
#define __LIBPRINT_H_

#include "indicators.hpp"
#include "peglib.h"
#include <chrono>
#include <codecvt>
#include <ctime>
#include <fmt/color.h>
#include <fmt/format.h>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <locale>
#include <map>
#include <regex>
#include <sstream>
#include <stack>
#include <string>

using namespace std::string_literals;
using milliseconds_t = std::chrono::duration<double, std::milli>;
using namespace peg;
using namespace peg::udl;

namespace LibPrint {

class utils {
public:
  static int realLength(std::string s) {
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>()
        .from_bytes(utils::stripEsc(s))
        .size();
  }
  static std::string repeat(std::string s, int n) {
    std::ostringstream os;
    for (int i = 0; i < n; i++)
      os << s;
    return os.str();
  }
  static void br() { fmt::print("\n"); }
  static void up(int n = 1) { fmt::print("\e[{}A", n); }
  static void clearLine() { fmt::print("\e[2K"); }

  static void saveCursor() { fmt::print("\e7"); }
  static void restoreCursor() { fmt::print("\e8"); }

  static void h1(std::string text) {
    fmt::print("\n{:━^80}\n\n", utils::bold(" {} ", text));
  }

  static void h2(std::string text) {
    fmt::print("\n━━{:━<78}\n\n", utils::bold(" {} ", text));
  }

  static void h3(std::string text) {
    fmt::print("──{:─<78}\n", utils::bold(" {} ", text));
  }

  static std::string
  rule(int l = 80,
       fmt::detail::color_type rule_color = fmt::terminal_color::white,
       bool end_line = true, bool thin = false) {
    return fmt::format(
        utils::color(rule_color, utils::bold(thin ? "{:─^{}}{}" : "{:━^{}}{}",
                                             "", l, end_line ? "\n" : "")));
  }

  static std::string
  stacked(int l, int r,
          fmt::detail::color_type l_color = fmt::terminal_color::white,
          fmt::detail::color_type r_color = fmt::color::gray,
          bool end_line = true) {
    return rule(l, l_color, false) + rule(r, r_color, end_line);
  }

  static std::string parse(std::string_view text) {
    parser parser(R"(
        ROOT      <- CONTENT
        CONTENT   <- (ELEMENT / TEXT)*
        ELEMENT   <- $(STAG CONTENT ETAG)
        STAG      <- '<' _ $tag<TAG_NAME> _ ARG? _ '>'
        ETAG      <- '</' _ $tag<TAG_NAME> _ '>'
        TAG_NAME  <- ([a-zA-Z])*
        ARG       <- '='$([^>])*
        TEXT      <- TEXT_DATA
        TEXT_DATA <- ![<] .
        ~_        <- [ \t\r\n]*
   )");
    parser.enable_ast();
    parser.log = [](std::size_t line, std::size_t col, const std::string &msg) {
      std::cerr << "Parse error at " << line << ":" << col << " => " << msg
                << "\n";
    };

    std::shared_ptr<Ast> ast;
    std::string content = "";
    if (parser.parse(text, ast)) {
      ast = parser.optimize_ast(ast);
      // fmt::print(ast_to_s(ast));
      content += process_node(ast);
    }

    return content;
  }

  static std::string
  process_node(std::shared_ptr<Ast> node,
               fmt::text_style parent_style = fmt::text_style{}) {
    if (node->name == "TAG_NAME")
      return "";
    std::string content = "";
    if (node->name == "TEXT_DATA") {
      content += node->token;
    } else if (node->name == "ELEMENT") {
      auto tag = node->nodes[0]->token;
      if (tag == "") {
        tag = node->nodes[0]->nodes[0]->token;
      }
      // fmt::print("pre-ELEMENT -> <{}>???</{}>\n", tag,
      // node->nodes[2]->token);
      auto style = parent_style;
      if (tag == "b") {
        style |= fmt::emphasis::bold;
      } else if (tag == "u") {
        style |= fmt::emphasis::underline;
      } else if (tag == "i") {
        style |= fmt::emphasis::italic;
      } else if (tag == "s") {
        style |= fmt::emphasis::strikethrough;
      } else if (tag == "red") {
        style |= fmt::fg(fmt::terminal_color::red);
      } else if (tag == "black") {
        style |= fmt::fg(fmt::terminal_color::black);
      } else if (tag == "green") {
        style |= fmt::fg(fmt::terminal_color::green);
      } else if (tag == "yellow") {
        style |= fmt::fg(fmt::terminal_color::yellow);
      } else if (tag == "blue") {
        style |= fmt::fg(fmt::terminal_color::blue);
      } else if (tag == "magenta") {
        style |= fmt::fg(fmt::terminal_color::magenta);
      } else if (tag == "cyan") {
        style |= fmt::fg(fmt::terminal_color::cyan);
      } else if (tag == "gray") {
        style |= fmt::fg(fmt::color::gray);
      } else if (tag == "color" || tag == "bgcolor") {
        auto arg = node->nodes[0]->nodes[1]->token;
        std::stringstream str;
        std::string s1 = std::string(arg).substr(2, arg.size() - 2);
        str << s1;
        uint32_t value;
        str >> std::hex >> value;
        if (tag == "bgcolor") {
          style |= fmt::bg(fmt::rgb(value));
        } else {
          style |= fmt::fg(fmt::rgb(value));
        }
      }
      content += process_node(node->nodes[1], style);
      content = fmt::format(style, content);
      // fmt::print("ELEMENT -> <{}>{}</{}>\n", tag, content,
      //            node->nodes[2]->token);
    } else if (node->name == "CONTENT") {
      for (auto child : node->nodes) {
        content += process_node(child, parent_style);
      }
      // fmt::print("{} -> {}\n", node->name, content);
    } else {
      // fmt::print("!{} -> {}\n", node->name, node->token);
    }
    return content;
  }

  template <typename S, typename... Args>
  static std::string color(fmt::detail::color_type c, const S &fmt_string,
                           const Args &... args) {
    return fmt::format(fmt::fg(c), fmt_string,
                       std::forward<const Args &>(args)...);
  }
  template <typename S, typename... Args>
  static std::string bg(fmt::detail::color_type c, const S &fmt_string,
                        const Args &... args) {
    return fmt::format(fmt::bg(c), fmt_string,
                       std::forward<const Args &>(args)...);
  }
  template <typename S, typename... Args>
  static std::string style(fmt::text_style c, const S &fmt_string,
                           const Args &... args) {
    return fmt::format(c, fmt_string, std::forward<const Args &>(args)...);
  }

  template <typename S, typename... Args>
  static std::string red(const S &fmt_string, Args... args) {
    return color(fmt::terminal_color::red, fmt_string,
                 std::forward<Args>(args)...);
  }
  template <typename S, typename... Args>
  static std::string black(const S &fmt_string, Args... args) {
    return color(fmt::terminal_color::black, fmt_string,
                 std::forward<Args>(args)...);
  }
  template <typename S, typename... Args>
  static std::string green(const S &fmt_string, Args... args) {
    return color(fmt::terminal_color::green, fmt_string,
                 std::forward<Args>(args)...);
  }
  template <typename S, typename... Args>
  static std::string yellow(const S &fmt_string, Args... args) {
    return color(fmt::terminal_color::yellow, fmt_string,
                 std::forward<Args>(args)...);
  }
  template <typename S, typename... Args>
  static std::string blue(const S &fmt_string, Args... args) {
    return color(fmt::terminal_color::blue, fmt_string,
                 std::forward<Args>(args)...);
  }
  template <typename S, typename... Args>
  static std::string magenta(const S &fmt_string, Args... args) {
    return color(fmt::terminal_color::magenta, fmt_string,
                 std::forward<Args>(args)...);
  }
  template <typename S, typename... Args>
  static std::string cyan(const S &fmt_string, Args... args) {
    return color(fmt::terminal_color::cyan, fmt_string,
                 std::forward<Args>(args)...);
  }
  template <typename S, typename... Args>
  static std::string gray(const S &fmt_string, Args... args) {
    return color(fmt::color::gray, fmt_string, std::forward<Args>(args)...);
  }
  template <typename S, typename... Args>
  static std::string bold(const S &fmt_string, Args... args) {
    return style(fmt::emphasis::bold, fmt_string, std::forward<Args>(args)...);
  }
  template <typename S, typename... Args>
  static std::string italic(const S &fmt_string, Args... args) {
    return style(fmt::emphasis::italic, fmt_string,
                 std::forward<Args>(args)...);
  }
  template <typename S, typename... Args>
  static std::string underline(const S &fmt_string, Args... args) {
    return style(fmt::emphasis::underline, fmt_string,
                 std::forward<Args>(args)...);
  }
  template <typename S, typename... Args>
  static std::string strikethrough(const S &fmt_string, Args... args) {
    return style(fmt::emphasis::strikethrough, fmt_string,
                 std::forward<Args>(args)...);
  }

  template <typename S, typename... Args>
  static std::string redBg(const S &fmt_string, Args... args) {
    return bg(fmt::terminal_color::red, fmt_string,
              std::forward<Args>(args)...);
  }
  template <typename S, typename... Args>
  static std::string blackBg(const S &fmt_string, Args... args) {
    return bg(fmt::terminal_color::black, fmt_string,
              std::forward<Args>(args)...);
  }
  template <typename S, typename... Args>
  static std::string greenBg(const S &fmt_string, Args... args) {
    return bg(fmt::terminal_color::green, fmt_string,
              std::forward<Args>(args)...);
  }
  template <typename S, typename... Args>
  static std::string yellowBg(const S &fmt_string, Args... args) {
    return bg(fmt::terminal_color::yellow, fmt_string,
              std::forward<Args>(args)...);
  }
  template <typename S, typename... Args>
  static std::string blueBg(const S &fmt_string, Args... args) {
    return bg(fmt::terminal_color::blue, fmt_string,
              std::forward<Args>(args)...);
  }
  template <typename S, typename... Args>
  static std::string magentaBg(const S &fmt_string, Args... args) {
    return bg(fmt::terminal_color::magenta, fmt_string,
              std::forward<Args>(args)...);
  }
  template <typename S, typename... Args>
  static std::string cyanBg(const S &fmt_string, Args... args) {
    return bg(fmt::terminal_color::cyan, fmt_string,
              std::forward<Args>(args)...);
  }
  template <typename S, typename... Args>
  static std::string grayBg(const S &fmt_string, Args... args) {
    return bg(fmt::color::gray, fmt_string, std::forward<Args>(args)...);
  }

  template <typename T>
  static std::string join(const T &array, const std::string &delimiter) {
    std::string res;
    for (auto &element : array) {
      if (!res.empty()) {
        res += delimiter;
      }

      res += element;
    }

    return res;
  }

  template <typename rT, typename iT>
  static std::vector<std::shared_ptr<rT>>
  castObjects(std::vector<std::shared_ptr<iT>> input, bool unique = false) {
    std::vector<std::shared_ptr<rT>> result;
    for (auto input_object : input) {
      if (auto casted_object = std::dynamic_pointer_cast<rT>(input_object)) {
        result.push_back(casted_object);
      }
    }
    if (unique) {
      auto it = std::unique(result.begin(), result.end());
      result.resize(std::distance(result.begin(), it));
    }
    return result;
  }

  static std::vector<std::string> split(std::string strToSplit,
                                        char delimeter) {
    std::stringstream ss(strToSplit);
    std::string item;
    std::vector<std::string> splittedStrings;
    while (std::getline(ss, item, delimeter)) {
      splittedStrings.push_back(item);
    }
    return splittedStrings;
  }

  static std::string stripEsc(std::string str) {
    // std::regex esc_re("\\033\[[0-9;]+m");
    std::regex esc_re("\e\[[0-9;]+m");
    return std::regex_replace(str, esc_re, "");
  }

  static constexpr fmt::detail::color_type NOCOLOR = fmt::detail::color_type{};
}; // namespace LibLog

enum class Align { LEFT, MIDDLE, RIGHT };

class Gutter {
public:
  // TODO: style state stack
  struct State {
    int width = 0;
    std::string content = "";
    Align align = Align::MIDDLE;
    fmt::detail::color_type bgColor = utils::NOCOLOR;
  };
  std::stack<State> states;

  void push(int w) {
    auto s = states.top();
    states.push(State{w, s.content, s.align, s.bgColor});
  }

  void push(std::string c) {
    auto s = states.top();
    states.push(State{s.width, c, s.align, s.bgColor});
  }

  void push(Align a) {
    auto s = states.top();
    states.push(State{s.width, s.content, a, s.bgColor});
  }

  void push(int w, std::string c, Align a) {
    auto s = states.top();
    states.push(State{w, c, a, s.bgColor});
  }

  void push(fmt::detail::color_type bg) {
    auto s = states.top();
    states.push(State{s.width, s.content, s.align, bg});
  }

  void pop(int t = 1) {
    for (auto i = 0; i < t; i++) {
      states.pop();
    }
  }

  void clear() { pop(states.size() - 1); }

  void print() {
    if (!enabled)
      return;

    auto content = states.top().content;
    auto align = states.top().align;
    auto bgColor = states.top().bgColor;
    auto width = states.top().width;
    if (width <= 0)
      return;

    auto f = content.size() - utils::stripEsc(content).size();
    switch (align) {
    case Align::LEFT:
      fmt::print(utils::bg(bgColor, fmt::format("{:<{}}", content, width + f)));
      break;
    case Align::MIDDLE:
      fmt::print(utils::bg(bgColor, fmt::format("{:^{}}", content, width + f)));
      break;
    case Align::RIGHT:
      fmt::print(utils::bg(bgColor, fmt::format("{:>{}}", content, width + f)));
      break;
    } // namespace LibPrint
  }

  bool enabled = true;
  Gutter(std::string c = "", int w = 0, Align a = Align::MIDDLE,
         fmt::detail::color_type bg = fmt::detail::color_type{}) {
    states.push(State{w, c, a, bg});
  }
};

class Printer {
public:
  Gutter gutter = Gutter();
  Gutter leftGutter = Gutter("", 1);
  Gutter rightGutter = Gutter("", 1);
  Gutter indentGutter = Gutter();
  void setGutter(Gutter g) { gutter = g; }
  void removeGutter() { gutter = Gutter(); }
  bool markup = true;
  bool raw = false;

  template <typename S, typename... Args>
  void markLine(fmt::detail::color_type color, std::string mark,
                const S &fmt_string, const Args &... args) {}

  template <typename S, typename... Args>
  void markLine(std::string mark, const S &fmt_string, const Args &... args) {
    // markLine(fmt::detail::color_type{}, fmt_string, std::forward<const Args
    // &>(args)...);
    if (rightGutter.enabled) {
      rightGutter.push(mark);
      println(fmt_string, std::forward<const Args &>(args)...);
      rightGutter.pop();
    } else {
      auto c = gutter.states.top().content;
      gutter.push(mark);
      auto id = utils::realLength(mark) - utils::realLength(c);
      auto iw = indentGutter.states.top().width;
      if (iw > id)
        indentGutter.push(iw - id);
      println(fmt_string, std::forward<const Args &>(args)...);
      if (iw > id)
        indentGutter.pop();
      gutter.pop();
    }
  }
  template <typename S, typename... Args>
  void markLine(fmt::detail::color_type color, const S &fmt_string,
                const Args &... args) {
    if (leftGutter.enabled) {
      leftGutter.push(utils::color(color, "▏"));
      println(fmt_string, std::forward<const Args &>(args)...);
      leftGutter.pop();
    } else {
      gutter.push(color);
      println(fmt_string, std::forward<const Args &>(args)...);
      gutter.pop();
    }
  }

  template <typename S, typename... Args>
  void print(const S &fmt_string, const Args &... args) {
    if (fmt_string == "")
      return;
    std::string msg = fmt_string;
    if (!raw) {
      msg = fmt::format(fmt_string, std::forward<const Args &>(args)...);
    }
    if (markup) {
      msg = utils::parse(msg);
    }
    if (!raw) {
      fmt::print(msg);
    } else {
      std::cout << msg;
    }
  }

  void println() { println(""); }

  void printGutters() {
    leftGutter.print();
    gutter.print();
    rightGutter.print();
    indentGutter.print();
  }

  template <typename S, typename... Args>
  void println(const S &fmt_string, const Args &... args) {
    printGutters();
    print(fmt_string, std::forward<const Args &>(args)...);
    std::cout << std::endl;
  }

  int indent = 0;
  Printer(int i = 0) : indent(i) {
    indentGutter.push(indent);
    leftGutter.enabled = false;
    gutter.enabled = true;
    rightGutter.enabled = false;
  }
};

class NumberedPrinter : public Printer {
public:
  int linenum = 0;
  fmt::detail::color_type fgColor = fmt::rgb(80, 80, 80);

  template <typename S, typename... Args>
  void println(const S &fmt_string, const Args &... args) {
    linenum++;
    if (linenum < 100)
      gutter.states.top().width = 2;
    else
      gutter.states.top().width = 3;
    gutter.states.top().content = utils::color(fgColor, "{}", linenum);
    Printer::println(fmt_string, std::forward<const Args &>(args)...);
  }
  NumberedPrinter() : Printer(1) {
    setGutter(Gutter("", 3));
    gutter.push(Align::RIGHT);
  }
};

class RawPrinter : public Printer {
public:
  RawPrinter() : Printer() {
    markup = false;
    raw = true;
  }
  template <typename S, typename... Args>
  void println(const S &fmt_string, const Args &... args) {
    std::string l;
    auto n = 0;
    std::stringstream ss(fmt_string);
    std::vector<std::string> lines;
    while (std::getline(ss, l, '\n')) {
      lines.push_back(l);
    }
    for (auto l : lines) {
      Printer::println(l);
    }
  }
};

class JSONPrinter : public Printer {
public:
  fmt::detail::color_type fgColor = fmt::rgb(70, 70, 70);
  std::string mark = "//";
  std::string blockMark1 = "/* ";
  std::string blockMark2 = " * ";
  std::string blockMark3 = " */";

  template <typename S, typename... Args>
  void println(const S &fmt_string, const Args &... args) {
    auto m = utils::italic(utils::color(fgColor, mark));
    setGutter(Gutter(m, mark.size()));
    auto line = fmt::format(fmt_string, std::forward<const Args &>(args)...);
    line = utils::italic(utils::color(fgColor, line));
    Printer::println(line);
  }

  template <typename S, typename... Args>
  void printBlock(const S &fmt_string, const Args &... args) {
    std::string l;
    auto line = fmt::format(fmt_string, std::forward<const Args &>(args)...);
    auto n = 0;
    std::stringstream ss(line);
    std::vector<std::string> lines;
    while (std::getline(ss, l, '\n')) {
      lines.push_back(l);
    }
    for (auto l : lines) {
      auto fl = utils::italic(utils::color(fgColor, l));
      if (n == 0) {
        gutter.push(utils::color(fgColor, blockMark1));
      } else if (n == lines.size() - 1) {
        gutter.push(utils::color(fgColor, blockMark3));
      } else {
        gutter.push(utils::color(fgColor, blockMark2));
      }
      Printer::println(fl);
      gutter.pop();
      n++;
    }
  }

  CommentPrinter() : Printer(1) {
    auto m = utils::italic(utils::color(fgColor, mark));
    setGutter(Gutter(m, mark.size()));
    gutter.push(Align::RIGHT);
  }
};

class PrinterWithStatusBar : public Printer {
public:
  int barWidth = 80;
  std::string statusBarChars = utils::yellow("");
  std::string statusBar = "";
  PrinterWithStatusBar() : Printer() { rebuild(); }
  void rebuild() {
    statusBar = utils::repeat(statusBarChars,
                              barWidth / utils::realLength(statusBarChars));
  }
  template <typename S, typename... Args>
  void println(const S &fmt_string, const Args &... args) {
    utils::up();
    utils::clearLine();
    Printer::println(fmt_string, std::forward<const Args &>(args)...);
    Printer::println(statusBar);
  }
  void update() {
    utils::up();
    utils::clearLine();
    Printer::println(statusBar);
  }
};

namespace helpers {
template <typename... Lines> void quote(const Lines &... args) {
  auto p = Printer(1);
  p.gutter.push(1, "┃", Align::MIDDLE);
  std::vector<std::string> lines = {args...};
  for (auto line : lines) {
    p.println(line);
  }
}

template <typename... Lines> void indent(int i, const Lines &... args) {
  auto p = Printer(i);
  std::vector<std::string> lines = {args...};
  for (auto line : lines) {
    p.println(line);
  }
}

template <typename... Lines> void comment(const Lines &... args) {
  auto p = CommentPrinter();
  std::vector<std::string> lines = {args...};
  for (auto line : lines) {
    p.println(line);
  }
}

template <typename M, typename... Lines>
void with_gutter(const M m, const Lines &... args) {
  auto p = Printer();
  p.gutter.push(m);
  std::vector<std::string> lines = {args...};
  for (auto line : lines) {
    p.println(line);
  }
}

template <typename... Lines>
void with_gutter(const Gutter::State s, const Lines &... args) {
  auto p = Printer();
  p.gutter.states.push(s);
  std::vector<std::string> lines = {args...};
  for (auto line : lines) {
    p.println(line);
  }
}

template <typename... Lines>
void numbered(const int sn, const Lines &... args) {
  auto p = NumberedPrinter();
  p.linenum = sn - 1;
  p.rightGutter.enabled = true;
  p.leftGutter.enabled = true;
  std::vector<std::string> lines = {args...};
  for (auto line : lines) {
    p.println(line);
  }
}

template <typename... Lines>
void aligned(const Align align, const int width, const Lines &... args) {
  auto p = Printer();
  std::vector<std::string> lines = {args...};
  for (auto content : lines) {
    content = utils::parse(content);
    auto f = content.size() - utils::stripEsc(content).size();
    switch (align) {
    case Align::LEFT:
      p.println(fmt::format("{:<{}}", content, width + f));
      break;
    case Align::MIDDLE:
      p.println(fmt::format("{:^{}}", content, width + f));
      break;
    case Align::RIGHT:
      p.println(fmt::format("{:>{}}", content, width + f));
      break;
    }
  }
}

} // namespace helpers
std::string operator""_p(const char *str, std::size_t len) {
  return utils::parse(str);
}

} // namespace LibPrint

#endif // __LIBPRINT_H_
